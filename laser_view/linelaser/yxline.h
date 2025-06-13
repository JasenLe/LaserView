#ifndef YXLINE_H
#define YXLINE_H
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include "llsdk_base.h"

// #define  PACK_SHORT

#define YXLI_SYNC_HEAD      0xFFFFFF

#define YXLI_CMD_ADDRESS   0x60
#define YXLI_CMD_VERSION   0x62
#define YXLI_CMD_SCAN      0x63
#define YXLI_CMD_STOP      0x64
#define YXLI_CMD_REBOOT    0x67

#define PackageSampleMaxLngth 160
#define PackagePaidBytes      7

struct cmd_pack_yx {
    uint8_t hByte0;
    uint8_t hByte1;
    uint8_t hByte2;
    uint8_t address;
    uint8_t cmd_;
    uint8_t features_MSB;
    uint8_t features_LSB;
} __attribute__((packed)) ;

#ifdef PACK_SHORT
struct PackageNode_short {
    uint8_t PakageX;
    uint8_t PakageZ;
} __attribute__((packed));

struct node_package {
    cmd_pack_yx Head;
    PackageNode_short  packageSample[PackageSampleMaxLngth];
    uint8_t  checkSum;
} __attribute__((packed)) ;
#else
struct PackageNode {
    uint8_t PakageX_MSB;
    uint8_t PakageX_LSB;
    uint8_t PakageZ_MSB;
    uint8_t PakageZ_LSB;
} __attribute__((packed));

struct node_package {
    cmd_pack_yx Head;
    PackageNode  packageSample[PackageSampleMaxLngth];
    uint8_t  checkSum;
} __attribute__((packed)) ;
#endif

class YXLine : public LLSDK_BASE
{
public:
    YXLine();
    ~YXLine();

    bool initLaserScan(QSerialPort *serial) override;
    bool StartLaserScan(QSerialPort *serial) override;
    bool StopLaserScan(QSerialPort *serial) override;
    void DataParsing(const QByteArray &data) override;
    void setDeviceSnCallback(std::function<void(const TYPE_LASER_ type, QString sn)> callback) override
    {
        YxDevSnFun = callback;
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
    uint8_t get_checksum(uint8_t *dataPack, uint16_t dataSize)
    {
        uint8_t sum = 0;

        for (uint16_t i = 3; i < dataSize+sizeof(cmd_pack_yx); i++)
        {
            sum += dataPack[i];
            sum &= 0xFF;
        }

        return sum;
    }
    uint64_t getClockT_ms(void)
    {
        struct timespec time_data_in;
        clock_gettime(CLOCK_MONOTONIC, &time_data_in);
        return (uint64_t)(time_data_in.tv_sec * 1000 + time_data_in.tv_nsec / 1000000);
    };
    bool ParsingPackage(const uint8_t *data, size_t len);
    bool original_parsing(node_package package);
    bool sendData(QSerialPort *serial, const uint8_t *data, size_t size);
    bool sendCmd(QSerialPort *serial, uint8_t addr, uint8_t cmd);
    bool sendCmd(QSerialPort *serial, uint8_t cmd);

private:
    QString other_message;
    std::function<void(const TYPE_LASER_, QString)> YxDevSnFun = nullptr;
    std::function<void(std::vector<W_DataScan>)> DataFun = nullptr;

    std::vector<uint8_t> data_buf_;
    node_package package;
    uint8_t moduleNum{0};
    bool get_version{false};
    uint64_t mlast_time;
    bool LaserStauts{false};
};

#endif // YXLINE_H
