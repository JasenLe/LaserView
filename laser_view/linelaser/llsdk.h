#ifndef LLSDK_H
#define LLSDK_H
#include "eailine.h"
#include "rsline.h"
#include "llsdk_base.h"
#include "yxline.h"
#include "ldline.h"

class LLSDK : public LLSDK_BASE
{
public:
    LLSDK();
    ~LLSDK();

    bool initLaserScan(QSerialPort *serial) override;
    bool StartLaserScan(QSerialPort *serial) override;
    bool StopLaserScan(QSerialPort *serial) override;
    void DataParsing(const QByteArray &data) override;

    void setDeviceSnCallback(std::function<void(const TYPE_LASER_ type, QString sn)> callback) override
    {
        DevSnFun = callback;
    }

    void setDataCallback(std::function<void(std::vector<W_DataScan>)> callback) override
    {
        DataFun = callback;
    }
    QString GetOtherMessage() override
    {
        if (laser_type == TYPE_LASER_EAI)
        {
            return EAIllaser.GetOtherMessage();
        }
        else if (laser_type == TYPE_LASER_RS || laser_type == TYPE_LASER_RS_NEW
                   || laser_type == TYPE_LASER_RS_XVB02)
        {
            return RSllaser.GetOtherMessage();
        }
        else if (laser_type == TYPE_LASER_YX)
        {
            return YXllaser.GetOtherMessage();
        }
        else if (laser_type == TYPE_LASER_LD)
        {
            return LDllaser.GetOtherMessage();
        }
        else
        {
            return "";
        }
    }

    void outside_set_type(QSerialPort *serial, TYPE_LASER_ _type=TYPE_LASER_EAI)
    {
        laser_type = _type;
        qDebug() << "outside_set_type" << laser_type;
        if (DevSnFun != nullptr)
            DevSnFun(laser_type, nullptr);
        initLaserScan(serial);
    }

    void SetFilter(bool filter)
    {
        filter_select = filter;
    }
private:
    std::vector<W_DataScan> filter(std::vector<W_DataScan> indata)
    {
        std::vector<W_DataScan> out_list;
        int contrastnum = 4;
        float ranges_t = 15; // 0.0045
        float angle_t = 0.8;    // 分辨率0.6

        for (size_t i = 0; i < indata.size(); i++)
        {
            uint16_t casual_cunt = 0;
            float ranges, angles_now, angles_next;
            float now_angles = indata[i].angles_;
            float now_ranges = indata[i].ranges_;
            for (int j = i - 1; j >= 0; j--)
            {
                ranges = indata[j].ranges_;
                angles_now = indata[j].angles_;
                angles_next = indata[j + 1].angles_;
                if ((fabs(now_ranges - ranges) < ranges_t) && (fabs(angles_next - angles_now) < angle_t))
                {
                    casual_cunt++;
                    continue;
                }
                else
                {
                    break;
                }
            }
            for (size_t j = i + 1; j < indata.size(); j++)
            {
                ranges = indata[j].ranges_;
                angles_now = indata[j].angles_;
                angles_next = indata[j - 1].angles_;
                if ((fabs(now_ranges - ranges) < ranges_t) && (fabs(angles_next - angles_now) < angle_t))
                {
                    casual_cunt++;
                    continue;
                }
                else
                {
                    break;
                }
            }
            if (casual_cunt >= contrastnum)
            {
                out_list.push_back(indata[i]);
            }
        }

        return out_list;
    }

private:
    std::function<void(const TYPE_LASER_, QString)> DevSnFun = nullptr;
    std::function<void(std::vector<W_DataScan>)> DataFun = nullptr;
    RSLine RSllaser;
    EAILine EAIllaser;
    YXLine YXllaser;
    LDLine LDllaser;
    std::vector<uint8_t> cmd_buf_;
    bool filter_select;

    TYPE_LASER_ laser_type;
};

#endif // LLSDK_H
