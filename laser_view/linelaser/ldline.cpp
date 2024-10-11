#include "ldline.h"

LDLine::LDLine()
{

}
LDLine::~LDLine() {}

bool LDLine::Pack(const TRData &in, std::vector<uint8_t> &out)
{
    out.resize(EXTRA_LEN + in.data.size());
    uint8_t *p = out.data();
    *(uint32_t *)p = LEADING_CODE;
    p += 4;
    *p++ = in.device_address;
    *p++ = in.pack_id;
    *(uint16_t *)p = in.chunk_offset;
    p += 2;
    *(uint16_t *)p = in.data.size();
    p += 2;
    std::memcpy(p, in.data.data(), in.data.size());
    uint8_t checksum = CalCheckSum(out.data() + 4, out.size() - 5);

    out.back() = checksum;

    return true;
}

bool LDLine::sendData(QSerialPort *serial, const uint8_t *data, size_t size)
{
    qint64  ret;
    QByteArray send_t((char *)(data), size);
    // qDebug() << send_t;
    ret = serial->write(send_t);
    if (ret < send_t.size())
    {
        return false;
    }

    return true;
}

bool LDLine::SendCmd(QSerialPort* port, uint8_t address, uint8_t id)
{
    std::vector<uint8_t> out;
    TRData out_data;
    out_data.device_address = address;
    out_data.pack_id = id;
    out_data.chunk_offset = 0;
    Pack(out_data, out);

    return sendData(port, (const uint8_t *)out.data(), out.size());
}

const TRData *LDLine::Unpack(const uint8_t *data, uint32_t len)
{
    if (data == nullptr || len < EXTRA_LEN) {
        return nullptr;
    }

    const uint8_t *p = data;
    uint32_t code = *(uint32_t *)data;

    if (code != LEADING_CODE) {
        return nullptr;
    }

    p += 8;
    uint16_t data_len = *(uint16_t *)p;

    if (data_len > (len - EXTRA_LEN)) {
        return nullptr;
    }

    p += 2;

    uint8_t checksum = CalCheckSum(data + 4, 6 + data_len);

    p += data_len;

    if (checksum == *p) {
        tr_unpack_data_.data.clear();
        p = data;
        p += 4;
        tr_unpack_data_.device_address = *p++;
        tr_unpack_data_.pack_id = *p++;
        tr_unpack_data_.chunk_offset = *(uint16_t *)p;
        p += 2;
        if (tr_unpack_data_.data.size() < data_len) {
            tr_unpack_data_.data.resize(data_len);
        }
        p += 2;
        std::memcpy(tr_unpack_data_.data.data(), p, data_len);
        parse_data_len_ = data_len + EXTRA_LEN;

        return &tr_unpack_data_;
    }

    return nullptr;
}

bool LDLine::AnalysisTRNetByte(uint8_t byte)
{
    static enum {
        HEADER1,
        HEADER2,
        HEADER3,
        HEADER4,
        LENS,
        DATA,
    } state = HEADER1;
    static uint16_t count = 0;
    static uint8_t tmp[500] = {0};
    static uint16_t pkg_count = 0;

    switch (state) {
    case HEADER1:
        if (byte == 0xAA) {
            tmp[count++] = byte;
            state = HEADER2;
        }
        break;
    case HEADER2:
        if (byte == 0xAA) {
            tmp[count++] = byte;
            state = HEADER3;
        } else {
            state = HEADER1;
            count = 0;
        }
        break;
    case HEADER3:
        if (byte == 0xAA) {
            tmp[count++] = byte;
            state = HEADER4;
        } else {
            state = HEADER1;
            count = 0;
        }
        break;
    case HEADER4:
        if (byte == 0xAA) {
            tmp[count++] = byte;
            state = LENS;
        } else {
            state = HEADER1;
            count = 0;
        }
        break;
    case LENS:
        tmp[count++] = byte;
        if (count == 10) {
            uint16_t data_lens_val = ((tmp[9] << 8) | tmp[8]);

            if (data_lens_val > 324) {
                state = HEADER1;
                count = 0;
            } else {
                pkg_count = data_lens_val + 11;
                state = DATA;
            }
        }
        break;
    case DATA:
        tmp[count++] = byte;
        if (count >= pkg_count) {
            state = HEADER1;
            count = 0;
            tr_data_ = Unpack(tmp, pkg_count);

            if ((tr_data_ != nullptr)) {
                return true;
            } else {
                return false;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

bool LDLine::Transform(const TRData *tr_data)
{
    // ldlidar::Points2D tmp,filter_tmp;
    /*Packet length minus 4-byte timestamp*/
    int data_amount = tr_data->data.size() - 4;
    int n = 0;
    // ldlidar::TransData trans_data;
    float range = 0.0;
    float angle = 0.0;
    uint8_t intensity = 0;
    std::vector<W_DataScan> showData;

    for (int i = 0; i < data_amount; i += 2, n++)
    {
        /*Acquired distance information data*/
        uint16_t value = *(uint16_t *)(tr_data->data.data() + i + 4);
        //get the distance,center_value,cofidence
        uint16_t confidence = (((value >> 10) & 0x007) << 5);  //confidence [12:10]   the value need to multiply by 32
        uint16_t center_dis = (value & 0x3ff);                 //distance   [9:0]    same as
        uint8_t center_kb  = ((value >> 13) & 0x007);         //kb range   [15:13]  high FOV,need this,because of the camera distortion
        // if (center_dis > 0)
        // {
        //     if (!trans_data.TransformSignlePoint(coe_, 35, center_dis, n*2, center_kb, confidence, tmp)) {
        //         std::cout << "[ldrobot] trans data error" << std::endl;
        //     }
        // }
        angle = (float)((A_FOV/320*i - A_FOV/2)*M_PI/180);
        range = (float)center_dis/cos(angle);
        intensity = (uint8_t)confidence;
        W_DataScan m_sData;
        m_sData.angles_ = angle*180/M_PI;
        m_sData.ranges_ = range;
        m_sData.intensity_ = intensity;
        m_sData.speed = (uint16_t)(1.0 /((getClockT_ms() - mlast_time)/1000.0));
        showData.push_back(m_sData);
    }
    if (!showData.empty())
    {
        if (DataFun != nullptr)
            DataFun(showData);
    }
    mlast_time = getClockT_ms();
    // Sslbf outlier_point(fov_angle_, total_point_number_);
    // filter_tmp = outlier_point.OutlierFilter(tmp);
    // SetLaserScanData(filter_tmp);
    // SetFrameReady();

    return true;
}

bool LDLine::UnpackData(const uint8_t *data, uint32_t len)
{

    for (uint32_t i = 0; i < len; i++)
    {
        if (AnalysisTRNetByte(data[i]))
        {
            switch (tr_data_->pack_id) {
            case PACK_GET_DISTANCE: {
                Transform(tr_data_);
                break;
            }
            case PACK_GET_COE: {
                qDebug() << "[ldrobot] get PACK_GET_COE ";
                memcpy(coe_, tr_data_->data.data(),sizeof(coe_));
                break;
            }
            case PACK_VIDEO_SIZE: {
                coe_u_ = *(uint16_t *)(tr_data_->data.data());
                coe_v_ = *(uint16_t *)(tr_data_->data.data() + 2);
                qDebug() << "[ldrobot] get PACK_VIDEO_SIZE " << coe_u_ << "*" << coe_v_;
                parameters_ready_ = true;
                break;
            }
            case PACK_VERSION: {
                qDebug() << "[ldrobot] get PACK_VERSION ";
                QString vStr;
                for (auto m : tr_data_->data)
                    vStr += QString::number(m, 16);
                if (!get_Version)
                {
                    get_Version = true;
                    other_message += "Version: ";
                    other_message += vStr;
                }
                break;
            }
            case PACK_ACK: {
                uint8_t cmd = *tr_data_->data.data();
                uint8_t ack = *(tr_data_->data.data()+1);
                qDebug() << "[ldrobot] ACK RSP cmd: "<< (int)cmd << " ,ack:" << (int)ack;
                if (!is_addr_ready_)
                {
                    is_addr_ready_ = true;
                    if (LdDevSnFun != nullptr)
                        LdDevSnFun(TYPE_LASER_LD, nullptr);
                }
            }
            default: {
                break;
            }
            }
        }
    }

    return true;
}

bool LDLine::initLaserScan(QSerialPort *serial)
{
    SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_STOP);
    MySleep(50);

    qDebug() << "LDLine PACK_CONFIG_ADDRESS";
    SendCmd(serial, 0, PACK_CONFIG_ADDRESS);
    MySleep(20);
    SendCmd(serial, 0, PACK_CONFIG_ADDRESS);
    MySleep(50);

    qDebug() << "LDLine PACK_GET_COE";
    SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_GET_COE);
    MySleep(20);
    SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_GET_COE);
    MySleep(50);

    qDebug() << "LDLine PACK_VIDEO_SIZE";
    SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_VIDEO_SIZE);
    MySleep(20);
    SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_VIDEO_SIZE);
    MySleep(50);

    qDebug() << "LDLine PACK_VERSION";
    SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_VERSION);
    MySleep(50);
    // SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_STOP);
    // SendCmd(serial, 0, PACK_CONFIG_ADDRESS);
    // SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_GET_COE);
    // SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_VIDEO_SIZE);

    return true;
}
bool LDLine::StartLaserScan(QSerialPort *serial)
{
    mlast_time = getClockT_ms();
    return SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_GET_DISTANCE);
}
bool LDLine::StopLaserScan(QSerialPort *serial)
{
    return SendCmd(serial, THIS_DEVICE_ADDREESS, PACK_STOP);
}
void LDLine::DataParsing(const QByteArray &data)
{
    QByteArray m_data = data;
    uint64_t data_size = data.size();
    uint8_t *dataPtr = reinterpret_cast<uint8_t*>(m_data.data());
    UnpackData(dataPtr, data_size);
}
