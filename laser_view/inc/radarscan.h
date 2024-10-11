#ifndef RADARSCAN_H
#define RADARSCAN_H
#include <QWidget>
#include <QFileDialog>
#include <QPainter>
#include <QPen>
#include <QDebug>
#include <QLinearGradient>
#include <QTimer>
#include <QTime>
#include <QWheelEvent>
#include <string>
#include <vector>
#include <fstream>
#include<sstream>
#include <iostream>
#include <QFileDialog>

#define filter_smooth     0x0001
#define filter_bilateral  0x0002
#define filter_tail       0x0004
#define filter_intensity  0x0008
#define filter_near       0x0010
#define filter_noise      0x0020
#define filter_tine       0x0040
#define filter_wander     0x0080
#define filter_shadows    0x0100
#define filter_median     0x0200

#define A_TO_RAD(angle) ((angle)*M_PI / 180)
#define RAD_TO_A(angle) ((angle)*180 / M_PI)

struct W_DataScan
{
    float ranges_{0};
    float angles_{0};
    uint8_t intensity_{0};
    uint16_t speed{0};
    float x()
    {
        return ranges_ * cos(A_TO_RAD(angles_));
    };
    float y()
    {
        return ranges_ * sin(A_TO_RAD(angles_));
    };
};

struct DataScan_point
{
    float x{0};
    float y{0};
};

struct drag_point
{
    DataScan_point start;
    DataScan_point end;
    bool flag{false};
};

struct H_file_data_
{
    std::vector<std::vector<W_DataScan>> data;
    QString open_file_Tips;
    int now_idx;
};
#endif // RADARSCAN_H
