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
bool RSLine::ConfigAddress(const Message &msg)
{
    device_addr_ = msg.addr_;
    param_.resize(AddrToDevCount(msg.addr_));
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
    LaserScanSendCMD(serial, 1, kCmdStop);
    MySleep(50);

    qDebug() << "initLaserScan kCmdAddress";
    LaserScanSendCMD(serial, 0, kCmdAddress);
    MySleep(20);
    LaserScanSendCMD(serial, 0, kCmdAddress);
    MySleep(50);

    qDebug() << "initLaserScan kCmdParam";
    LaserScanSendCMD(serial, 1, kCmdParam);
    MySleep(20);
    LaserScanSendCMD(serial, 1, kCmdParam);
    MySleep(50);

    qDebug() << "initLaserScan kCmdSN";
    LaserScanSendCMD(serial, 1, kCmdSN);
    MySleep(20);
    LaserScanSendCMD(serial, 1, kCmdSN);
    MySleep(50);

    qDebug() << "initLaserScan kCmdWave";
    LaserScanSendCMD(serial, 1, kCmdWave);
    MySleep(50);
    qDebug() << "initLaserScan kCmdVersion";
    LaserScanSendCMD(serial, 1,  kCmdVersion);
    MySleep(50);

    return true;
}

bool RSLine::StartLaserScan(QSerialPort *serial)
{
    last_time = getClockTime_ms();
    return LaserScanSendCMD(serial, device_addr_, kCmdDistance);
}

bool RSLine::StopLaserScan(QSerialPort *serial)
{
    return LaserScanSendCMD(serial, device_addr_, kCmdStop);
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
        if (msg.cmd_ == kCmdDistance)
        {
            if(is_param_ready_)
            {
                if (CalcDistance(msg))
                {
                    std::unique_lock<std::mutex> lf(_mutex_LaserData);
                    LaserDataScanData.clear();
                    LineLaserDataScan data_temp;
                    std::vector<W_DataScan> showData;
                    for(size_t i = 0; i < pointcloud_.size(); i++)
                    {
                        data_temp.angles_ = -(float)atan2(pointcloud_[i].X(), pointcloud_[i].Y());
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
        }
        else if (msg.cmd_ == kCmdAddress)
        {
            if (ConfigAddress(msg))
            {
                is_addr_ready_ = true;
                if (!is_SN_ready_ && RsDevSnFun != nullptr)
                    RsDevSnFun(TYPE_LASER_RS, nullptr);
                qDebug() << "RS_linelaser ConfigAddress";
            }
        }
        else if (msg.cmd_ == kCmdParam)
        {
            if (ConfigParam(msg))
            {
                is_param_ready_ = true;
                qDebug() << "RS_linelaser ConfigParam";
            }
        }
        else if (msg.cmd_ == kCmdStop)
        {
            if (StopWork(msg))
            {
                qDebug() << "RS_linelaser Stop";
            }
        }
        else if (msg.cmd_ == kCmdWave)
        {
            if(!msg.data_.empty())
            {
                qDebug() << "RS_linelaser Wave:" <<  0x300+msg.data_.front();
                if (!get_Wave)
                {
                    get_Wave = true;
                    QString m_wave = QString::number(0x300+msg.data_.front());
                    if (!other_message.isEmpty())
                        other_message += "<br>Vave: ";
                    else
                        other_message += "Vave: ";
                    other_message += m_wave;
                }
            }
        }
        else if (msg.cmd_ == kCmdVersion)
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
        else if (msg.cmd_ == kCmdSN)
        {
            is_SN_ready_ = true;
            if (msg.data_.size() >= 8)
            {
                lineSensorSn = *((uint64_t *)msg.data_.data());
                if (lineSensorSn != 0xffffffffffffffff && lineSensorSn >= 530200000000000) //530300000000000
                {
                    intensity_need_ = true;
                    QString qStr = QString::number(lineSensorSn);
                    if (RsDevSnFun != nullptr)
                        RsDevSnFun(TYPE_LASER_RS_NEW, qStr);
                }
                else
                {
                    intensity_need_ = false;
                    if (RsDevSnFun != nullptr)
                        RsDevSnFun(TYPE_LASER_RS, nullptr);
                }
                qDebug() << "RS_linelaser SN:"<< lineSensorSn << Qt::hex << lineSensorSn << "size:" << msg.data_.size() << intensity_need_;
            }
        }
        else
        {
            // 扩展任务事件
            HandleElseTask(msg);
        }
    }
    return true;
}
