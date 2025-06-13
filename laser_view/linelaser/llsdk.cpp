#include "llsdk.h"

LLSDK::LLSDK()
    :laser_type(TYPE_LASER_NONE)
{
    //RS
    RSllaser.setDeviceSnCallback([this](const TYPE_LASER_ type, QString sn)
                                {
                                    if (DevSnFun != nullptr)
                                        DevSnFun(type, sn);

                                    if (type == TYPE_LASER_RS)
                                        laser_type = TYPE_LASER_RS;
                                    else if (type == TYPE_LASER_RS_NEW)
                                        laser_type = TYPE_LASER_RS_NEW;
                                    else if (type == TYPE_LASER_RS_XVB02)
                                        laser_type = TYPE_LASER_RS_XVB02;
                                    else
                                        laser_type = TYPE_LASER_EAI;
                                });
    RSllaser.setDataCallback([this](std::vector<W_DataScan> data)
                             {
                                if (laser_type == TYPE_LASER_RS_NEW || laser_type == TYPE_LASER_RS
                                    || laser_type == TYPE_LASER_RS_XVB02)
                                {
                                    if (DataFun != nullptr)
                                    {
                                        if (filter_select)
                                            DataFun(filter(data));
                                        else
                                            DataFun(data);
                                    }
                                }
                             });

    //EAI
    EAIllaser.setDeviceSnCallback([this](const TYPE_LASER_ type, QString sn)
                                 {
                                    laser_type = type;
                                    if (DevSnFun != nullptr)
                                        DevSnFun(type, sn);
                                 });
    EAIllaser.setDataCallback([this](std::vector<W_DataScan> data)
                             {
                                 if (laser_type == TYPE_LASER_EAI)
                                 {
                                     if (DataFun != nullptr)
                                     {
                                         if (filter_select)
                                             DataFun(filter(data));
                                         else
                                             DataFun(data);
                                     }
                                 }
                             });

    //YX
    YXllaser.setDeviceSnCallback([this](const TYPE_LASER_ type, QString sn)
                                  {
                                      laser_type = type;
                                      if (DevSnFun != nullptr)
                                          DevSnFun(type, sn);
                                  });
    YXllaser.setDataCallback([this](std::vector<W_DataScan> data)
                              {
                                  if (laser_type == TYPE_LASER_YX)
                                  {
                                      if (DataFun != nullptr)
                                      {
                                          if (filter_select)
                                              DataFun(filter(data));
                                          else
                                              DataFun(data);
                                      }
                                  }
                              });

    //LD
    LDllaser.setDeviceSnCallback([this](const TYPE_LASER_ type, QString sn)
                                 {
                                     laser_type = type;
                                     if (DevSnFun != nullptr)
                                         DevSnFun(type, sn);
                                 });
    LDllaser.setDataCallback([this](std::vector<W_DataScan> data)
                             {
                                 if (laser_type == TYPE_LASER_LD)
                                 {
                                     if (DataFun != nullptr)
                                     {
                                         if (filter_select)
                                             DataFun(filter(data));
                                         else
                                             DataFun(data);
                                     }
                                 }
                             });
}
LLSDK::~LLSDK() {}

bool LLSDK::initLaserScan(QSerialPort *serial)
{
    if (laser_type == TYPE_LASER_YX || laser_type == TYPE_LASER_NONE)
        YXllaser.initLaserScan(serial);

    if (laser_type == TYPE_LASER_RS || laser_type == TYPE_LASER_RS_NEW
        || laser_type == TYPE_LASER_RS_XVB02 || laser_type == TYPE_LASER_NONE)
        RSllaser.initLaserScan(serial);

    if (laser_type == TYPE_LASER_EAI || laser_type == TYPE_LASER_NONE)
        EAIllaser.initLaserScan(serial);

    if (laser_type == TYPE_LASER_LD || laser_type == TYPE_LASER_NONE)
        LDllaser.initLaserScan(serial);

    return true;
}

bool LLSDK::StartLaserScan(QSerialPort *serial)
{
    bool res = false;
    if (laser_type == TYPE_LASER_RS || laser_type == TYPE_LASER_RS_NEW
        || laser_type == TYPE_LASER_RS_XVB02)
        res = RSllaser.StartLaserScan(serial);
    else if(laser_type == TYPE_LASER_EAI)
        res = EAIllaser.StartLaserScan(serial);
    else if (laser_type == TYPE_LASER_YX)
        res = YXllaser.StartLaserScan(serial);
    else if (laser_type == TYPE_LASER_LD)
        res = LDllaser.StartLaserScan(serial);

    return res;
}

bool LLSDK::StopLaserScan(QSerialPort *serial)
{
    bool res = false;
    if (laser_type == TYPE_LASER_RS || laser_type == TYPE_LASER_RS_NEW
        || laser_type == TYPE_LASER_RS_XVB02)
        res = RSllaser.StopLaserScan(serial);
    else if(laser_type == TYPE_LASER_EAI)
        res = EAIllaser.StopLaserScan(serial);
    else if(laser_type == TYPE_LASER_YX)
        res = YXllaser.StopLaserScan(serial);
    else if(laser_type == TYPE_LASER_LD)
        res = LDllaser.StopLaserScan(serial);

    return res;
}

void LLSDK::DataParsing(const QByteArray &data)
{
    if (laser_type == TYPE_LASER_EAI || laser_type == TYPE_LASER_NONE)
    {
        EAIllaser.DataParsing(data);
    }

    if (laser_type == TYPE_LASER_RS || laser_type == TYPE_LASER_RS_NEW
        || laser_type == TYPE_LASER_RS_XVB02 || laser_type == TYPE_LASER_NONE)
    {
        RSllaser.DataParsing(data);
    }

    if (laser_type == TYPE_LASER_YX || laser_type == TYPE_LASER_NONE)
    {
        YXllaser.DataParsing(data);
    }

    if (laser_type == TYPE_LASER_LD || laser_type == TYPE_LASER_NONE)
    {
        LDllaser.DataParsing(data);
    }
}
