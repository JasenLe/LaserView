#ifndef FULLSCANFILTER_H
#define FULLSCANFILTER_H
#include <QWidget>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iostream>

#define   Rth    30   //相邻点距离阈值
#define   Ath    0.8  //相邻角度阈值
#define     pai      M_PI//3.1415926
#define     DEC      (pai/180)
#define     Rad      (180/pai)
#define     w_r       2   //划窗大小，5

struct PointData {
    // Polar coordinate representation
    float angle;         // Angle ranges from 0 to 359 degrees
    uint16_t distance;   // Distance is measured in millimeters
    uint8_t confidence;  // Confidence is 0 to 255
    // Cartesian coordinate representation
    double x;
    double y;
    uint64_t timeStamp;
    PointData(float angle, uint16_t distance, uint8_t confidence, uint64_t timeStamp, double x = 0,double y = 0)
    {
        this->angle = angle;
        this->distance = distance;
        this->confidence = confidence;
        this->x = x;
        this->y = y;
        this->timeStamp = timeStamp;
    }
    PointData() {}
    friend std::ostream &operator<<(std::ostream &os, const PointData &data)
    {
        os << data.angle << " " << data.distance << " " << (int)data.confidence
           << " " << data.x << " " << data.y;
        return os;
    }
};
typedef std::vector<PointData> Points2D;

typedef struct FilterParas
{
    int filter_type=0;    //滤波类型，参考enum FilterStrategy

    //平滑滤波
    int maxRange = 150; ///观测最远距离, 150mm，对15cm内数据进行平滑
    int minRange = 0;    ///观测最近距离,

        ////双边滤波
        //     int Sigma_D = 5;  ///全局方差，位置索引权重分配
        //     int Sigma_R = 3;  ///局部方差，像素值（距离）
        int Sigma_D = 5;  ///全局方差，位置索引权重分配
    int Sigma_R = 6;  ///局部方差，像素值（距离）

    //强度滤波
    int IntesntiyFilterRange = 70;  ///距离
    int Weak_Intensity_Th = 31;  //弱信号强度

    //拖尾滤波
    int Rotation = 10;  //转速，默认,10hz
    int level = 0; //滤波强度，默认，8°2个点
} FilterPara;


class FullScanFilter
{
public:
    enum FilterStrategy
    {
        FS_Smooth,      //平滑滤波
        FS_Bilateral,   //双边滤波
        FS_Tail,        //去拖尾滤波
        FS_Intensity,       //强度滤波
    };
public:
    FullScanFilter();
    ~FullScanFilter();
    void filter(const std::vector<PointData> &in,
                FilterPara ParaInf,
                std::vector<PointData> &out);

protected:
    void smooth_filter(const std::vector<PointData> &in,
                       FilterPara ParaInf,
                       std::vector<PointData> &out);

    void bilateral_filter(const std::vector<PointData> &in,
                          FilterPara ParaInf,
                          std::vector<PointData> &out);

    void tail_filter(const std::vector<PointData> &in,
                     FilterPara ParaInf,
                     std::vector<PointData> &out);

    void intensity_filter(const std::vector<PointData> &in,
                          FilterPara ParaInf,
                          std::vector<PointData> &out);


    bool isValidRange(FilterPara ParaInf, uint16_t current_data);

    void swap(uint16_t *a, uint16_t *b);
    void BubbleSort(uint16_t *data, int len);
protected:
    static const int FILTER_WINDOW_SIZE = 3; //5;

};

#endif // FULLSCANFILTER_H
