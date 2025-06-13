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

#define WIDGET  "Widget"
#define WWIDTH  "width"
#define WHEIGHT "height"

#define P_COOR "Polar_coordinates"
#define CWCCW  "p_flag"
#define ROTATE "rot_angle"

#define SEND   "Send"
#define SDATA  "S_data"

#define CONSOLE "Console"
#define DEVICE  "device"
#define BAND    "band"
#define AUTOCLOCE "autoclose"
#define TESTON    "TOn"
#define TESTOFF   "TOff"
#define LOCK      "lock"
#define DATANUM   "datanum"

#define BACKGROUND "backgrorund"
#define BCOLOR "backgrorund color"
#define CSCOLOR "CS bg color"

#define INDICATORLINE "Indicator Line"
#define LINEDIS       "distance"
#define LINEANGLE     "angle"
#define LINECONF      "confidence"
#define LINECOLOR     "linecolor"

#define PONITPIXEL "Point info"
#define PPIXEL     "pixel"
#define PCOLOR     "color"

#define FLITER      "Filter"
// #define FLITER_all  "Filter all"
#define F_SMOOTH    "smooth"
#define F_BILATERAL "bilateral"
#define F_TAIL      "tail"
#define F_INTENSITY "intensity"
#define F_NEAR      "near"
#define F_NOISE     "noise"
#define F_TINE      "tine"
#define F_WANDER    "wander"
#define F_SHADOWS   "shadows"
#define F_MEDIAN    "median"

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

enum SYB
{
    none = 0,
    START = 1,
    STANDBY = 2
};

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

struct FILTER_S {
    uint16_t _smooth:1;
    uint16_t _bilateral:1;
    uint16_t _tail:1;
    uint16_t _intensity:1;
    uint16_t _near:1;
    uint16_t _noise:1;
    uint16_t _tine:1;
    uint16_t _wander:1;
    uint16_t _shadows:1;
    uint16_t _median:1;
    uint16_t :1;

    FILTER_S(){};
    FILTER_S(uint16_t data)
    {
        *this = data;
    }
    FILTER_S &operator=(uint16_t data)
    {
        this->_smooth = (bool)(data & filter_smooth);
        this->_bilateral = (bool)(data & filter_bilateral);
        this->_tail = (bool)(data & filter_tail);
        this->_intensity = (bool)(data & filter_intensity);
        this->_near = (bool)(data & filter_near);
        this->_noise = (bool)(data & filter_noise);
        this->_tine = (bool)(data & filter_tine);
        this->_wander = (bool)(data & filter_wander);
        this->_shadows = (bool)(data & filter_shadows);
        this->_median = (bool)(data & filter_median);

        return *this;
    };
} __attribute__((packed));
#endif // RADARSCAN_H
