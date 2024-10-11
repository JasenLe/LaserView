#ifndef EAILINE_H
#define EAILINE_H
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include "llsdk_base.h"

#define LIDAR_CMD_SYNC_BYTE                 0xA5
#define LIDAR_ANS_SYNC_BYTE1                0xA5

#define GS_LIDAR_CMD_GET_ADDRESS               0x60
#define GS_LIDAR_CMD_GET_PARAMETER             0x61
#define GS_LIDAR_CMD_GET_VERSION               0x62
#define GS_LIDAR_CMD_SCAN                      0x63
#define GS_LIDAR_ANS_SCAN                      0x63
#define GS_LIDAR_CMD_STOP                      0x64
#define GS_LIDAR_CMD_RESET                     0x67

#define PackageMaxModuleNums  0x03
#define PackagePaidBytes_GS      8
#define PackageSampleMaxLngth_GS 160
#define Node_Default_Quality (10)
#define Node_Sync 1
#define Node_NotSync 2

#define Angle_Px   1.22
#define Angle_Py   5.315
#define Angle_PAngle   22.5

#define LIDAR_RESP_MEASUREMENT_CHECKBIT       (0x1<<0)
#define LIDAR_RESP_MEASUREMENT_ANGLE_SHIFT    1

struct cmd_packet_gs {
    uint8_t syncByte0;
    uint8_t syncByte1;
    uint8_t syncByte2;
    uint8_t syncByte3;
    uint8_t address;
    uint8_t cmd_flag;
    uint16_t size;
} __attribute__((packed)) ;

struct GS2PackageNode {
    uint16_t PakageSampleDistance:9;
    uint16_t PakageSampleQuality:7;
} __attribute__((packed));

struct gs2_node_package {
    uint32_t  package_Head;
    uint8_t   address;
    uint8_t   package_CT;
    uint16_t  size;
    uint16_t  BackgroudLight;
    GS2PackageNode  packageSample[PackageSampleMaxLngth_GS];
    uint8_t  checkSum;
} __attribute__((packed)) ;

struct node_info {
    uint8_t    sync_flag;  //sync flag
    uint16_t   sync_quality;//!信号质量
    uint16_t   angle_q6_checkbit; //!测距点角度
    uint16_t   distance_q2; //! 当前测距点距离
    uint64_t   stamp; //! 时间戳
    uint8_t    scan_frequence;//! 特定版本此值才有效,无效值是0
    uint8_t    debug_info[12];
    uint8_t    index;
} __attribute__((packed)) ;

struct gs_device_para {
    uint16_t u_compensateK0;
    uint16_t u_compensateB0;
    uint16_t u_compensateK1;
    uint16_t u_compensateB1;
    int8_t  bias;
    uint8_t  crc;
} __attribute__((packed)) ;

class EAILine : public LLSDK_BASE
{
public:
    EAILine();
    ~EAILine();

    virtual bool initLaserScan(QSerialPort *serial);
    virtual bool StartLaserScan(QSerialPort *serial);
    virtual bool StopLaserScan(QSerialPort *serial);
    virtual void DataParsing(const QByteArray &data);

    virtual void setDeviceSnCallback(std::function<void(const TYPE_LASER_ type, QString sn)> callback)
    {
        EAIDevSnFun = callback;
    };
    virtual void setDataCallback(std::function<void(std::vector<W_DataScan>)> callback)
    {
        DataFun = callback;
    }
    virtual QString GetOtherMessage()
    {
        return other_message;
    }
    bool sendCommand(QSerialPort *serial,
                     uint8_t cmd,
                     const void *payload = nullptr,
                     size_t payloadsize = 0);

private:
    bool sendData(QSerialPort *serial, const uint8_t *data, size_t size);
    bool sendCommand(QSerialPort *serial,
                    uint8_t addr,
                    uint8_t cmd,
                    const void *payload = nullptr,
                    size_t payloadsize = 0);
    bool waitPackage(const uint8_t *data, size_t len);
    bool original_parsing(gs2_node_package package);
    void angTransform(uint16_t dist, int n, double *dstTheta, uint16_t *dstDist);

    double normalize_angle_positive(double angle)
    {
        return fmod(fmod(angle, 2.0 * M_PI) + 2.0 * M_PI, 2.0 * M_PI);
    }
    double normalize_angle(double angle)
    {
        double a = normalize_angle_positive(angle);

        if (a > M_PI) {
            a -= 2.0 * M_PI;
        }
        return a;
    }
    bool isRangeValid(double reading) const
    {
        if (reading >= m_MinRange && reading <= m_MaxRange)
        {
            return true;
        }

        return false;
    }
    double from_degrees(double degrees)
    {
        return degrees * M_PI / 180.0;
    }

    uint64_t getClockTimeStamp_ms(void)
    {
        struct timespec time_data_in;
        clock_gettime(CLOCK_MONOTONIC, &time_data_in);
        return (uint64_t)(time_data_in.tv_sec * 1000 + time_data_in.tv_nsec / 1000000);
    };

private:
    std::vector<uint8_t> data_buf_;
    std::vector<uint8_t> init_buf_;
    gs2_node_package package; ///< 带信号质量协议包
    uint8_t moduleNum{0};
    bool m_intensities;//
    float m_AngleOffset{0.0};
    float m_MaxRange{1000};
    float m_MinRange{30};
    float m_MinAngle{-180};
    float m_MaxAngle{180};
    bool parameter_flag{false};
    uint64_t last_time;
    QString other_message;
    bool get_version{false};

    double  d_compensateK0[PackageMaxModuleNums];
    double  d_compensateK1[PackageMaxModuleNums];
    double  d_compensateB0[PackageMaxModuleNums];
    double  d_compensateB1[PackageMaxModuleNums];
    uint16_t  u_compensateK0[PackageMaxModuleNums];
    uint16_t  u_compensateK1[PackageMaxModuleNums];
    uint16_t  u_compensateB0[PackageMaxModuleNums];
    uint16_t  u_compensateB1[PackageMaxModuleNums];
    double  bias[PackageMaxModuleNums];

    std::function<void(const TYPE_LASER_, QString)> EAIDevSnFun = nullptr;
    std::function<void(std::vector<W_DataScan>)> DataFun = nullptr;
};

#endif // EAILINE_H
