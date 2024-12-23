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
#include "radar/lpkg.h"
#include "radar/HCbase/hclidar.h"
#include "ui_widget.h"
#include "comreadthread.hpp"
#include "linelaser/llsdk.h"
#include "IniFile.h"
#include "datadisplaywindow.h"

#define MY_WINDOWTITLE "LB laser 3.1"
#define SOFTSC "SoftwareSettingsCache.ini"

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
    //清除用户自定义设置恢复默认信息
    bool DeleteMemoryFile(void);
    //修改控制区背景颜色
    void ChangeCSBackgroundColor(QColor _color);
    //OC按键样式
    void OCButtonColor(QColor _color, bool stu);
    //SY按键样式
    void SYButtonColor(QColor _color, uint8_t stu);
    //显示设备SN信息
    void setDeviceSn(const device_sn_t devSn);
    void setDeviceSn(const TYPE_LASER_ type, QString sn);
    //检测端口接入情况
    void checkPorts();
    void DelLaserObj();
    //点云放置
    void pushPoint(std::vector<W_DataScan> data);
    //显示画布
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
        setIniint(WWIDTH, this->width());
        setIniint(WHEIGHT, this->height());
        setIniint(DEVICE, ui->DeviceBox->currentIndex());
        setIniint(BAND, ui->bandBox->currentIndex());
        closeWindow();
        DelLaserObj();
        QWidget::closeEvent(event);
    }
protected slots:
    void timerTimeOut();

private:
    float getWidth();
    float getHeight();
    float getDiameter();
    float getRadius();
    QPoint getPoint(bool original=false);
    void m_resizeEvent(void);
    void Resetdeviation();
    int getIniint(const string &key, int value);
    float getInifloat(const string &key, float value);
    string getString(const string &key, string value);
    bool setIniint(const string &key, int value);
    bool setInifloat(const string &key, float value);
    bool setString(const string &key, string value);
    void closeWindow();
    void HistoryData(QString fileName);
    bool SaveNewFile(H_file_data_ data);

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
    DataScan_point measure_point{0,0};
    DataScan_point point_deviation{0,0};
    float Display_factor{1.0};
    float Display_factor_last{1.0};
    QColor m_point_color{Qt::red};
    QColor m_line_color{Qt::green};
    QColor m_background_color{Qt::white};
    QColor CS_background_color{255,251,242};
    drag_point drag_L{.start={-1, -1}, .end={-1, -1}, .flag=false};
    drag_point drag_R{.start={-1, -1}, .end={-1, -1}, .flag=false};
    DataScan_point moving;
    Send_mem_ lineEditSlist;
    uint8_t SYButton_stu{0};
    int Widget_width{800};
    int Widget_height{560};
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
    uint16_t filter_select{0x0001};
    float m_point_pixel{1.0};
    bool contextMenu_miss_flag{false};
    bool move_miss_flag{false};
    bool history_file_opened{false};
    bool CSwindows_flag{true};
    H_file_data_ history_file_data;
    H_file_data_ save_temp_data;

    QString appPath;
    QMutex save_data_mutex;

    bool test_flag{false};
    int test_count{0};
    uint64_t test_datanum{0};
};
#endif // WIDGET_H
