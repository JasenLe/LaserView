#include "lpkg.h"

LPkg::LPkg()
{

}

LPkg::~LPkg()
{

}

bool LPkg::DeviceOpen(QSerialPort *cmd_port)
{
    qint64  ret;
    QByteArray sendData;
    sendData.append(static_cast<char>(HEAD_FLAG & 0xff));
    sendData.append(static_cast<char>(HEAD_FLAG >>8 & 0xff));
    sendData.append(static_cast<char>(SET_RUN_MODE));
    sendData.append(static_cast<char>(WRITE_PARAM));
    sendData.append(static_cast<char>(DATA_LEN));
    sendData.append(0x81);
    sendData.append(static_cast<char>(xor_check(sendData, HEAD_LEN + DATA_LEN)));
    sendData.append(0x31);
    sendData.append(0xF2);
    cmd_port->flush();
    m_timerMs(0, -1);
    if (close_again_flag_)
    {
        init_info_flag_ = false;
        close_again_flag_ = false;
    }
    ret = cmd_port->write(sendData);
    if (ret < HEAD_LEN + DATA_LEN +3)
    {
        qDebug() << "DeviceOpen failed.";
        return false;
    }
    last_pkg_timestamp_ = getClockTimeStamp_ns();
    return true;
}

bool LPkg::DeviceClose(QSerialPort *cmd_port)
{
    qint64  ret;
    QByteArray sendData;
    sendData.append(static_cast<char>(HEAD_FLAG & 0xff));
    sendData.append(static_cast<char>((HEAD_FLAG >> 8) & 0xff));
    sendData.append(static_cast<char>(SET_RUN_MODE));
    sendData.append(static_cast<char>(WRITE_PARAM));
    sendData.append(static_cast<char>(DATA_LEN));
    sendData.append(0X80);
    sendData.append(static_cast<char>(xor_check(sendData, HEAD_LEN + DATA_LEN)));
    sendData.append(0x31);
    sendData.append(0xF2);
    init_info_flag_ = true;
    close_again_flag_ = true;
    lidar_type_ = TYPE_RADAR_NONE;
    ret = cmd_port->write(sendData);
    if (ret < HEAD_LEN + DATA_LEN +3)
    {
        qDebug() << "RebootDeviceClose failed.";
        return false;
    }
    cmd_port->flush();

    return true;
}

void LPkg::DataParsing(const QByteArray &data)
{
    QByteArray m_data = data;
    uint64_t data_size = data.size();
    uint8_t *dataPtr = reinterpret_cast<uint8_t*>(m_data.data());
    if (!init_info_flag_)
    {
        init_info_flag_ = find_init_info(dataPtr, data_size);
        if (init_info_flag_)
        {
            if (DevSnFun)
                DevSnFun(device_sn_);
            lidar_type_ = device_sn_.id;
        }
        else
        {
            if (device_sn_.id != TYPE_RADAR_NONE && device_sn_.id != lidar_type_)
            {
                if (DevSnFun)
                    DevSnFun(device_sn_);
                lidar_type_ = device_sn_.id;
            }

            if (m_timerMs(0, 5000)) //timeout
            {
                if (device_sn_.id == TYPE_RADAR_NONE)
                {
                    device_sn_.id = TYPE_RADAR_LD;
                    device_sn_.buff.clear();
                    if (DevSnFun)
                        DevSnFun(device_sn_);
                    lidar_type_ = device_sn_.id;
                }
                init_info_flag_ = true;
            }
        }
    }
    if (Parse(dataPtr, data_size))
    {
        AssemblePacket();
    }
}

bool LPkg::find_init_info(unsigned char *data, int len)
{
    cmd_buf_.insert(cmd_buf_.end(), data, data + len);
    int cmd_buf_len = cmd_buf_.size();
    uint8_t cur_cmd = 0;
    int pos = 0, data_len = 0;
    uint16_t *id_head;
    static TYPE_RADAR_ head_flag = TYPE_RADAR_NONE;
    while (pos < (cmd_buf_len - 3))
    {
        if (head_flag == TYPE_RADAR_NONE)
        {
            id_head = (uint16_t *)&cmd_buf_[pos];
            if (*id_head == ID_HEAD_AOBI)
            {
                head_flag = TYPE_RADAR_AOBI;
                break;
            }
            else if (*id_head == ID_HEAD_LANHAI)
            {
                head_flag = TYPE_RADAR_LANHAI;
                break;
            }
            else if (*id_head == ID_HEAD_GW)
            {
                head_flag = TYPE_RADAR_GW;
                break;
            }
            else if (*id_head == ID_HEAD_BZ)
            {
                head_flag = TYPE_RADAR_BZ;
                break;
            }
        }
        else
        {
            break;
        }
        pos++;
    }
    if (pos != 0)
    {
        cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + pos);
    }

    int tmp_buf_len = cmd_buf_.size();
    if (tmp_buf_len > 3)
    {
        switch (head_flag)
        {
        case TYPE_RADAR_AOBI:
        {
            cur_cmd = cmd_buf_[2];
            data_len = cmd_buf_[3];

            if (tmp_buf_len >= (data_len + 7)) // 7= 2byte head + 1byte flag + 1byte len + 1byte crc + 2byte tail
            {
                if (cur_cmd == 0x01)
                {
                    if (cmd_buf_[5 + data_len] == 0x31 && cmd_buf_[6 + data_len] == 0xF2)
                    {
                        device_sn_.buff.clear();
                        device_sn_.buff.assign((char *)&cmd_buf_[4], data_len); //MS200->"CF3" MS200k->"CF7"
                        if (device_sn_.buff.find("CF7") != std::string::npos)
                            device_sn_.id = TYPE_RADAR_AOBIK;
                        else
                            device_sn_.id = TYPE_RADAR_AOBI;
                    }
                    else
                    {
                        // qDebug() << "find device sn fail";
                    }
                    cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + data_len + 7);
                    head_flag = TYPE_RADAR_NONE;
                    return true;
                }
                else if (cur_cmd == 0x02)
                {
                    cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + data_len + 7);
                    head_flag = TYPE_RADAR_NONE;
                    return false;
                }
                else if (cur_cmd == 0x03)
                {
                    //init_info_flag_ = false;
                    cmd_buf_.clear();
                    head_flag = TYPE_RADAR_NONE;
                    return false;
                }
                else
                {
                    cmd_buf_.clear();
                    head_flag = TYPE_RADAR_NONE;
                    return false;
                }
            }
        }
        break;
        case TYPE_RADAR_LANHAI:
        {
            cur_cmd = cmd_buf_[2];
            data_len = cmd_buf_[3];
            if (tmp_buf_len >= (data_len + 5)) // 5= 2byte head + 1byte flag + 1byte len + 1byte crc
            {
                if (cur_cmd == 0x01)
                {
                    if (cmd_buf_[4 + data_len] == and_check(cmd_buf_, data_len+4)) // crc is [0]~[end()-1]
                    {
                        std::string Lidar_ID = "E120-R"; //{0x45/*'E'*/, 0x31/*'1'*/, 0x32/*'2'*/, 0x30/*'0'*/, 0x2d/*'-'*/, 0x52/*'R'*/} //0x7f
                        std::string Lidar_ID2 = "E110-R"; //{0x45/*'E'*/, 0x31/*'1'*/, 0x31/*'1'*/, 0x30/*'0'*/, 0x2d/*'-'*/, 0x52/*'R'*/} //0x7e
                        std::string identify;
                        identify.append((char *)&cmd_buf_[4], data_len);
                        if ((strcmp(identify.c_str(), Lidar_ID.c_str()) == 0))
                        {
                            device_sn_.buff.clear();
                            device_sn_.buff.assign((char *)&cmd_buf_[4], data_len);
                            device_sn_.id = TYPE_RADAR_LANHAI;
                        }
                        else if ((strcmp(identify.c_str(), Lidar_ID2.c_str()) == 0))
                        {
                            device_sn_.buff.clear();
                            device_sn_.buff.assign((char *)&cmd_buf_[4], data_len);
                            device_sn_.id = TYPE_RADAR_LANHAI;
                        }
                        else
                        {
                            // device_sn_.buff.clear();
                            // device_sn_.buff.assign((char *)&cmd_buf_[4], data_len);
                            // device_sn_.id = TYPE_RADAR_LANHAI;
                        }
                    }
                    cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + data_len + 5);
                    head_flag = TYPE_RADAR_NONE;
                    // return true;
                }
                else if (cur_cmd == 0x02)
                {
                    if (cmd_buf_[4 + data_len] == and_check(cmd_buf_, data_len+4))
                    {
                        std::string s_SN((char *)&cmd_buf_[4], data_len);
                        other_message = QString::fromStdString(device_sn_.buff) + "/" + QString::fromStdString(s_SN);
                        device_sn_.id = TYPE_RADAR_LANHAI;
                    }
                    cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + data_len + 5);
                    head_flag = TYPE_RADAR_NONE;
                    return true;
                }
                else
                {
                    // cmd_buf_.clear();
                    cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + data_len + 5);
                    head_flag = TYPE_RADAR_NONE;
                    return false;
                }
            }

        }
        break;
        case TYPE_RADAR_GW:
        {
            cur_cmd = cmd_buf_[2];
            data_len = cmd_buf_[3];
            if (tmp_buf_len >= (data_len + 5)) // 5= 2byte head + 1byte flag + 1byte len + 1byte crc
            {
                if (cur_cmd == 0x01)
                {
                    if (cmd_buf_[4 + data_len] == and_check(cmd_buf_, data_len+4)) // crc is [0]~[end()-1] //0x37
                    {
                        device_sn_.buff.clear();
                        device_sn_.buff.assign((char *)&cmd_buf_[4], data_len); //"LTXMD1"
                        device_sn_.id = TYPE_RADAR_GW;
                    }
                    cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + data_len + 5);
                    head_flag = TYPE_RADAR_NONE;
                    return true;
                }
                else
                {
                    cmd_buf_.clear();
                    head_flag = TYPE_RADAR_NONE;
                    return false;
                }
            }
        }
        break;
        case TYPE_RADAR_BZ:
        {
            cur_cmd = cmd_buf_[2];
            data_len = cmd_buf_[3];
            if (tmp_buf_len >= (data_len + 5))
            {
                if (cmd_buf_[4 + data_len] == and_check(cmd_buf_, data_len+4))
                {
                    if (cur_cmd == 0x01) //下 UID
                    {
                    }
                    else if (cur_cmd == 0x02) //下 ID
                    {
                    }
                    else if (cur_cmd == 0x03) //上 UID
                    {
                        device_sn_.buff.clear();
                        device_sn_.buff.assign((char *)&cmd_buf_[4], data_len);
                        device_sn_.id = TYPE_RADAR_BZ;
                        cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + data_len + 5);
                        head_flag = TYPE_RADAR_NONE;
                        return true;
                    }
                    else if (cur_cmd == 0x04) //上 ID
                    {
                    }
                    else if (cur_cmd == 0x05) //SN
                    {
                        device_sn_.buff.clear();
                        device_sn_.buff.assign((char *)&cmd_buf_[4], data_len);
                        device_sn_.id = TYPE_RADAR_BZ;
                        cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + data_len + 5);
                        head_flag = TYPE_RADAR_NONE;
                        return true;
                    }
                    else
                    {
                    }

                }
                cmd_buf_.erase(cmd_buf_.begin(), cmd_buf_.begin() + data_len + 5);
                head_flag = TYPE_RADAR_NONE;
                return false;
            }
        }
        break;

        default:
            break;
        }
    }
    return false;
}

bool LPkg::AnalysisOne(uint8_t byte)
{
    static enum { HEADER,
                  VER_LEN,
                  DATA,
                  } state = HEADER;
    static uint16_t count = 0;
    static uint8_t tmp[128] = {0};
    static uint16_t pkg_count = sizeof(LiDARFrameTypeDef);

    switch (state)
    {
    case HEADER:
        if (byte == PKG_HEADER)
        {
            tmp[count++] = byte;
            state = VER_LEN;
        }
        break;
    case VER_LEN:
        if (byte == PKG_VER_LEN)
        {
            tmp[count++] = byte;
            state = DATA;
        }
        else
        {
            state = HEADER;
            count = 0;
            return false;
        }
        break;
    case DATA:
        tmp[count++] = byte;
        if (count >= pkg_count)
        {
            memcpy((uint8_t *)&pkg, tmp, pkg_count);
            uint8_t crc = CalCRC8((uint8_t *)&pkg, pkg_count - 1);
            state = HEADER;
            count = 0;
            if (crc == pkg.crc8)
            {
                return true;
            }
            else
            {
                // error_times_++;
                qDebug() << "data error.";
                return false;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

bool LPkg::Parse(const uint8_t *data, long len)
{
    for (int i = 0; i < len; i++)
    {
        if (AnalysisOne(data[i]))
        {
            // parse a package is success
            current_pack_stamp = getClockTimeStamp_ns();
            speed_ = pkg.speed;         // Degrees per second
            timestamp_ = pkg.timestamp; // In milliseconds
            double diff = (pkg.end_angle / 100 - pkg.start_angle / 100 + 360) % 360;
            if (diff > (double)pkg.speed * POINT_PER_PACK / kPointFrequence * 1.5) // 2300
            {
                // error_times_++;
                qDebug() << "speed error end angle =" << (int)pkg.end_angle << "start angle = " << (int)pkg.start_angle << "speed = "<< (int)pkg.speed;
            }
            else
            {
                // if (0 == last_pkg_timestamp_)
                // {
                //     last_pkg_timestamp_ = getClockTimeStamp_ns();
                //     continue;
                // }

                double pack_stamp_diff = static_cast<double>(current_pack_stamp - last_pkg_timestamp_);
                int pkg_interval_number = POINT_PER_PACK - 1;
                double pack_stamp_point_step = pack_stamp_diff / static_cast<double>(pkg_interval_number);

                uint32_t diff = (static_cast<uint32_t>(pkg.end_angle) + 36000 - static_cast<uint32_t>(pkg.start_angle)) % 36000;
                float step = static_cast<float>(diff) / pkg_interval_number / 100.0;
                float start = static_cast<float>(pkg.start_angle) / 100.0;
                // float end = (double)(pkg.end_angle % 36000) / 100.0;
                PointData data;
                for (int i = 0; i < POINT_PER_PACK; i++)
                {
                    data.distance = pkg.point[i].distance;
                    data.angle = start + i * step;
                    if (data.angle >= 360.0)
                    {
                        data.angle -= 360.0;
                    }
                    data.confidence = pkg.point[i].confidence;
                    data.timeStamp = static_cast<uint64_t>(current_pack_stamp - (pack_stamp_point_step * (pkg_interval_number - i)));
                        //static_cast<uint64_t>(last_pkg_timestamp_ + (pack_stamp_point_step * i));
                    // data.timeStamp = timeStamp;
                    // one_pkg_[i] = data;
                    std::unique_lock<std::mutex> lf(frame_tmp_mutex);
                    frame_tmp_.push_back(
                        PointData(data.angle, data.distance, data.confidence, data.timeStamp));
                    lf.unlock();
                }

                // prevent angle invert
                // one_pkg_.back().angle = end;
                is_pkg_ready_ = true;
            }
            last_pkg_timestamp_ = getClockTimeStamp_ns();//current_pack_stamp;
        }
    }
    return true;
}

bool LPkg::AssemblePacket()
{
    std::unique_lock<std::mutex> lf(frame_tmp_mutex);
    Points2D tmp, data;
    int count = 0;
    float last_angle = 0;
    static int last_count = 0;
    for (auto n : frame_tmp_)
    {
#ifdef A_CIRCLE
        auto sTemp = *frame_tmp_.begin();
        float dTemp = sTemp.angle - n.angle;
        if (count > 0 && ((/* fabs(dTemp) <= 0.8 || */ dTemp > 0) || (count >= 550)))
#else
        if (((n.angle < 20.0) && (last_angle > 340.0)) || count >= 550)
#endif
        {
            if (count >= 500)
                qDebug() << "AssemblePacket data count over angle:" <<  n.angle <<  count;

            if ((count * GetSpeed()) > (kPointFrequence * 1.4))
            {
                frame_tmp_.erase(frame_tmp_.begin(), frame_tmp_.begin() + count);
                qDebug() <<"GetSpeed over :" << GetSpeed() << count;
                last_count = 0;
                return false;
            }
            data.insert(data.begin(), frame_tmp_.begin(), frame_tmp_.begin() + count);
            tmp = data;

            // std::sort(tmp.begin(), tmp.end(), [](PointData a, PointData b)
            //           { return a.angle < b.angle; });
            frame_tmp_.erase(frame_tmp_.begin(), frame_tmp_.begin() + count);
            if (tmp.size() > 0)
            {
                ToLaserData(tmp);
                last_count = 0;
                return true;
            }
            else
            {
                qDebug() << "Getdata tmp none -> frame_tmp_size:" << frame_tmp_.size();
                return false;
            }
        }
        else
        {
            if (last_angle - n.angle > 0)
            {
                if (last_count != count)
                    qDebug() << "AssemblePacket angle over" << last_angle << n.angle << count << frame_tmp_.size();
                last_count = count;
            }
        }
        count++;
        last_angle = n.angle;
    }
    return false;
}

void LPkg::ToLaserData(std::vector<PointData> src)
{
    int count = 0;
    std::vector<std::vector<PointData>> out;
    std::vector<PointData> new_club;

    std::unique_lock<std::mutex> lf(laser_data_mutex);
    memset(&laser_data_, 0, sizeof(laser_data_));
    if (src.size() > 600)
    {
        is_frame_ready_ = false;
        lf.unlock();
        qDebug() << "ToLaserData data too long" << src.size();
        return;
    }

    is_frame_ready_ = true;

    out.push_back(src);
    if ((filter_select & (filter_smooth & 0xffff)) != 0)
    {
        para_inf_.filter_type = FullScanFilter::FS_Smooth;
        full_scan_filter_.filter(out.back(), para_inf_, new_club);
        out.push_back(new_club);
        new_club.clear();
        // qDebug() << "FS_Smooth";
    }

    if ((filter_select & (filter_bilateral & 0xffff)) != 0)
    {
        para_inf_.filter_type = FullScanFilter::FS_Bilateral;
        full_scan_filter_.filter(out.back(), para_inf_, new_club);
        out.push_back(new_club);
        new_club.clear();
        // qDebug() << "FS_Bilateral";
    }

    if ((filter_select & (filter_tail & 0xffff)) != 0)
    {
        para_inf_.filter_type = FullScanFilter::FS_Tail;
        full_scan_filter_.filter(out.back(), para_inf_, new_club);
        out.push_back(new_club);
        new_club.clear();
        // qDebug() << "FS_Tail";
    }

    if ((filter_select & (filter_intensity & 0xffff)) != 0)
    {
        para_inf_.filter_type = FullScanFilter::FS_Intensity;
        full_scan_filter_.filter(out.back(), para_inf_, new_club);
        out.push_back(new_club);
        new_club.clear();
        // qDebug() << "FS_Intensity";
    }

    Tofbf tofbf(speed_);
    if ((filter_select & (filter_near & 0xffff)) != 0)
    {
        new_club = tofbf.NearFilter(out.back());
        out.push_back(new_club);
        new_club.clear();
    }
    if ((filter_select & (filter_noise & 0xffff)) != 0)
    {
        new_club = tofbf.NoiseFilter(out.back());
        out.push_back(new_club);
        new_club.clear();
    }
    if ((filter_select & (filter_tine & 0xffff)) != 0)
    {
        new_club = tofbf.TineFilter(out.back());
        out.push_back(new_club);
        new_club.clear();
    }
    if ((filter_select & (filter_wander & 0xffff)) != 0)
    {
        new_club = tofbf.WanderFilter(out.back());
        out.push_back(new_club);
        new_club.clear();
    }
    if ((filter_select & (filter_shadows & 0xffff)) != 0)
    {
        new_club = tofbf.ShadowsFilter(out.back());
        out.push_back(new_club);
        new_club.clear();
    }
    if ((filter_select & (filter_median & 0xffff)) != 0)
    {
        new_club = tofbf.MedianFilter(out.back());
        out.push_back(new_club);
        new_club.clear();
    }

    std::vector<W_DataScan> showData;
    uint64_t start_time = 0, end_time = 0;
    for (size_t i = 0; i < out.back().size(); i++)
    {
        laser_data_.Angle[count] = ANGLED_TO_RADIAN(out.back()[i].angle);
        laser_data_.Distance[count] = out.back()[i].distance;
        laser_data_.confidence[count] = out.back()[i].confidence;

        W_DataScan m_sData;
        m_sData.angles_ = out.back()[i].angle;
        m_sData.ranges_ = out.back()[i].distance;
        m_sData.intensity_ = out.back()[i].confidence;
        m_sData.speed = (uint16_t)speed_;
        showData.push_back(m_sData);

        count++;
    }

    start_time = out.back().front().timeStamp;
    end_time = out.back().back().timeStamp;
    // auto s_t = std::min_element(out.begin(), out.end(), [](PointData& a, PointData& b){ return a.timeStamp < b.timeStamp; });
    // auto e_t = std::max_element(out.begin(), out.end(), [](PointData& a, PointData& b){ return a.timeStamp < b.timeStamp; });
    // start_time = s_t->timeStamp;
    // end_time = e_t->timeStamp;

    laser_data_.valid_num = count;
    laser_data_.start_timestamp = start_time;
    laser_data_.end_timestamp = end_time;
    // qDebug() << "ToLaserData :" << laser_data_.valid_num << getClockTimeStamp_ms();

    if (!showData.empty())
    {
        if (DataFun != nullptr)
            DataFun(showData);
    }

    lf.unlock();
}

bool LPkg::GetLaserData(Extern_Laser_Data *laser_data)
{
    if (laser_data == NULL)
    {
        return false;
    }

    std::unique_lock<std::mutex> lf(laser_data_mutex);
    if (!is_frame_ready_)
    {
        lf.unlock();
        return false;
    }

    is_frame_ready_ = false;
    memcpy(laser_data, &laser_data_, sizeof(Extern_Laser_Data));
    memset(&laser_data_, 0, sizeof(Extern_Laser_Data));
    lf.unlock();
    if (laser_data->valid_num > 0)
    {
        return true;
    }
    else
    {
        qDebug() << "GetLaserData no valid num..";
        return false;
    }
}
