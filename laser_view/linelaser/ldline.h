#ifndef LDLINE_H
#define LDLINE_H
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "llsdk_base.h"
#include "lib/pointdata.h"

#define THIS_DEVICE_ADDREESS 0x01

typedef enum {
    PACK_GET_DISTANCE = 0x02,               /*Frame ID of distance data*/
    PACK_RESET_SYSTEM = 0x0D,								/*Frame ID of distance reset system*/
    PACK_STOP = 0x0F,		                		/*Frame ID of stop distance data transmission*/
    PACK_ACK = 0x10,												/*Frame ID of ACK*/
    PACK_GET_COE = 0x12,	                	/*Frame ID of Get parameters*/
    PACK_VERSION = 0x14,	                	/*Frame ID of Get Machine version*/
    PACK_VIDEO_SIZE = 0x15,	                /*Frame ID of Get camera resolution*/
    PACK_CONFIG_ADDRESS = 0x16,             /*Frame ID of Configure address*/
    PACK_GET_DISTANCE_TRIG_MODE = 0x26,     /*Frame ID of trig distance data*/
    PACK_SET_GROUND_DISTANCE = 0x28,        /*Frame ID of set ground distance*/
}PackageIDTypeDef;

struct TRData {
    uint8_t device_address;
    uint8_t pack_id;
    uint16_t chunk_offset;
    std::vector<uint8_t> data;
};

class LDLine : public LLSDK_BASE
{
    const uint32_t LEADING_CODE = 0xAAAAAAAA;
    const uint32_t HEADER_LEN = 4; //device_address , pack_id , chunk_offset len
    const uint32_t EXTRA_LEN = 11;
public:
    LDLine();
    ~LDLine();

    bool initLaserScan(QSerialPort *serial) override;
    bool StartLaserScan(QSerialPort *serial) override;
    bool StopLaserScan(QSerialPort *serial) override;
    void DataParsing(const QByteArray &data) override;
    void setDeviceSnCallback(std::function<void(const TYPE_LASER_ type, QString sn)> callback) override
    {
        LdDevSnFun = callback;
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
    uint64_t getClockT_ms(void)
    {
        struct timespec time_data_in;
        clock_gettime(CLOCK_MONOTONIC, &time_data_in);
        return (uint64_t)(time_data_in.tv_sec * 1000 + time_data_in.tv_nsec / 1000000);
    };
    uint8_t CalCheckSum(const uint8_t *data, uint16_t len)
    {
        uint8_t checksum = 0;

        for (uint16_t i = 0; i < len; i++)
        {
            checksum += *data++;
        }

        return checksum;
    }
    bool Pack(const TRData &in, std::vector<uint8_t> &out);
    bool sendData(QSerialPort *serial, const uint8_t *data, size_t size);
    bool SendCmd(QSerialPort* port, uint8_t address, uint8_t id);
    const TRData *Unpack(const uint8_t *data, uint32_t len);
    bool AnalysisTRNetByte(uint8_t byte);
    bool Transform(const TRData *tr_data);
    bool UnpackData(const uint8_t *data, uint32_t len);

private:
    QString other_message;
    std::function<void(const TYPE_LASER_, QString)> LdDevSnFun = nullptr;
    std::function<void(std::vector<W_DataScan>)> DataFun = nullptr;
    const float A_FOV = 100;

    const TRData* tr_data_;
    TRData tr_unpack_data_;
    uint32_t parse_data_len_;

    uint64_t mlast_time;
    bool parameters_ready_{false};
    bool is_addr_ready_{false};
    bool get_Version{false};
    uint32_t coe_[35];
    uint16_t coe_u_;
    uint16_t coe_v_;
};

#endif // LDLINE_H
