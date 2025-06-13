#include "rsline.h"

RSLine::RSLine()
{
    pointcloud_.resize(160);
}
RSLine::~RSLine() {}

uint8_t RSLine::check(const QByteArray data, size_t len)
{
    uint8_t cs = 0;
    for (size_t i = sizeof(start_bit_); i < len; i++)
        cs += data[i];
    return cs;
}
Message RSLine::Unpack(const std::vector<uint8_t> &unkown_data, int &processed_num)
{
    Message msg;
    // 寻找起始位
    if (unkown_data.size() < sizeof(format_.start_bit_))
    {
        processed_num = 0;
        return msg;
    }
    auto it = std::search(unkown_data.begin(), unkown_data.end(), format_.start_bit_, format_.start_bit_ + sizeof(format_.start_bit_));
    if (it == unkown_data.end())
    {
        processed_num = unkown_data.size();
        return msg;
    }
    // 判断是否接收完数据头
    processed_num = it - unkown_data.begin();
    int vail_num = unkown_data.end() - it;
    if (vail_num < sizeof(Protocol_vx90))
    {
        return msg;
    }

    // 判断是否接收完一帧
    const uint8_t *data_start = unkown_data.data() + processed_num;
    auto msg_head = reinterpret_cast<const Protocol_vx90 *>(data_start);
    int msg_length = sizeof(Protocol_vx90) + msg_head->length_;
    if (vail_num < msg_length)
    {
        return msg;
    }
    processed_num += msg_length;

    // 校验数据包
    auto msg_cs = reinterpret_cast<const Protocol_vx90 *>(data_start + msg_head->length_)->cs_;
    if (msg_cs != Protocol_vx90::check(data_start, msg_length))
    {
        return msg;
    }

    // 封装数据包
    msg.addr_ = msg_head->addr_;
    msg.cmd_ = msg_head->cmd_;
    msg.offset_ = msg_head->offset_;
    msg.length_ = msg_head->length_;
    msg.data_.assign(data_start + data_pos(), data_start + msg_length - 1);
    msg.isnull_ = false;
    return msg;
}
std::vector<uint8_t> RSLine::Pack(Message &msg)
{
    std::vector<uint8_t> ret_data(sizeof(Protocol_vx90) + msg.data_.size()); // 先开辟空间
    Protocol_vx90 original;
    original.addr_ = msg.addr_;
    original.cmd_ = msg.cmd_;
    original.offset_ = msg.offset_;
    original.length_ = msg.data_.size();

    // 将原始协议拷贝到vector
    std::copy_n(reinterpret_cast<uint8_t *>(&original), data_pos(), ret_data.begin());
    std::copy_n(msg.data_.begin(), msg.data_.size(), ret_data.begin() + data_pos());
    ret_data.back() = Protocol_vx90::check(ret_data.data(), ret_data.size());

    return ret_data;
}

bool RSLine::CalcDistance(const Message &msg)
{
    if (msg.data_.size() != 320)
    {
        return false;
    }

    const uint16_t *pvalue = reinterpret_cast<const uint16_t *>(msg.data_.data());
    for (size_t i = 0; i < msg.data_.size() / sizeof(uint16_t); i++)
    {
        TransformSignlePoint(pvalue[i], i, AddrToDevId(msg.addr_));
    }
    return true;
}
float RSLine::computePointCloudX(
    float u,
    float v,
    float depth,
    std::vector<float> param)
{
    float rd_x = (u - param[0]) / param[2];
    float rd_y = (v - param[1]) / param[2];
    float theta_d = std::sqrt(rd_x * rd_x + rd_y * rd_y);
    float theta = theta_d;
    for (int i = 0; i < 5; i++) {
        float theta_2 = theta * theta;
        float theta_4 = theta_2 * theta_2;
        float f_theta = theta * (1.0 + param[3] * theta_2 + param[4] * theta_4) - theta_d;
        float delta_f_theta = 1.0 + 3.0 * param[3] * theta_2 + 5.0 * param[4] * theta_4;
        theta = theta - f_theta / delta_f_theta;
    }
    float r_x = rd_x / (theta_d + FLT_MIN) * std::tan(theta);

    return r_x*(depth- param[5]);
}

bool RSLine::BCalcDistance(const Message& msg)
{
    if (msg.data_.size() == pointcloud_.size() * 2 + 3)
    {
        // for debug
    }
    else if (msg.data_.size() != pointcloud_.size() * 2)
    {
        return false;
    }

    uint8_t dev_idx = AddrToDevId(msg.addr_);

    if (param_type_.size() <= dev_idx || param_type_[dev_idx] == 0)  // 无参数解析数据
    {
        // [0,9]位是深度数据，[10,12]是强度数据，[13,15]是角度分组下标
        for (int i = 0; i < pointcloud_.size(); ++i)
        {
            uint16_t data = ((uint16_t)msg.data_[i * 2 + 1] << 8) | msg.data_[i * 2];
            if ((data & 0x3FF) != 0)
            {
                pointcloud_[i].Y() = data & 0x3FF;
                pointcloud_[i].X() = i;
                pointcloud_[i].intensity() = (data >> 10) & 0x07;;
            }
            else
            {
                pointcloud_[i].Y() = 0;
                pointcloud_[i].X() = 0;
                pointcloud_[i].intensity() = 0;

            }
        }
    }
    else if (param_type_[dev_idx] == 1)  // 第一种标定参数计算
    {
        // [0,9]位是深度数据，[10,12]是强度数据，[13,15]是角度分组下标
        for (int i = 0; i < pointcloud_.size(); ++i)
        {
            uint16_t data = ((uint16_t)msg.data_[i * 2 + 1] << 8) | msg.data_[i * 2];
            if ((data & 0x3FF) != 0)
            {
                int angle_idx = data >> 13;
                double c1 = Bparam_[dev_idx][4 * angle_idx + 0];
                double c2 = Bparam_[dev_idx][4 * angle_idx + 1];
                double c3 = Bparam_[dev_idx][4 * angle_idx + 2];
                double c4 = Bparam_[dev_idx][4 * angle_idx + 3];
                int n = (i / 3 * 4) + (i % 3) + 1 - c4;
                int n_2 = n * n;
                pointcloud_[i].Y() = data & 0x3FF;
                pointcloud_[i].X() = (c1 * n) / (1 + c2 * n_2 + c3 * n_2 * n_2) * pointcloud_[i].Y();
                pointcloud_[i].intensity() = (data >> 10) & 0x07;;
            }
            else
            {
                pointcloud_[i].Y() = 0;
                pointcloud_[i].X() = 0;
                pointcloud_[i].intensity() = 0;
            }
        }
    }
    else if (param_type_[dev_idx] == 2)  // 第二种标定参数计算
    {
        const std::vector<float>& dist_param = param_[dev_idx];
        // [0,9]位是深度数据，[10,12]是强度数据，[13,15]是角度分组下标
        for (int i = 0; i < pointcloud_.size(); ++i)
        {
            uint16_t data = ((uint16_t)msg.data_[i * 2 + 1] << 8) | msg.data_[i * 2];
            if ((data & 0x3FF) != 0)
            {
                float vd = (v_start_[dev_idx] + (data >> 13) * v_gap_[dev_idx] + 15) / 2;
                float ud = (i / 3 * 4) + (i % 3) + 1;
                pointcloud_[i].Y() = data & 0x3FF;
                pointcloud_[i].X() = computePointCloudX(ud, vd, pointcloud_[i].Y(),dist_param);
                pointcloud_[i].intensity() = (data >> 10) & 0x07;
            }
            else
            {
                pointcloud_[i].Y() = 0;
                pointcloud_[i].X() = 0;
                pointcloud_[i].intensity() = 0;
            }
        }
    }

    return true;
}

bool RSLine::ConfigAddress(const Message &msg)
{
    device_addr_ = msg.addr_;
    int device_num = AddrToDevCount(msg.addr_);
    param_.resize(device_num);
    Bparam_.resize(device_num);
    param_type_.resize(device_num, 0);
    v_start_.resize(device_num, 0);
    v_gap_.resize(device_num, 0);
    return true;
}

bool RSLine::ConfigParam(const Message &msg)
{
    if (msg.data_.empty())
    {
        return false;
    }
    uint8_t devID = AddrToDevId(msg.addr_);
    const float *rev_param = reinterpret_cast<const float *>(msg.data_.data());
    param_[devID].assign(rev_param, rev_param + 6);
    return true;
}

bool RSLine::BConfigParam(const Message& msg)
{
    if (msg.data_.empty())
    {
        return false;
    }
    uint8_t dev_idx = AddrToDevId(msg.addr_);
    if (msg.length_ == 32 * 8 + 2)
    {
        param_type_[dev_idx] = 1;
        v_start_[dev_idx] = msg.offset_;
        v_gap_[dev_idx] = msg.data_[0] + (msg.data_[1] << 8);
        const double* rev_param = reinterpret_cast<const double*>(msg.data_.data() + 2);
        Bparam_[dev_idx].assign(rev_param, rev_param + 32);
    }
    else if (msg.length_ == 6 * 4 + 2)
    {
        param_type_[dev_idx] = 2;
        v_start_[dev_idx] = msg.offset_;
        v_gap_[dev_idx] = msg.data_[0] + (msg.data_[1] << 8);
        const float* rev_param = reinterpret_cast<const float*>(msg.data_.data() + 2);
        param_[dev_idx].assign(rev_param, rev_param + 6);
    }
    else
    {
        param_type_[dev_idx] = 0;
        return false;
    }
    return true;
}

void RSLine::TransformSignlePoint(const uint16_t dist, const int n, const uint8_t device_id)
{
    pointcloud_[n].intensity() = (dist >> 12);
    uint16_t m_dist = (dist & 0x0FFF);
    if (m_dist == 0)
    {
        pointcloud_[n].X() = 0;
        pointcloud_[n].Y() = 0;
        return;
    }

    double Z = m_dist / 10.0 - 5;
    double U, X;
    if (n < 80)
    {
        U = n * 4;
        if(param_.size()>device_id && param_[device_id].size() > 2)
            X = (param_[device_id][0] * U * Z + param_[device_id][1] * Z) / (param_[device_id][2] * U + 1);
        X = X - 5;
    }
    else
    {
        U = (n - 80) * 4;
        X = (param_[device_id][3] * U * Z + param_[device_id][4] * Z) / (param_[device_id][5] * U + 1);
        X = X + 5;
    }
    pointcloud_[n].X() = X;
    pointcloud_[n].Y() = m_dist / 10.0f;
}
uint8_t RSLine::AddrToDevId(uint8_t addr)
{
    int count = 0;
    while (addr != 1)
    {
        addr = addr >> 1;
        count++;
    }
    return count;
}

uint8_t RSLine::AddrToDevCount(uint8_t addr)
{
    int count = 0;
    while (addr != 0)
    {
        count++;
        addr &= (addr - 1);
    }
    return count;
}

bool RSLine::LaserScanSendCMD(QSerialPort *serial, int addr, uint8_t cmd)
{
    qint64  ret;
    Message Mes(addr, cmd);
    std::vector<uint8_t> msg = Pack(Mes);
    QByteArray sendData((char *)(msg.data()), msg.size());
    // uint16_t data_len = 0;
    // sendData.append(static_cast<char>(start_bit_[0]));
    // sendData.append(static_cast<char>(start_bit_[1]));
    // sendData.append(static_cast<char>(addr));
    // sendData.append(static_cast<char>(cmd));
    // sendData.append(static_cast<char>((offset_ & 0xff)));
    // sendData.append(static_cast<char>(((offset_ >> 8) & 0xff)));
    // sendData.append(static_cast<char>((data_len & 0xff)));
    // sendData.append(static_cast<char>(((data_len >> 8) & 0xff)));
    // uint8_t check_ = check(sendData, sendData.size());
    // sendData.append(static_cast<char>(check_));
    //qDebug() << Qt::hex <<sendData;
    ret = serial->write(sendData);
    if (ret < sendData.size())
    {
        qDebug() << "RebootDeviceClose failed.";
        return false;
    }

    return true;
}

bool RSLine::initLaserScan(QSerialPort *serial)
{
    LaserScanSendCMD(serial, 1, RSCommand::CMD_STOP_MEASURE);
    MySleep(50);

    qDebug() << "initLaserScan kCmdAddress";
    LaserScanSendCMD(serial, 0, RSCommand::CMD_SET_DEV_ADDR);
    MySleep(20);
    LaserScanSendCMD(serial, 0, RSCommand::CMD_SET_DEV_ADDR);
    MySleep(50);

    qDebug() << "initLaserScan type";
    LaserScanSendCMD(serial, 1, RSCommand::CMD_SET_GET_TYPE);
    MySleep(20);
    LaserScanSendCMD(serial, 1, RSCommand::CMD_SET_GET_TYPE);
    MySleep(50);

    qDebug() << "initLaserScan kCmdParam";
    LaserScanSendCMD(serial, 1, RSCommand::CMD_GET_PARAME);
    MySleep(20);
    LaserScanSendCMD(serial, 1, RSCommand::CMD_GET_PARAME);
    MySleep(50);

    qDebug() << "initLaserScan kCmdSN";
    LaserScanSendCMD(serial, 1, RSCommand::CMD_GET_SN);
    MySleep(20);
    LaserScanSendCMD(serial, 1, RSCommand::CMD_GET_SN);
    MySleep(50);

    qDebug() << "initLaserScan kCmdWave";
    LaserScanSendCMD(serial, 1, RSCommand::CMD_GET_WAVE);
    MySleep(50);
    qDebug() << "initLaserScan kCmdVersion";
    LaserScanSendCMD(serial, 1,  RSCommand::CMD_GET_VERSION);
    MySleep(50);

    return true;
}

bool RSLine::StartLaserScan(QSerialPort *serial)
{
    last_time = getClockTime_ms();
    return LaserScanSendCMD(serial, device_addr_, RSCommand::CMD_START_MEASURE);
}

bool RSLine::StopLaserScan(QSerialPort *serial)
{
    return LaserScanSendCMD(serial, device_addr_, RSCommand::CMD_STOP_MEASURE);
}

void RSLine::DataParsing(const QByteArray &data)
{
    QByteArray m_data = data;
    uint64_t data_size = data.size();
    uint8_t *dataPtr = reinterpret_cast<uint8_t*>(m_data.data());
    UnpackData(dataPtr, data_size);
}

bool RSLine::UnpackData(const uint8_t *data, size_t len)
{
    int traverse_num;
    rev_data_.insert(rev_data_.end(), data, data + len);
    while (1)
    {
        auto msg = Unpack(rev_data_, traverse_num);
        rev_data_.erase(rev_data_.begin(), rev_data_.begin() + traverse_num); // 删除遍历过的数据
        if (msg.IsNull())
        {
            return -1;
        }
        //qDebug() << "cccccccccccccccccccc" << traverse_num << rev_data_.size();
        switch(msg.cmd_)
        {
        case RSCommand::CMD_START_MEASURE:
        {
            bool nowReady = false;
            if(is_param_ready_ != 255)
            {
                if (is_param_ready_ != XV90)
                {
                    if (BCalcDistance(msg))
                        nowReady = true;
                }
                else
                {
                    if (CalcDistance(msg))
                        nowReady = true;
                }
            }

            if (nowReady)
            {
                std::unique_lock<std::mutex> lf(_mutex_LaserData);
                LaserDataScanData.clear();
                LineLaserDataScan data_temp;
                std::vector<W_DataScan> showData;
                for(size_t i = 0; i < pointcloud_.size(); i++)
                {
                    data_temp.angles_ = (float)atan2(pointcloud_[i].X(), pointcloud_[i].Y());
                    data_temp.ranges_ = (float)hypot(pointcloud_[i].X(), pointcloud_[i].Y());
                    if (intensity_need_)
                        data_temp.intensity_ = (float)pointcloud_[i].intensity();
                    else
                        data_temp.intensity_ = 100;
                    LaserDataScanData.push_back(data_temp);
                    // qDebug() << data_temp.angles_ << data_temp.ranges_ << data_temp.intensity_;

                    W_DataScan m_sData;
                    m_sData.angles_ = data_temp.angles_*180/M_PI;
                    m_sData.ranges_ = data_temp.ranges_;
                    m_sData.intensity_ = data_temp.intensity_;
                    m_sData.speed = (uint16_t)(1.0 /((getClockTime_ms() - last_time)/1000.0));
                    showData.push_back(m_sData);
                }
                laserdatascan_reday = true;
                // qDebug() << "*********************************" << LaserDataScanData.size();
                if (!showData.empty())
                {
                    if (DataFun != nullptr)
                        DataFun(showData);
                }
                last_time = getClockTime_ms();
                lf.unlock();
            }
        }
        break;
        case RSCommand::CMD_SET_DEV_ADDR:
        {
            if (ConfigAddress(msg))
            {
                is_addr_ready_ = true;
                if (!is_SN_ready_ && RsDevSnFun != nullptr)
                    RsDevSnFun(TYPE_LASER_RS, nullptr);
                qDebug() << "RS_linelaser ConfigAddress";
            }
        }
        break;
        case RSCommand::CMD_GET_PARAME:
        {
            if (RS_type != XV90)
            {
                if (BConfigParam(msg))
                {
                    is_param_ready_ = RS_type;
                    pointcloud_.resize(240);
                    qDebug() << "RS_linelaser ConfigParam" << RS_type;
                }
            }
            else
            {
                if (ConfigParam(msg))
                {
                    is_param_ready_ = XV90;
                    pointcloud_.resize(160);
                    qDebug() << "RS_linelaser ConfigParam" << RS_type;
                }
            }
        }
        break;
        case RSCommand::CMD_STOP_MEASURE:
        {
            if (StopWork(msg))
            {
                qDebug() << "RS_linelaser Stop";
            }
        }
        break;
        case RSCommand::CMD_GET_WAVE:
        {
            if(!msg.data_.empty())
            {
                if (!get_Wave)
                {
                    QString m_wave;
                    get_Wave = true;
                    if (msg.data_.size() > 1)
                    {
                        uint64_t sD = 0;
                        uint16_t sD_c = 0;
                        for ( auto s:msg.data_ )
                        {
                            sD += s << sD_c*8;
                            sD_c++;
                        }
                        m_wave = QString::number(sD);
                        if (!other_message.isEmpty())
                            other_message += "<br>";
                    }
                    else
                        m_wave = QString::number(0x300+msg.data_.front());
                    qDebug() << "RS_linelaser Wave:" <<  m_wave << msg.data_.size();

                    if (!other_message.isEmpty())
                        other_message += "<br>Vave: ";
                    else
                        other_message += "Vave: ";
                    other_message += m_wave;
                }
            }
        }
        break;
        case RSCommand::CMD_GET_VERSION:
        {
            if(!msg.data_.empty())
            {
                qDebug() << "RS_linelaser Version:" << Qt::hex << msg.data_.front();
                if (!get_Version)
                {
                    get_Version = true;
                    QString m_version = QString::number(msg.data_.front(), 16);
                    if (!other_message.isEmpty())
                        other_message += "<br>Version: ";
                    else
                        other_message += "Version: ";
                    other_message += m_version;
                }
            }
        }
        break;
        case RSCommand::CMD_GET_SN:
        {
            is_SN_ready_ = true;
            if (msg.data_.size() >= 8)
            {
                lineSensorSn = *((uint64_t *)msg.data_.data());
                if (lineSensorSn != 0xffffffffffffffff /*&& lineSensorSn >= 530200000000000*/) //530300000000000
                {
                    intensity_need_ = true;
                    QString qStr = QString::number(lineSensorSn);
                    if (RsDevSnFun != nullptr)
                    {
                        if (RS_type != XV90)
                            RsDevSnFun(TYPE_LASER_RS_XVB02, qStr);
                        else
                            RsDevSnFun(TYPE_LASER_RS_NEW, qStr);
                    }
                }
                else
                {
                    intensity_need_ = false;
                    if (RsDevSnFun != nullptr)
                    {
                        if (RS_type != XV90)
                            RsDevSnFun(TYPE_LASER_RS_XVB02, nullptr);
                        else
                            RsDevSnFun(TYPE_LASER_RS, nullptr);
                    }
                }
                qDebug() << "RS_linelaser SN:"<< lineSensorSn << Qt::hex << lineSensorSn << "size:" << msg.data_.size() << intensity_need_;
            }
        }
        break;
        case RSCommand::CMD_SET_GET_TYPE:
        {
            if(!msg.data_.empty())
            {
                RS_type = msg.data_.front();
                qDebug() << "RS_linelaser TYPE:"<< msg.data_ << msg.data_.size();
            }
        }
        break;
        default:
        {
            // 扩展任务事件
            HandleElseTask(msg);
        }
        break;
        }
    }
    return true;
}
