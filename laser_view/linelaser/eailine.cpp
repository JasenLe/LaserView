#include "eailine.h"

EAILine::EAILine() {}
EAILine::~EAILine() {}

bool EAILine::waitPackage(const uint8_t *data, size_t len)
{
    data_buf_.insert(data_buf_.end(), data, data + len);
    uint8_t  *packageBuffer = (uint8_t *)&package;
    int recvPos = 0;
    uint8_t CheckSumCal = 0;
    uint16_t sample_lens = 0;
    bool has_device_header = false;
    uint8_t command = 0;
    for ( ; recvPos < data_buf_.size(); )
    {
        uint8_t currentByte = data_buf_[recvPos];
        switch (recvPos)
        {
        case 0:
            if (currentByte != LIDAR_ANS_SYNC_BYTE1)
            {
                data_buf_.erase(data_buf_.begin());
                recvPos = 0;
                continue;
            }
            break;

        case 1:
            if (currentByte != LIDAR_ANS_SYNC_BYTE1)
            {
                data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
                recvPos = 0;
                continue;
            }
            break;

        case 2:
            if (currentByte != LIDAR_ANS_SYNC_BYTE1)
            {
                data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
                recvPos = 0;
                continue;
            }
            break;

        case 3:
            if (currentByte != LIDAR_ANS_SYNC_BYTE1)
            {
                data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
                recvPos = 0;
                continue;
            }
            has_device_header = true;
            break;

        case 4:
            moduleNum = currentByte;
            CheckSumCal += currentByte;
            break;

        case 5:
            if (currentByte < GS_LIDAR_CMD_GET_ADDRESS || currentByte > GS_LIDAR_ANS_SCAN)
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

        case 6:
            sample_lens |= 0x00ff&currentByte;
            CheckSumCal += currentByte;
            break;

        case 7:
            sample_lens |= (0x00ff&currentByte)<<8;
            CheckSumCal += currentByte;
            break;

        default :
            if (recvPos < sample_lens + PackagePaidBytes_GS)
                CheckSumCal += currentByte;
            break;
        }

        packageBuffer[recvPos++] = currentByte;
        if (recvPos == (sample_lens + PackagePaidBytes_GS + 1))
        {
            uint8_t CheckSum = packageBuffer[recvPos-1];
            if (has_device_header && CheckSumCal == CheckSum)
            {
                if (command == GS_LIDAR_ANS_SCAN)
                {
                    if (parameter_flag)
                        original_parsing(package);
                }
                else if (command == GS_LIDAR_CMD_GET_ADDRESS)
                {
                    qDebug() << "GS_LIDAR_CMD_GET_ADDRESS ok";
                    if (!get_version && EAIDevSnFun != nullptr)
                        EAIDevSnFun(TYPE_LASER_EAI, nullptr);
                }
                else if (command == GS_LIDAR_CMD_GET_PARAMETER)
                {
                    qDebug() << "GS_LIDAR_CMD_GET_PARAMETER ok";

                    gs_device_para info;
                    memcpy((uint8_t *)(&info), (uint8_t *)(&(packageBuffer[PackagePaidBytes_GS])), sizeof(info));
                    u_compensateK0[0] = info.u_compensateK0;
                    u_compensateK1[0] = info.u_compensateK1;
                    u_compensateB0[0] = info.u_compensateB0;
                    u_compensateB1[0] = info.u_compensateB1;
                    d_compensateK0[0] = info.u_compensateK0 / 10000.00;
                    d_compensateK1[0] = info.u_compensateK1 / 10000.00;
                    d_compensateB0[0] = info.u_compensateB0 / 10000.00;
                    d_compensateB1[0] = info.u_compensateB1 / 10000.00;
                    bias[0] = double(info.bias) * 0.1;
                    parameter_flag = true;
                }
                else if (command == GS_LIDAR_CMD_GET_VERSION)
                {
                    qDebug() << "GS_LIDAR_CMD_GET_VERSION ok" << sample_lens;
                    if (!get_version)
                    {
                        QString m_version;
                        QString m_sn;
                        get_version = true;
                        for(int i = 0; i < sample_lens; i++)
                        {
                            if (i < 3 )
                                m_version += QString::number(packageBuffer[PackagePaidBytes_GS + i]);
                            else
                                m_sn += QString::number(packageBuffer[PackagePaidBytes_GS + i]);
                        }
                        if (!other_message.isEmpty())
                            other_message += "<br>Version: ";
                        else
                            other_message += "Version: ";
                        other_message += m_version;

                        if (EAIDevSnFun != nullptr)
                            EAIDevSnFun(TYPE_LASER_EAI, m_sn);
                    }
                }
            }
            data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
            recvPos = 0;
            return true;
        }
        else if (recvPos > (sample_lens + PackagePaidBytes_GS + 1))
        {
            data_buf_.erase(data_buf_.begin(), data_buf_.begin()+recvPos);
            recvPos = 0;
            return false;
        }
    }
    return false;
}
bool EAILine::original_parsing(gs2_node_package package)
{
    node_info *node = new node_info;
    (*node).index = 255;
    (*node).scan_frequence  = 0;
    (*node).sync_quality = Node_Default_Quality;
    (*node).stamp = 0;
    (*node).scan_frequence = 0;
    (*node).sync_flag = Node_NotSync;
    std::vector<W_DataScan> showData;
    for (int i = 0; i < PackageSampleMaxLngth_GS; i++)
    {
        float range = 0.0;
        float intensity = 0.0;
        float angle = 0.0;
        double sampleAngle = 0;
        (*node).distance_q2 = package.packageSample[i].PakageSampleDistance;

        if (m_intensities)
            (*node).sync_quality = (uint16_t)package.packageSample[i].PakageSampleQuality;

        if (node->distance_q2 > 0)
        {
            angTransform((*node).distance_q2, i, &sampleAngle, &(*node).distance_q2);
        }

        if (sampleAngle< 0)
        {
            (*node).angle_q6_checkbit = (((uint16_t)(sampleAngle * 64 + 23040)) << LIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) +
                                        LIDAR_RESP_MEASUREMENT_CHECKBIT;
        }
        else
        {
            if ((sampleAngle * 64) > 23040)
            {
                (*node).angle_q6_checkbit = (((uint16_t)(sampleAngle * 64 - 23040)) << LIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) +
                                            LIDAR_RESP_MEASUREMENT_CHECKBIT;
            }
            else
            {
                (*node).angle_q6_checkbit = (((uint16_t)(sampleAngle * 64)) << LIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) +
                                            LIDAR_RESP_MEASUREMENT_CHECKBIT;
            }
        }

        if(i < 80)
        { //CT_RingStart  CT_Normal
            if((*node).angle_q6_checkbit <= 23041)
                (*node).distance_q2 = 0;
        }
        else
        {
            if((*node).angle_q6_checkbit > 23041)
                (*node).distance_q2 = 0;
        }

        {
            angle = static_cast<float>(((*node).angle_q6_checkbit >>
                                        LIDAR_RESP_MEASUREMENT_ANGLE_SHIFT) / 64.0f) + m_AngleOffset;
            range = static_cast<float>((*node).distance_q2);

            intensity = static_cast<float>((*node).sync_quality);
            angle = from_degrees(angle);

            angle = normalize_angle(angle);

            //valid range
            if (!isRangeValid(range))
            {
                range = 0.0;
                intensity = 0.0;
            }

            if (angle >= from_degrees(m_MinAngle) &&
                angle <= from_degrees(m_MaxAngle))
            {
                W_DataScan m_sData;
                m_sData.angles_ = angle*180/M_PI;
                m_sData.ranges_ = range;
                m_sData.intensity_ = intensity;
                m_sData.speed = (uint16_t)(1.0 /((getClockTimeStamp_ms() - last_time)/1000.0));
                showData.push_back(m_sData);
            }
        }


    }
    if (!showData.empty())
    {
        if (DataFun != nullptr)
            DataFun(showData);
    }
    last_time = getClockTimeStamp_ms();

    delete node;
    return true;
}

void EAILine::angTransform(uint16_t dist, int n, double *dstTheta, uint16_t *dstDist)
{
    double pixelU = n, Dist, theta, tempTheta, tempDist, tempX, tempY;
    uint8_t mdNum = 0;//0x03 & (moduleNum >> 1);//1,2,4
    if (n < 80)
    {
        pixelU = 80 - pixelU;
        if (d_compensateB0[mdNum] > 1) {
            tempTheta = d_compensateK0[mdNum] * pixelU - d_compensateB0[mdNum];
        }
        else
        {
            tempTheta = atan(d_compensateK0[mdNum] * pixelU - d_compensateB0[mdNum]) * 180 / M_PI;
        }
        tempDist = (dist - Angle_Px) / cos(((Angle_PAngle + bias[mdNum]) - (tempTheta)) * M_PI / 180);
        tempTheta = tempTheta * M_PI / 180;
        tempX = cos((Angle_PAngle + bias[mdNum]) * M_PI / 180) * tempDist * cos(tempTheta) + sin((Angle_PAngle + bias[mdNum]) * M_PI / 180) * (tempDist *
                                                                                                                                               sin(tempTheta));
        tempY = -sin((Angle_PAngle + bias[mdNum]) * M_PI / 180) * tempDist * cos(tempTheta) + cos((Angle_PAngle + bias[mdNum]) * M_PI / 180) * (tempDist *
                                                                                                                                                sin(tempTheta));
        tempX = tempX + Angle_Px;
        tempY = tempY - Angle_Py; //5.315
        Dist = sqrt(tempX * tempX + tempY * tempY);
        theta = atan(tempY / tempX) * 180 / M_PI;
    }
    else
    {
        pixelU = 160 - pixelU;
        if (d_compensateB1[mdNum] > 1)
        {
            tempTheta = d_compensateK1[mdNum] * pixelU - d_compensateB1[mdNum];
        }
        else
        {
            tempTheta = atan(d_compensateK1[mdNum] * pixelU - d_compensateB1[mdNum]) * 180 / M_PI;
        }
        tempDist = (dist - Angle_Px) / cos(((Angle_PAngle + bias[mdNum]) + (tempTheta)) * M_PI / 180);
        tempTheta = tempTheta * M_PI / 180;
        tempX = cos(-(Angle_PAngle + bias[mdNum]) * M_PI / 180) * tempDist * cos(tempTheta) + sin(-(Angle_PAngle + bias[mdNum]) * M_PI / 180) * (tempDist *
                                                                                                                                                 sin(tempTheta));
        tempY = -sin(-(Angle_PAngle + bias[mdNum]) * M_PI / 180) * tempDist * cos(tempTheta) + cos(-(Angle_PAngle + bias[mdNum]) * M_PI / 180) * (tempDist *
                                                                                                                                                  sin(tempTheta));
        tempX = tempX + Angle_Px;
        tempY = tempY + Angle_Py; //5.315
        Dist = sqrt(tempX * tempX + tempY * tempY);
        theta = atan(tempY / tempX) * 180 / M_PI;
    }
    if (theta < 0)
    {
        theta += 360;
    }
    *dstTheta = theta;
    *dstDist = Dist;
}

bool EAILine::sendData(QSerialPort *serial, const uint8_t *data, size_t size)
{
    qint64  ret;
    QByteArray send_t((char *)(data), size);
    ret = serial->write(send_t);
    if (ret < send_t.size())
    {
        return false;
    }

    return true;
}

bool EAILine::sendCommand(QSerialPort *serial,
                                    uint8_t addr,
                                    uint8_t cmd,
                                    const void *payload,
                                    size_t payloadsize)
{
    uint8_t pkt_header[12];
    cmd_packet_gs *header = reinterpret_cast<cmd_packet_gs * >(pkt_header);
    uint8_t checksum = 0;

    header->syncByte0 = LIDAR_CMD_SYNC_BYTE;
    header->syncByte1 = LIDAR_CMD_SYNC_BYTE;
    header->syncByte2 = LIDAR_CMD_SYNC_BYTE;
    header->syncByte3 = LIDAR_CMD_SYNC_BYTE;
    header->address = addr;
    header->cmd_flag = cmd;
    header->size = 0xffff&payloadsize;
    sendData(serial, pkt_header, 8) ;
    checksum += cmd;
    checksum += 0xff&header->size;
    checksum += 0xff&(header->size>>8);

    if (payloadsize && payload) {
        for (size_t pos = 0; pos < payloadsize; ++pos) {
            checksum += ((uint8_t *)payload)[pos];
        }
        uint8_t sizebyte = (uint8_t)(payloadsize);
        sendData(serial, (const uint8_t *)payload, sizebyte);
    }

    sendData(serial, &checksum, 1);

    return true;
}

bool EAILine::sendCommand(QSerialPort *serial,
                          uint8_t cmd,
                          const void *payload,
                          size_t payloadsize)
{
    return sendCommand(serial, 0x0, cmd, payload, payloadsize);
}

bool EAILine::initLaserScan(QSerialPort *serial)
{
    sendCommand(serial, GS_LIDAR_CMD_STOP);
    MySleep(50);

    qDebug() << "EAILine GS_LIDAR_CMD_GET_ADDRESS";
    sendCommand(serial, GS_LIDAR_CMD_GET_ADDRESS);
    MySleep(20);
    sendCommand(serial, GS_LIDAR_CMD_GET_ADDRESS);
    MySleep(50);

    qDebug() << "EAILine GS_LIDAR_CMD_GET_PARAMETER";
    sendCommand(serial, GS_LIDAR_CMD_GET_PARAMETER);
    MySleep(20);
    sendCommand(serial, GS_LIDAR_CMD_GET_PARAMETER);
    MySleep(50);

    qDebug() << "EAILine GS_LIDAR_CMD_GET_VERSION";
    sendCommand(serial, GS_LIDAR_CMD_GET_VERSION);
    MySleep(20);
    sendCommand(serial, GS_LIDAR_CMD_GET_VERSION);
    MySleep(50);

    return true;
}

bool EAILine::StartLaserScan(QSerialPort *serial)
{
    last_time = getClockTimeStamp_ms();
    return sendCommand(serial, GS_LIDAR_CMD_SCAN);
}

bool EAILine::StopLaserScan(QSerialPort *serial)
{
   return sendCommand(serial, GS_LIDAR_CMD_STOP);
}

void EAILine::DataParsing(const QByteArray &data)
{
    QByteArray m_data = data;
    uint64_t data_size = data.size();
    uint8_t *dataPtr = reinterpret_cast<uint8_t*>(m_data.data());
    waitPackage(dataPtr, data_size);
}
