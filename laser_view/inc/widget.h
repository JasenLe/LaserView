#ifndef WIDGET_H
#define WIDGET_H
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include <QContextMenuEvent>
#include <QTextEdit>
#include <QColorDialog>
#include <QKeyEvent>
#include <QMutex>
#include <QProcess>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include "radar/lpkg.h"
#include "radar/HCbase/hclidar.h"
#include "radar/BZbase/lidar_protocol.h"
#include "ui_widget.h"
#include "comreadthread.hpp"
#include "linelaser/llsdk.h"
#include "IniFile.h"
#include "datadisplaywindow.h"

#define MY_WINDOWTITLE "Laser View 3.1"
#define SOFTSC "SoftwareSettingsCache.ini"
#define SCAN_SET

struct Send_mem_
{
    std::vector<std::string> list;
    int now_idx;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    Ui::Widget *get_ui(){ return this->ui; }
    //初始化页面显示信息
    void SetInitFromMemory(void);
    //数据显示信息入口
    void VDATAcreatRows(std::vector<W_DataScan> data);
    void VDATAcreatHor(void);
    //清除用户自定义设置恢复默认信息
    bool DeleteMemoryFile(void);
    //修改控制区背景颜色
    void ChangeCSBackgroundColor(QColor _color);
    //OC按键样式
    void OCButtonColor(QColor _color, bool stu);
    //SY按键样式
    void SYButtonColor(QColor _color, SYB stu);
    //显示设备SN信息
    void setDeviceSn(const device_sn_t devSn);
    void setDeviceSn(const TYPE_LASER_ type, QString sn);
    //检测端口接入情况
    void checkPorts();
    void DelLaserObj();
    //点云放置
    void pushPoint(std::vector<W_DataScan> data);
    //显示画布
    QPen MDotLinePen(const QColor &color, qreal width,
                      Qt::PenCapStyle c = Qt::RoundCap, Qt::PenJoinStyle j = Qt::RoundJoin);
    QPixmap paintWidget();
    QPixmap paintWidget_time();
    //描绘文案（文案角度自定义）
    bool custom_drawText(QPainter *MPainter, float x, float y, float w, float h, float angle, float font_s, const QString &str);
    void normal_drawText(QPainter *MPainter, float  x, float y, float w, float h, float font_s, const QString &str, bool fl=false);
    // double Contrast(const QColor &color1, const QColor &color2)
    // {
    //     // 计算两个颜色的对比度  L = 0.2126 * R + 0.7152 * G + 0.0722 * B
    //     double contrastRatio = (0.2126 * color1.red() + 0.7152 * color1.green() + 0.0722 * color1.blue() + 0.05)/
    //                            (0.2126 * color2.red() + 0.7152 * color2.green() + 0.0722 * color2.blue() + 0.05);

    //     return contrastRatio;
    // }
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    //关闭程序
    void closeEvent(QCloseEvent *event)
    {
        if (this->windowState() & Qt::WindowMaximized)
        {
            qDebug() << "窗口处于最大化状态";
        }
        else
        {
            setIni<int>(WWIDTH, this->width());
            setIni<int>(WHEIGHT, this->height());
        }
        setIni<int>(DEVICE, ui->DeviceBox->currentIndex());
        setIni<int>(BAND, ui->bandBox->currentIndex());
        closeWindow();
        DelLaserObj();
        QWidget::closeEvent(event);
    }
protected slots:
    void timerTimeOut();

private:
    QPropertyAnimation* NewAnimation();
    void MyAnimationShow();
    void MyAnimationRetract();

    float getWidth();
    float getHeight();
    float getDiameter();
    float getRadius();
    QPoint getPoint(bool original=false);
    void m_resizeEvent(void);
    void Resetdeviation();
    void closeWindow();
    void HistoryData(QString fileName);
    bool SaveNewFile(H_file_data_ data);

    void set_filter(bool en, FILTER_S _select)
    {
        if (en)
        {
            if (lidar != nullptr)
                lidar->SetFilter(*(uint16_t*)&_select);
            if (hclidar != nullptr)
                hclidar->SetFilter(*(uint16_t*)&_select);
            if (Dbzlidar != nullptr)
                Dbzlidar->SetFilter(*(uint16_t*)&_select);
        }
        else
        {
            if (lidar != nullptr)
                lidar->SetFilter(0);
            if (hclidar != nullptr)
                hclidar->SetFilter(0);
            if (Dbzlidar != nullptr)
                Dbzlidar->SetFilter(0);
        }
    }

    template<typename T>
    T getIni(const string &key, T value)
    {
        QString filename = appPath+"/"+SOFTSC;
        std::string openfile = filename.toLocal8Bit().toStdString();
        IniFileSTL m_file(openfile);

        /*******************int***********************/
        //WIDGET
        if (key.compare(WWIDTH) == 0 || key.compare(WHEIGHT) == 0)
        {
            return m_file.ReadValue(WIDGET, key, value);
        }
        //CONSOLE
        else if (key.compare(DEVICE) == 0 || key.compare(BAND) == 0 ||
                 key.compare(AUTOCLOCE) == 0 || key.compare(TESTON) == 0 || key.compare(TESTOFF) == 0 ||
                 key.compare(LOCK) == 0 || key.compare(DATANUM) == 0)
            return m_file.ReadValue(CONSOLE, key, value);
        //BACKGROUND
        else if (key.compare(BCOLOR) == 0 || key.compare(CSCOLOR) == 0)
            return m_file.ReadValue(BACKGROUND, key, value);
        //P_COOR
        else if (key.compare(CWCCW) == 0 || key.compare(ROTATE) == 0)
            return m_file.ReadValue(P_COOR, key, value);
        //INDICATORLINE
        else if (key.compare(INDICATORLINE) == 0)
            return m_file.ReadValue(INDICATORLINE, "EN", value);
        else if (key.compare(LINEDIS) == 0 || key.compare(LINEANGLE) == 0 || key.compare(LINECONF) == 0 || key.compare(LINECOLOR) == 0)
            return m_file.ReadValue(INDICATORLINE, key, value);

        //FLITER
        else if (key.compare(FLITER) == 0)
            return m_file.ReadValue(FLITER, "EN", value);
        else if (key.compare(F_SMOOTH) == 0 || key.compare(F_BILATERAL) == 0 || key.compare(F_TAIL) == 0 ||
                 key.compare(F_INTENSITY) == 0 || key.compare(F_NEAR) == 0 || key.compare(F_NOISE) == 0 ||
                 key.compare(F_TINE) == 0 || key.compare(F_WANDER) == 0 || key.compare(F_SHADOWS) == 0 || key.compare(F_MEDIAN) == 0)
        {       
            return m_file.ReadValue(FLITER, key, value);
        }
        //PONITPIXEL
        else if (key.compare(PCOLOR) == 0)
            return m_file.ReadValue(PONITPIXEL, key, value);
        /*******************float***********************/
        //PONITPIXEL
        else if (key.compare(PPIXEL) == 0)
            return m_file.ReadValue(PONITPIXEL, key, value);
        /*******************String***********************/
        //SEND
        else if (key.compare(SDATA) == 0)
            return m_file.ReadValue(SEND, key, value);

        return value;
    }

    template<typename T>
    bool setIni(const string &key, T value)
    {
        QString filename = appPath+"/"+SOFTSC;
        std::string openfile = filename.toLocal8Bit().toStdString();
        IniFileSTL m_file(openfile);

        /*******************int***********************/
        //WIDGET
        if (key.compare(WWIDTH) == 0 || key.compare(WHEIGHT) == 0)
        {
            m_file.WriteValue(WIDGET, key, value);
        }
        //CONSOLE
        else if (key.compare(DEVICE) == 0 || key.compare(BAND) == 0 ||
                 key.compare(AUTOCLOCE) == 0 || key.compare(TESTON) == 0 || key.compare(TESTOFF) == 0 ||
                 key.compare(LOCK) == 0 || key.compare(DATANUM) == 0)
            m_file.WriteValue(CONSOLE, key, value);
        //BACKGROUND
        else if (key.compare(BCOLOR) == 0 || key.compare(CSCOLOR) == 0)
            m_file.WriteValue(BACKGROUND, key, value);
        //P_COOR=
        else if (key.compare(CWCCW) == 0 || key.compare(ROTATE) == 0)
            m_file.WriteValue(P_COOR, key, value);
        //INDICATORLINE
        else if (key.compare(INDICATORLINE) == 0)
            m_file.WriteValue(INDICATORLINE, "EN", value);
        else if (key.compare(LINEDIS) == 0 || key.compare(LINEANGLE) == 0 || key.compare(LINECONF) == 0 || key.compare(LINECOLOR) == 0)
            m_file.WriteValue(INDICATORLINE, key, value);

        //FLITER
        else if (key.compare(FLITER) == 0)
            m_file.WriteValue(FLITER, "EN", value);
        else if (key.compare(F_SMOOTH) == 0 || key.compare(F_BILATERAL) == 0 || key.compare(F_TAIL) == 0 ||
                 key.compare(F_INTENSITY) == 0 || key.compare(F_NEAR) == 0 || key.compare(F_NOISE) == 0 ||
                 key.compare(F_TINE) == 0 || key.compare(F_WANDER) == 0 || key.compare(F_SHADOWS) == 0 || key.compare(F_MEDIAN) == 0)
        {
            m_file.WriteValue(FLITER, key, value);
        }
        //PONITPIXEL
        else if (key.compare(PCOLOR) == 0)
            m_file.WriteValue(PONITPIXEL, key, value);
        /*******************float***********************/
        //PONITPIXEL
        else if (key.compare(PPIXEL) == 0)
            m_file.WriteValue(PONITPIXEL, key, value);
        /*******************string***********************/
        //SEND
        else if (key.compare(SDATA) == 0)
            m_file.WriteValue(SEND, key, value);

        return m_file.WriteFile();
    }

private slots:
    void on_Button_Open_clicked();
    void on_Button_Close_clicked();
    void on_Button_Start_clicked();
    void on_Button_Standby_clicked();
    void on_Button_send_clicked();

private:
    Ui::Widget *ui = nullptr;
    QSerialPort *serialPort = nullptr;
    LPkg *lidar = nullptr;
    LLSDK *linelaser = nullptr;
    HCLidar *hclidar = nullptr;
    nvistar::LidarProtocol *Dbzlidar = nullptr;
    DataDisplayWindow *window = nullptr;
    const DataScan_point Side_head{0, 0};
    const DataScan_point Side_end{0, 10};
    QString OpenedCom;
    QString OpenenType;
    int Devicebox_Idx{0};
    int BandBox_Idx{0};
    int linegetTypeTimeOut{0};

    QTimer *timer = nullptr;
    QPoint point;
    int i_diameter{0};
    double d_angle{0.0};
    QList<QPixmap> list_pixmap;
    std::vector<W_DataScan> m_scandata;
    float Width_offset{200 + Side_head.x};
    DataScan_point deviation{.x=Width_offset, .y=Side_head.y};
    W_DataScan measure_point{0,0,0,0};
    DataScan_point point_deviation{0,0};
    float Display_factor{1.0};
    float Display_factor_last{1.0};
    QColor m_point_color{Qt::red};
    QColor m_line_color{170,170,255};
    QColor m_background_color{Qt::white};
    QColor CS_background_color{220,220,220};
    drag_point drag_L{.start={-1, -1}, .end={-1, -1}, .flag=false};
    drag_point drag_R{.start={-1, -1}, .end={-1, -1}, .flag=false};
    DataScan_point moving;
    Send_mem_ lineEditSlist;
    SYB SYButton_stu{SYB::none};
    int Widget_width{800};
    int Widget_height{610};
    DataScan_point MwindowSize{-1, -1};
    DataScan_point MwindowPos{.x=-1, .y=-1};

    std::vector<float> line_speed;
    bool Show_indicator_line{true};
    bool Show_indicator_distance{true};
    bool Show_indicator_angle{true};
    bool Show_indicator_confidence{true};
    bool Enable_filte{true};
    bool p_flag{false};//极坐标旋转反向
    int rot_angle{-90}; //极坐标角度旋转
    FILTER_S filter_select{0x0001};
    float m_point_pixel{1.0};
    bool contextMenu_miss_flag{false};
    bool move_miss_flag{false};
    bool history_file_opened{false};
    bool CSwindows_flag{true};
    H_file_data_ history_file_data;
    H_file_data_ save_temp_data;

    QString appPath;
    QMutex save_data_mutex;

    bool my_ctrl{false};
    int test_count{0};
    int test_datanum{0};
    int set_datanum{7};
    bool test_lock{false};

    QItemSelectionModel *selectionModel;
    bool updata_select{false};

    QLabel *ctrl = nullptr;
    DataScan_point ctrlSize{15, 60};
    DataScan_point ctrlPoint{0, 20};
    QString ctrl_o_color;
    QString ctrl_c_color;
    QLabel *ctrlAutoClose = nullptr;
    bool ctrlAuto_flag{true};
    int ctrlAuto_time = -1;
    const DataScan_point ctrlAutoCloseSize{15, 15};
    QPropertyAnimation *animation = nullptr;
    QLabel *statusPonit = nullptr;

    const int timerTime{20}; //20ms
    int testOpen{2};
    int testOff{1};
};
#endif // WIDGET_H
