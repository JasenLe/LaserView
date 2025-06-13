#include "yxline.h"

YXLine::YXLine()
{

}
YXLine::~YXLine() {}

bool YXLine::ParsingPackage(const uint8_t *data, size_t len)
{
    data_buf_.insert(data_buf_.end(), data, data + len);
    int recvPos = 0;
    uint8_t CheckSumCal = 0;
    uint8_t command = 0;
    uint16_t sample_lens = 0;
    bool has_device_header = false;
    uint8_t  *packageBuffer = (uint8_t *)&package;
    for ( ; recvPos < data_buf_.size(); )
    {
        uint8_t currentByte = data_buf_[recvPos];
        switch (recvPos)
        {
        case 0:
            if (currentByte != (YXLI_SYNC_HEAD & 0x0000ff))
            {
                data_buf_.erase(data_buf_.begin());
                recvPos = 0;
                continue;
            }
            break;

        case 1:
            if (currentByte != (YXLI_SYNC_HEAD & 0x00ff00) >> 8)
            {
                data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
                recvPos = 0;
                continue;
            }
            break;

        case 2:
            if (currentByte != (YXLI_SYNC_HEAD & 0xff0000) >> 16)
            {
                data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
                recvPos = 0;
                continue;
            }
            has_device_header = true;
            break;

        case 3:
            moduleNum = currentByte;
            CheckSumCal += currentByte;
            break;

        case 4:
            if ((currentByte < YXLI_CMD_ADDRESS || currentByte > YXLI_CMD_REBOOT) && currentByte != 1)
            {
                data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
                recvPos = 0;
                CheckSumCal = 0;
                moduleNum = 0;
                has_device_header = false;
                continue;
            }
            command = currentByte;
            CheckSumCal += currentByte;
            break;

        case 5:
            sample_lens |= (0x00ff&currentByte)<<8;
            CheckSumCal += currentByte;
            break;

        case 6:
            sample_lens |= 0x00ff&currentByte;
            CheckSumCal += currentByte;
            break;

        default :
            if (recvPos < sample_lens + PackagePaidBytes)
                CheckSumCal += currentByte;
            break;
        }

        packageBuffer[recvPos++] = currentByte;
        if (recvPos == (sample_lens + PackagePaidBytes + 1))
        {
            uint8_t CheckSum = packageBuffer[recvPos-1];
            if (has_device_header && CheckSumCal == CheckSum)
            {
                if (command == YXLI_CMD_SCAN || command == 1)
                {
                    original_parsing(package);
                    // qDebug() << "YXLI_CMD_SCAN ok";
                }
                else if (command == YXLI_CMD_STOP)
                {
                    LaserStauts = false;
                    qDebug() << "YXLI_CMD_STOP ok";
                }
                else if (command == YXLI_CMD_ADDRESS)
                {
                    qDebug() << "YXLI_CMD_ADDRESS ok";
                    if (!get_version && YxDevSnFun != nullptr)
                        YxDevSnFun(TYPE_LASER_YX, nullptr);
                }
                else if (command == YXLI_CMD_VERSION)
                {
                    qDebug() << "YXLI_CMD_VERSION ok";
                    if (!get_version)
                    {
                        QString m_version;
                        QString m_sn;
                        get_version = true;
                        for(int i = 0; i < sample_lens; i++)
                        {
                            if (i < 2 )
                                m_version += QString::number(packageBuffer[PackagePaidBytes + i], 16);
                            else
                                m_sn += QString::number(packageBuffer[PackagePaidBytes + i], 16);
                        }
                        if (!other_message.isEmpty())
                            other_message += "<br>Version: ";
                        else
                            other_message += "Version: ";
                        other_message += m_version;

                        if (YxDevSnFun != nullptr)
                            YxDevSnFun(TYPE_LASER_YX, m_sn);
                    }
                }
            }
            data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
            recvPos = 0;
            return true;
        }
        else if (recvPos > (sample_lens + PackagePaidBytes + 1) || sample_lens > 0x6000)
        {
            data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
            recvPos = 0;
            return false;
        }
    }
    return false;
}

bool YXLine::original_parsing(node_package package)
{
    float range = 0.0;
    float intensity = 0.0;
    float angle = 0.0;
    std::vector<W_DataScan> showData;
    for (int i = 0; i < PackageSampleMaxLngth; i++)
    {
        intensity = 0;
#ifdef PACK_SHORT
        float X = (int16_t)((int8_t)(package.packageSample[i].PakageX)*4);
        float Z = (int16_t)(package.packageSample[i].PakageZ);
#else
        float X = (int16_t)(package.packageSample[i].PakageX_MSB << 8 | package.packageSample[i].PakageX_LSB);
        float Z = (int16_t)(package.packageSample[i].PakageZ_MSB << 8 | package.packageSample[i].PakageZ_LSB);
#endif
        range = (float)hypot(X, Z);
        angle = (float)atan2(X, Z);

        W_DataScan m_sData;
        m_sData.angles_ = angle*180/M_PI;
        m_sData.ranges_ = range;
        m_sData.intensity_ = intensity;
        m_sData.speed = (uint16_t)(1.0 /((getClockT_ms() - mlast_time)/1000.0));
        showData.push_back(m_sData);
        // qDebug() << i << m_sData.angles_ << m_sData.ranges_;
    }
    if (!showData.empty())
    {
        if (DataFun != nullptr)
            DataFun(showData);
    }
    mlast_time = getClockT_ms();
    return true;
}

bool YXLine::sendData(QSerialPort *serial, const uint8_t *data, size_t size)
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

bool YXLine::sendCmd(QSerialPort *serial, uint8_t addr, uint8_t cmd)
{
    uint8_t pkt_header[12];
    cmd_pack_yx *header = reinterpret_cast<cmd_pack_yx * >(pkt_header);
    uint8_t checksum = 0;

    header->hByte0 = (YXLI_SYNC_HEAD & 0x0000FF);
    header->hByte1 = (YXLI_SYNC_HEAD & 0x00FF00) >> 8;
    header->hByte2 = (YXLI_SYNC_HEAD & 0xFF0000) >> 16;
    header->address = addr;
    header->cmd_ = cmd;
    header->features_MSB = 0;
    header->features_LSB = 0;
    checksum = get_checksum(pkt_header, 0);

    int header_size = sizeof(cmd_pack_yx);
    pkt_header[header_size] = checksum;
    sendData(serial, pkt_header, header_size+1) ;
    // sendData(serial, &checksum, 1);
    return true;
}

bool YXLine::sendCmd(QSerialPort *serial, uint8_t cmd)
{
    return sendCmd(serial, 0x0, cmd);
}

bool YXLine::initLaserScan(QSerialPort *serial)
{
    sendCmd(serial, YXLI_CMD_STOP);
    MySleep(50);

    qDebug() << "YXLine YXLI_CMD_ADDRESS";
    sendCmd(serial, YXLI_CMD_ADDRESS);
    MySleep(20);
    sendCmd(serial, YXLI_CMD_ADDRESS);
    MySleep(50);

    qDebug() << "YXLine YXLI_CMD_VERSION";
    sendCmd(serial, YXLI_CMD_VERSION);
    MySleep(20);
    sendCmd(serial, YXLI_CMD_VERSION);
    MySleep(50);

    return true;
}

bool YXLine::StartLaserScan(QSerialPort *serial)
{
    mlast_time = getClockT_ms();
    LaserStauts = true;
    return sendCmd(serial, YXLI_CMD_SCAN);
}

bool YXLine::StopLaserScan(QSerialPort *serial)
{
    int s = 0;

    sendCmd(serial, YXLI_CMD_STOP);
    while (++s <= 3)
    {
        if (!LaserStauts)
            return true;

        sendCmd(serial, YXLI_CMD_STOP);
        MySleep(50);
        qDebug() << "YX re send stop count: " << s;
    }

    return false;
}

void YXLine::DataParsing(const QByteArray &data)
{
    QByteArray m_data = data;
    uint64_t data_size = data.size();
    uint8_t *dataPtr = reinterpret_cast<uint8_t*>(m_data.data());
    // for (int i = 0; i < data_size; i++)
    //     qDebug() << Qt::hex << dataPtr[i];
    ParsingPackage(dataPtr, data_size);
}
