#ifndef RSLINE_H
#define RSLINE_H
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <cfloat>
#include "rsline_tool.h"
#include "llsdk_base.h"

#define XV90  0
#define XVB01 1
#define XVB02 2
#define XVY01 3

enum RSCommand
{
    CMD_START_MEASURE = 0x01,
    CMD_STOP_MEASURE = 0x02,
    CMD_GET_PARAME = 0x03,
    CMD_SET_DEV_ADDR = 0x04,
    CMD_SET_CURRENT = 0x05,
    CMD_GET_VERSION = 0x06,
    CMD_GET_SN = 0x0D,
    CMD_SET_BAUDRATE = 0x0E,
    CMD_GET_WAVE = 0x0f,
    CMD_OPEN_CAMERA = 0xE1,
    CMD_CLOSE_CAMERA = 0xE2,
    CMD_OPEN_LASER = 0xE3,
    CMD_CLOSE_LASER = 0xE4,
    CMD_CAMERA0_OPEN = 0xE5,
    CMD_CAMERA1_OPEN = 0xE6,
    CMD_GET_GRAY_SUM = 0xE7,
    CMD_USART_IMAGE_MODE = 0xEA,
    CMD_GET_LASER_POS = 0xEB,
    CMD_SET_GET_EXPOSURE = 0xEC,
    CMD_SET_GET_CAMERA_REG = 0xED,
    CMD_SET_GET_TYPE = 0xEF,
    CMD_IAP_SET_ADDR = 0xA0,
    CMD_IAP_UPDATE = 0xA1,
    CMD_IAP_REQUEST = 0xA2,
    CMD_IAP_END = 0xA3,
    CMD_RESTART = 0xFF
};

struct LineLaserDataScan
{
    float ranges_;
    float angles_;
    float intensity_;
};

class RSLine : public LLSDK_BASE
{
    typedef struct pro_com_
    {
        const uint8_t start_bit_[2] = {0xAA, 0xAA};
        uint8_t addr_;
        uint8_t cmd_;
        uint16_t offset_;
        uint16_t length_;
    }protocol_com_;
public:
    RSLine();
    ~RSLine();

    bool initLaserScan(QSerialPort *serial) override;
    bool StartLaserScan(QSerialPort *serial) override;
    bool StopLaserScan(QSerialPort *serial) override;
    void DataParsing(const QByteArray &data) override;
    void setDeviceSnCallback(std::function<void(const TYPE_LASER_ type, QString sn)> callback) override
    {
        RsDevSnFun = callback;
    }
    void setDataCallback(std::function<void(std::vector<W_DataScan>)> callback) override
    {
        DataFun = callback;
    }
    QString GetOtherMessage() override
    {
        return other_message;
    }

private:
    uint8_t check(const QByteArray data, size_t len);
    Message Unpack(const std::vector<uint8_t> &unkown_data, int &processed_num);
    std::vector<uint8_t> Pack(Message &msg);
    bool CalcDistance(const Message &msg);
    bool BCalcDistance(const Message& msg);
    bool ConfigAddress(const Message &msg);
    bool ConfigParam(const Message &msg);
    bool BConfigParam(const Message& msg);
    void TransformSignlePoint(const uint16_t dist, const int n, const uint8_t device_id);
    float computePointCloudX(
        float u,
        float v,
        float depth,
        std::vector<float> param);
    uint8_t AddrToDevId(uint8_t addr);
    uint8_t AddrToDevCount(uint8_t addr);
    constexpr size_t data_pos()
    {
        return offsetof(Protocol_vx90, data_);
    }
    bool StopWork(const Message &msg)
    {
        return true;
    }
    bool HandleElseTask(const Message &msg)
    {
        return true;
    }
    uint64_t getClockTime_ms(void)
    {
        struct timespec time_data_in;
        clock_gettime(CLOCK_MONOTONIC, &time_data_in);
        return (uint64_t)(time_data_in.tv_sec * 1000 + time_data_in.tv_nsec / 1000000);
    };

    bool LaserScanSendCMD(QSerialPort *serial, int addr, uint8_t cmd);
    bool UnpackData(const uint8_t *data, size_t len);

private:
    const uint8_t start_bit_[2] = {0xAA, 0xAA};
    uint16_t offset_{0};
    int device_addr_;
    bool is_addr_ready_{false};
    bool is_SN_ready_{false};
    bool intensity_need_{false};
    uint64_t last_time;
    QString other_message;
    bool get_Wave{false};
    bool get_Version{false};
    uint8_t is_param_ready_{255};
    uint8_t RS_type = XV90;

    Protocol_vx90 format_;
    std::vector<Point3f_t> pointcloud_;
    std::vector<std::vector<float>> param_;
    std::vector<int> param_type_;
    std::vector<std::vector<double>> Bparam_;
    std::vector<int> v_start_;
    std::vector<int> v_gap_;
    std::vector<uint8_t> rev_data_;
    std::vector<LineLaserDataScan> LaserDataScanData;
    bool laserdatascan_reday{false};
    uint64_t lineSensorSn{0x0};

    std::mutex _mutex_LaserData;

    std::function<void(const TYPE_LASER_, QString)> RsDevSnFun = nullptr;
    std::function<void(std::vector<W_DataScan>)> DataFun = nullptr;
};

#endif // RSLINE_H
