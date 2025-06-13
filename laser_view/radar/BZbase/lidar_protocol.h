/*
 * @Version      : V1.0
 * @Date         : 2024-10-14 11:15:28
 * @Description  : lidar protocol functon, and the protocol support multi type lidar
 */
#ifndef __LIDAR_PROTOCOL_H__
#define __LIDAR_PROTOCOL_H__
#include <QWidget>
#include <stdint.h>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstring>
#include <mutex>
#include <QSerialPort>
#include "../../inc/radarscan.h"
#include "../fullscanfilter.h"
#include "../morefilter.h"
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

#define HEAD_FLAG 0x5AA5
#define RUN_SCAN   0x01
#define STOP_SCAN  0x02
#define RESET_SCAN 0x03

namespace nvistar{

#ifndef DLL_EXPORT
  #ifdef _MSC_VER
    #define DLL_EXPORT __declspec(dllexport)
  #else
    #define DLL_EXPORT
  #endif 
#endif 

//callback function,it to be serailport,socket,etc...
typedef struct{
  std::function<int(const uint8_t* data,int length)> write;
  std::function<int(uint8_t *data,int max_length)>  read;
  std::function<void(void)> flush;
}lidar_transmit_interface_t;
//callback function
typedef struct{
  lidar_transmit_interface_t  transmit;
  std::function<uint64_t(void)> get_timestamp = nullptr;
}lidar_interface_t;
//single point info 
typedef struct{
    double    angle;
    double    distance;
    double    intensity;
    uint64_t  timestamp;
}lidar_scan_point_t;
//point info for 1 period 
typedef struct{
  int       model_code;                   //lidar model code 
  std::vector<lidar_scan_point_t> points; //one period points 
  bool      intensity_flag;               //intensity?
  double    speed;                        //RPM
  int       error_code;                   //error code 
  uint64_t  timestamp_start;              //stamp start 
  uint64_t  timestamp_stop;               //stamp stop 
}lidar_scan_period_t;

class LidarProtocolImpl;     //forward declaration

class DLL_EXPORT LidarProtocol{
  public:
    //error code define
    typedef enum{
        ERROR_CODE_RESET = 0,
        ERROR_CODE_MOTOR_LOCK = 1,
        ERROR_CODE_UP_NO_POINT = 2,
        ERROR_CODE_MOTOR_SHORTCIRCUIT = 3, 
        ERROR_CODE_NO_DATA = 0xFE,
        ERROR_CODE_NONE = 0xFF,
    }lidar_error_code_t;
    //protocol type
    typedef enum{
        PROTOCOL_MODEL_NORMAL_NO_QUALITY = 0x0208,                 //normal protocol no quality
        PROTOCOL_MODEL_NORMAL_HAS_QUALITY = 0x0308,                //normal protocol has quality
        PROTOCOL_MODEL_YW_HAS_QUALITY = 0x070C,                    //yw protocol has quality
        PROTOCOL_MODEL_LD_HAS_QUAILIY = 0x2C54,                    //ld protocol has quality 
        PROTOCOL_MODEL_ROBOROCK_HAS_QUALITY = 0xFAA0,              //roborock protocol has quality 
        PROTOCOL_MODEL_ERROR_FAULT = 0x8008                        //protocol error code
    }lidar_protocol_model_t;

    //function callback define 
    typedef std::function<void(lidar_scan_period_t)> protocol_rawdata_output_callback;     //pointcloud callback 

    //function 
    LidarProtocol();
    ~LidarProtocol();
    void DataParsing(const QByteArray &data);
    bool DeviceOpen(QSerialPort *cmd_port);
    bool DeviceClose(QSerialPort *cmd_port);
    void MySleep(uint32_t ms)
    {
        QElapsedTimer timer;
        timer.start();

        while(timer.elapsed() < ms){
            // 处理其他任务
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }
    void setDataCallback(std::function<void(std::vector<W_DataScan>)> callback)
    {
        DataFun = callback;
    }
    void setDeviceSnCallback(std::function<void(const std::string)> callback)
    {
        SnFun = callback;
    }
    void SetFilter(uint16_t filter)
    {
        filter_select = filter;
    }
    QString GetOtherMessage()
    {
        return infoData;
    }
    void lidar_protocol_register(lidar_interface_t* api, protocol_rawdata_output_callback rawdata_output); //register communitcation api 
    void lidar_protocol_unregister();                                   //unregister 
  private:
    LidarProtocolImpl* _impl;  //pimpl function
    std::function<void(std::vector<W_DataScan>)> DataFun = nullptr;
    std::function<void(const std::string)> SnFun = nullptr;
    QString infoData{""};

    uint16_t filter_select{0x0};
    FullScanFilter full_scan_filter_;
    FilterPara para_inf_;
};

}

#endif


