#ifndef LPKG_H
#define LPKG_H
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include "FullScanFilter.h"
#include "inc/radarscan.h"
#include "morefilter.h"

#define ANGLED_TO_RADIAN(angle) ((angle)*M_PI / 180)
#define RADIAN_TO_ANGLED(angle) ((angle)*180 / M_PI)

#define HEAD_LEN    (5)
#define HEAD_FLAG   (0xF5A5)
#define DATA_LEN    (1)

typedef enum
{
    SET_ROTATION_SPEED = 0xA1,
    SET_RUN_MODE = 0xA2,
}CMD;
typedef enum
{
    WRITE_PARAM = 0xC1,
    WRITE_PARAM_RESPONSE = 0xC2,
    READ_PARAM = 0xC3,
    READ_PARAM_RESPONSE = 0xC4,
}CMD_TYPE;

enum
{
    PKG_HEADER = 0x54,
    PKG_VER_LEN = 0x2C,
    POINT_PER_PACK = 12,
};

typedef enum
{
    TYPE_RADAR_LD = 0,//乐动
    TYPE_RADAR_AOBI = 1,//奥比MS200
    TYPE_RADAR_LANHAI = 2,//蓝海
    TYPE_RADAR_GW = 3,//光为
    TYPE_RADAR_AOBIK = 4,//奥比MS200K
    TYPE_RADAR_BZ = 5,//不止

    TYPE_RADAR_NONE = 255,
}TYPE_RADAR_;

typedef struct device_sn_st
{
    std::string buff;
    TYPE_RADAR_ id{TYPE_RADAR_LD};
}device_sn_t;

typedef struct __attribute__((packed))
{
    uint16_t distance;
    uint8_t confidence;
} LidarPointStructDef;

typedef struct __attribute__((packed))
{
    uint8_t header;
    uint8_t ver_len;
    uint16_t speed;
    uint16_t start_angle;
    LidarPointStructDef point[POINT_PER_PACK];
    uint16_t end_angle;
    uint16_t timestamp;
    uint8_t crc8;
} LiDARFrameTypeDef;

typedef struct EXTERN_LASER_ODOM_DATA {
    float Distance[720];
    float Angle[720];
    uint8_t confidence[720];
    uint64_t start_timestamp;
    uint64_t end_timestamp;
    uint32_t valid_num;
    bool laser_data_ok_;
    bool laser_data_cover_;
}Extern_Laser_Data;

static const uint8_t CrcTable[256] = {
    0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3, 0xae, 0xf2, 0xbf, 0x68, 0x25,
    0x8b, 0xc6, 0x11, 0x5c, 0xa9, 0xe4, 0x33, 0x7e, 0xd0, 0x9d, 0x4a, 0x07,
    0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8, 0xf5, 0x1f, 0x52, 0x85, 0xc8,
    0x66, 0x2b, 0xfc, 0xb1, 0xed, 0xa0, 0x77, 0x3a, 0x94, 0xd9, 0x0e, 0x43,
    0xb6, 0xfb, 0x2c, 0x61, 0xcf, 0x82, 0x55, 0x18, 0x44, 0x09, 0xde, 0x93,
    0x3d, 0x70, 0xa7, 0xea, 0x3e, 0x73, 0xa4, 0xe9, 0x47, 0x0a, 0xdd, 0x90,
    0xcc, 0x81, 0x56, 0x1b, 0xb5, 0xf8, 0x2f, 0x62, 0x97, 0xda, 0x0d, 0x40,
    0xee, 0xa3, 0x74, 0x39, 0x65, 0x28, 0xff, 0xb2, 0x1c, 0x51, 0x86, 0xcb,
    0x21, 0x6c, 0xbb, 0xf6, 0x58, 0x15, 0xc2, 0x8f, 0xd3, 0x9e, 0x49, 0x04,
    0xaa, 0xe7, 0x30, 0x7d, 0x88, 0xc5, 0x12, 0x5f, 0xf1, 0xbc, 0x6b, 0x26,
    0x7a, 0x37, 0xe0, 0xad, 0x03, 0x4e, 0x99, 0xd4, 0x7c, 0x31, 0xe6, 0xab,
    0x05, 0x48, 0x9f, 0xd2, 0x8e, 0xc3, 0x14, 0x59, 0xf7, 0xba, 0x6d, 0x20,
    0xd5, 0x98, 0x4f, 0x02, 0xac, 0xe1, 0x36, 0x7b, 0x27, 0x6a, 0xbd, 0xf0,
    0x5e, 0x13, 0xc4, 0x89, 0x63, 0x2e, 0xf9, 0xb4, 0x1a, 0x57, 0x80, 0xcd,
    0x91, 0xdc, 0x0b, 0x46, 0xe8, 0xa5, 0x72, 0x3f, 0xca, 0x87, 0x50, 0x1d,
    0xb3, 0xfe, 0x29, 0x64, 0x38, 0x75, 0xa2, 0xef, 0x41, 0x0c, 0xdb, 0x96,
    0x42, 0x0f, 0xd8, 0x95, 0x3b, 0x76, 0xa1, 0xec, 0xb0, 0xfd, 0x2a, 0x67,
    0xc9, 0x84, 0x53, 0x1e, 0xeb, 0xa6, 0x71, 0x3c, 0x92, 0xdf, 0x08, 0x45,
    0x19, 0x54, 0x83, 0xce, 0x60, 0x2d, 0xfa, 0xb7, 0x5d, 0x10, 0xc7, 0x8a,
    0x24, 0x69, 0xbe, 0xf3, 0xaf, 0xe2, 0x35, 0x78, 0xd6, 0x9b, 0x4c, 0x01,
    0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17, 0x5a, 0x06, 0x4b, 0x9c, 0xd1,
    0x7f, 0x32, 0xe5, 0xa8};

class LPkg
{
    enum
    {
        ID_HEAD_AOBI = 0x55AA,
        ID_HEAD_LANHAI = 0xCC55,
        ID_HEAD_GW = 0xCCAA,
        ID_HEAD_BZ = 0xDDAA,//0xAE55,
    };

public:
        LPkg();
        ~LPkg();
        void MySleep(uint32_t ms)
        {
            QElapsedTimer timer;
            timer.start();

            while(timer.elapsed() < ms){
                // 处理其他任务
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
        }
        bool m_timerMs(uint8_t timer, int64_t timeout, time_t *out=nullptr)
        {
            time_t nowT = getClockTimeStamp_ms();
            if (out != nullptr) *out = 0;

            if (TimerMap.find(timer) == TimerMap.end())
                TimerMap[timer] = nowT;

            time_t time_diff = nowT - TimerMap[timer];
            if (timeout == -1)
            {
                TimerMap[timer] = nowT;
            }
            else
            {
                while (time_diff < 0)
                    time_diff += DataFilling<time_t>();
                if (time_diff >= timeout)
                {
                    if (out != nullptr) *out = time_diff;
                    TimerMap[timer] = nowT;
                    return true;
                }
            }
            if (out != nullptr) *out = time_diff;

            return false;
        };

        void setDeviceSnCallback(std::function<void(const device_sn_t)> callback)
        {
            DevSnFun = callback;
        }
        void setDataCallback(std::function<void(std::vector<W_DataScan>)> callback)
        {
            DataFun = callback;
        }

        bool DeviceOpen(QSerialPort *cmd_port);
        bool DeviceClose(QSerialPort *cmd_port);
        void DataParsing(const QByteArray &data);
        void SetFilter(uint16_t filter)
        {
            filter_select = filter;
        }
        uint16_t GetFilter()
        {
            return filter_select;
        }
        QString GetOtherMessage()
        {
            return other_message;
        }
private:
        template<typename T> T DataFilling(bool unsig=false)
        {
            T dataF = 0;
            uint64_t ty_size = 0;

            if (unsig)
            {
                ty_size = sizeof(T) * 8;
                while (ty_size)
                {
                    dataF = (dataF << 1) + 1;
                    ty_size --;
                }
            }
            else
            {
                ty_size = sizeof(T) * 8 - 1;
                while (ty_size)
                {
                    dataF = (dataF << 1) + 1;
                    ty_size --;
                }
            }

            return dataF;
        };

        uint8_t xor_check(QByteArray data, uint16_t len)
        {
            uint8_t check = 0;
            for (int i = 0; i < len; i++)
            {
                check ^= data[i];
            }
            return check;
        }
        uint8_t and_check(const std::vector<uint8_t> data, uint16_t len)
        {
            uint8_t check = 0;
            for (int i = 0; i < len; i++)
            {
                check += data[i];
            }
            return check;
        }
        uint8_t CalCRC8(const uint8_t *data, uint16_t data_len)
        {
            uint8_t crc = 0;
            while (data_len--)
            {
                crc = CrcTable[(crc ^ *data) & 0xff];
                data++;
            }
            return crc;
        }
        uint64_t getClockTimeStamp_ns(void)
        {
            struct timespec time_data_in;
            clock_gettime(CLOCK_MONOTONIC, &time_data_in);
            return (uint64_t)(time_data_in.tv_sec * 1000000000 + time_data_in.tv_nsec);
        };
        uint64_t getClockTimeStamp_ms(void)
        {
            struct timespec time_data_in;
            clock_gettime(CLOCK_MONOTONIC, &time_data_in);
            return (uint64_t)(time_data_in.tv_sec * 1000 + time_data_in.tv_nsec / 1000000);
        };
        double GetSpeed(void) { return speed_ / 360.0; }

        bool find_init_info(unsigned char *data, int len);
        bool AnalysisOne(uint8_t byte);
        bool Parse(const uint8_t *data, long len);
        bool AssemblePacket();
        void ToLaserData(std::vector<PointData> src);
        bool GetLaserData(Extern_Laser_Data *laser_data);

private:
    std::map<uint8_t, time_t> TimerMap;
    std::function<void(const device_sn_t)> DevSnFun = nullptr;
    std::function<void(std::vector<W_DataScan>)> DataFun = nullptr;
    std::vector<uint8_t> cmd_buf_;
    device_sn_t device_sn_;
    bool init_info_flag_{true};
    bool open_again_flag_{false};
    bool close_again_flag_{false};

    const int kPointFrequence = 4500;
    LiDARFrameTypeDef pkg;
    uint64_t current_pack_stamp;
    uint64_t last_pkg_timestamp_;
    uint16_t timestamp_;
    double speed_;
    std::vector<PointData> frame_tmp_;
    bool is_pkg_ready_;
    bool is_frame_ready_;
    std::mutex frame_tmp_mutex;
    std::mutex laser_data_mutex;
    Extern_Laser_Data laser_data_;

    TYPE_RADAR_ lidar_type_ = TYPE_RADAR_LD;
    FullScanFilter full_scan_filter_;
    FilterPara para_inf_;
    uint16_t filter_select{0x01};
    QString other_message;
};

#endif // LPKG_H
