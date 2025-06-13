#include "inc/widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget), serialPort(new QSerialPort), timer(new QTimer),
    ctrl(new QLabel), ctrlAutoClose(new QLabel), statusPonit(new QLabel)
{
    ui->setupUi(this);
    this->setWindowTitle(MY_WINDOWTITLE);
    this->layout()->setContentsMargins(Side_head.x, Side_head.y, Side_end.x, Side_end.y);
    this->layout()->setSpacing(0); // 设置布局间没有空白间隔

    //crtl
    ctrl->setParent(this);;
    //ctrlAutoClose
    ctrlAutoClose->setParent(this);
    //statusPonit
    statusPonit->setParent(this);

    //安装目录地址
    appPath = QCoreApplication::applicationDirPath();
    SetInitFromMemory();

    //接收数据接口
    serialPort->setParent(this);
    connect(serialPort, &QSerialPort::readyRead, this, [this] () {
        QByteArray data = serialPort->readAll();
        // 处理接收到的数据
        if (data.size() > 0)
        {
            if (OpenenType.compare("LB_L") == 0 || OpenenType.compare("LB_LOther") == 0)
            {
                if (linelaser != nullptr)
                    linelaser->DataParsing(data);
            }
            else if (OpenenType.compare("LB_RHC") == 0)
            {
                if (hclidar != nullptr)
                    hclidar->readData(data);
            }
            else if (OpenenType.compare("LB_DBZ") == 0)
            {
                if (Dbzlidar != nullptr)
                    Dbzlidar->DataParsing(data);
            }
            else
            {
                if (lidar != nullptr)
                    lidar->DataParsing(data);
            }

            if (window != nullptr)
                window->InsertData(data);
        }
    });

    //页面定时刷新
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));
    timer->start(timerTime);

    // QProcess::startDetached("cmd", QStringList() << "/c" << appPath.replace("&", "^&") + "/laser_view使用说明.pdf");
}

Widget::~Widget()
{
    if (animation)
        delete animation;

    closeWindow();
    DelLaserObj();
    delete ctrl;
    delete ctrlAutoClose;
    delete statusPonit;
    delete serialPort;
    delete timer;
    delete ui;
}

void Widget::VDATAcreatRows(std::vector<W_DataScan> data)
{
    ui->tableWidget_scan->setRowCount(data.size());

    QHeaderView*headView = ui->tableWidget_scan->verticalHeader();
    headView->setSectionResizeMode(QHeaderView::Fixed);
    QFont font;
    font.setPointSize(5);
    font.setFamily("行楷");
    for (size_t i = 0; i < data.size(); i++)
    {
        font.setBold(true);
        QTableWidgetItem* item=new QTableWidgetItem(QString::number(i+1));
        item->setFont(font);
        ui->tableWidget_scan->setVerticalHeaderItem(i, item);

        font.setBold(false);
        QTableWidgetItem* item0=new QTableWidgetItem(QString::number(data[i].angles_, 'f', 2));
        item0->setFont(font);
        item0->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget_scan->setItem(i,0,item0);

        QTableWidgetItem* item1=new QTableWidgetItem(QString::number(data[i].ranges_, 'f', 0));
        item1->setFont(font);
        item1->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget_scan->setItem(i,1,item1);

        QTableWidgetItem* item2=new QTableWidgetItem(QString::number(data[i].intensity_));
        item2->setFont(font);
        item2->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget_scan->setItem(i,2,item2);
    }

}

void Widget::VDATAcreatHor(void)
{
    QStringList headList={"角度","距离","置信度"};
    ui->tableWidget_scan->setColumnCount(headList.count());

    QHeaderView*headView= ui->tableWidget_scan->horizontalHeader();//返回表头的视图
    headView->setSectionResizeMode(QHeaderView::Stretch);//可伸展

    QFont font;
    font.setPointSize(5);//设置字体大小
    font.setBold(true);//设置字体为粗体
    font.setFamily("行楷");//设置字体样式
    for(int i=0;i<ui->tableWidget_scan->columnCount();i++)
    {
        QTableWidgetItem*item=new QTableWidgetItem(headList[i]);
        item->setFont(font);
        // item->setForeground(Qt::black);//设置字体颜色
        ui->tableWidget_scan->setHorizontalHeaderItem(i,item);
    }
    QObject::connect(ui->tableWidget_scan, &QTableWidget::itemSelectionChanged,this, [&]()
                     {
                         QList<QTableWidgetItem*> items = ui->tableWidget_scan->selectedItems();
                         if (items.size() == 3)
                         {
                             if (items[0]->row() == items[1]->row() &&
                                 items[0]->row() == items[2]->row())
                             {
                                 // 整行被选中时的处理逻辑
                                 float angle_g = items[0]->text().toFloat();
                                 while (angle_g < 0)
                                     angle_g += 360;
                                 while (angle_g > 360)
                                     angle_g -= 360;
                                 measure_point.angles_ = angle_g;
                                 measure_point.ranges_ = items[1]->text().toFloat();
                                 // qDebug() << "信号源触发"  << measure_point.angles_ << " " <<  measure_point.ranges_;
                             }
                         }
                     });
}

bool Widget::DeleteMemoryFile(void)
{
    QString filename = appPath+"/"+SOFTSC;
    // std::string openfile = filename.toLocal8Bit().toStdString();
    QFile m_file(filename);

    test_lock = (bool)getIni<int>(LOCK, (int)test_lock);
    my_ctrl = test_lock;
    set_datanum = getIni<int>(DATANUM, set_datanum);

    if (m_file.remove())
    {
        this->resize(800,610);
        ui->DeviceBox->setCurrentIndex(0);
        ui->bandBox->setCurrentIndex(0);
        setIni<int>(LOCK, (int)test_lock);
        setIni<int>(DATANUM, set_datanum);
        QProcess::startDetached(QApplication::applicationFilePath(), QApplication::arguments());
        qApp->quit();
        return true;
    }

    return false;
}
QPropertyAnimation* Widget::NewAnimation()
{
    QPropertyAnimation *Ant = new QPropertyAnimation(ui->SCwidget, "geometry");
    connect(Ant, &QPropertyAnimation::valueChanged, this, [this](const QVariant &value) {
        // 在这里处理动画的当前值
        Width_offset = value.toRect().width();
        deviation = {.x=Width_offset, .y=Side_head.y};
        // qDebug() << "Current value:" << value.toRect() << value.toRect().width();

        if (Width_offset <= Side_head.x)
        {
            ui->SCwidget->hide();
            // ctrl->show();
            ctrlAutoClose->hide();
        }
        else if (Width_offset >= 200+Side_head.x)
        {
            ui->SCwidget->show();
            // ctrl->show();
            ctrlAutoClose->show();
        }
        else
        {
            if (ui->SCwidget->isHidden())
                ui->SCwidget->show();

            // if (!ctrl->isHidden())
            //     ctrl->hide();

            if (!ctrlAutoClose->isHidden())
                ctrlAutoClose->hide();
        }
        ctrlPoint.x = value.toRect().width()-ctrlSize.x;
        if (ctrlPoint.x < Side_head.x)
            ctrlPoint.x = Side_head.x;
        ctrl->move(ctrlPoint.x, ctrlPoint.y);
        ctrlAutoClose->move(ctrlPoint.x-ctrlAutoCloseSize.x-1, Side_head.y);

        m_resizeEvent();
    });

    return Ant;
}
void Widget::MyAnimationShow()
{
    if (animation == nullptr)
        animation = NewAnimation();

    // ctrl->setText("控制台◀");
    ctrl->setText("⨉");
    ctrlSize = {.x=15,.y=15};
    ctrlPoint = {.x=(200+Side_head.x-ctrlSize.x),.y=Side_head.y};
    ctrl->move(ctrlPoint.x, ctrlPoint.y);
    ctrl->resize(ctrlSize.x, ctrlSize.y);
    // ctrl->hide();
    ctrl->setStyleSheet((CSwindows_flag ? ctrl_o_color : ctrl_c_color));
    animation->setStartValue(QRect(Side_head.x, ctrlPoint.y, Side_head.x, ctrlSize.y)); // 设置起始位置和大小
    animation->setEndValue(QRect(Side_head.x, Side_head.y, 200 + Side_head.x, getHeight())); // 设置结束位置和大小
    animation->setDuration(300);
    animation->start(); // 启动动画
}
void Widget::MyAnimationRetract()
{
    if (animation == nullptr)
        animation = NewAnimation();

    ctrl->setText("控制台▶");
    ctrlSize = {.x=15,.y=60};
    ctrlPoint = {.x=Side_head.x,.y=20+Side_head.y};
    ctrl->move(ctrlPoint.x, ctrlPoint.y);
    ctrl->resize(ctrlSize.x, ctrlSize.y);
    // ctrl->hide();
    ctrl->setStyleSheet((CSwindows_flag ? ctrl_o_color : ctrl_c_color));
    animation->setStartValue(QRect(Side_head.x, Side_head.y, 200 + Side_head.x, getHeight())); // 设置起始位置和大小
    animation->setEndValue(QRect(Side_head.x, ctrlPoint.y, Side_head.x, ctrlSize.y)); // 设置结束位置和大小
    animation->setDuration(300);
    animation->start();
}
void Widget::SetInitFromMemory(void)
{
    /* 虚拟隐藏按键 */
    QFont fontV;
    fontV.setBold(true);
    fontV.setFamily("行楷");
    fontV.setPointSize(8);
    /* crtl */
    ctrl->setWordWrap(true);
    ctrl->setAlignment(Qt::AlignCenter);
    ctrl->resize(ctrlSize.x, ctrlSize.y);
    ctrl->move(ctrlPoint.x, ctrlPoint.y);
    ctrl->setFont(fontV);
    // ctrl->hide();
    /* ctrlAutoClose */
    ctrlAutoClose->setWordWrap(true);
    ctrlAutoClose->setAlignment(Qt::AlignCenter);
    ctrlAutoClose->resize(ctrlAutoCloseSize.x, ctrlAutoCloseSize.y);
    ctrlAutoClose->setFont(fontV);
    // ctrlAutoClose->show();
    /* statusPonit */
    statusPonit->setWordWrap(true);
    statusPonit->setAlignment(Qt::AlignCenter);
    statusPonit->resize(10,10);
    statusPonit->setText("〓");
    QFont fontSP;
    fontSP.setBold(true);
    fontSP.setPointSize(10);
    statusPonit->setFont(fontSP);

    //各按钮状态
    ui->label_openfile->setWordWrap(true);
    ui->Button_Start->setEnabled(false);
    ui->Button_Standby->setEnabled(false);
    ui->Button_Open->setEnabled(true);
    //ui->Button_Close->setEnabled(false);
    // ui->Button_send->setEnabled(false);

    //发送编辑框
    // ui->lineEdit_send->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9a-fA-F ]+$")));
    ui->lineEdit_send->setValidator(new QRegularExpressionValidator(QRegularExpression("(TEST)?(([0-9a-fA-F]{2})+[ ]?)+")));
    ui->lineEdit_send->setPlaceholderText("16进制内容/上下键翻历史");

    //label_device
    QFont font = ui->label_device->font();
    font.setBold(true);
    ui->label_device->setFont(font);
    //label_com
    font = ui->label_com->font();
    font.setBold(true);
    ui->label_com->setFont(font);
    //label_band
    font = ui->label_band->font();
    font.setBold(true);
    ui->label_band->setFont(font);
    //Console
    font = ui->Console->font();
    font.setBold(true);
    ui->Console->setFont(font);
    //SensorInf
    font = ui->SensorInf->font();
    font.setBold(true);
    ui->SensorInf->setFont(font);
    //label_type
    font = ui->label_type->font();
    font.setBold(true);
    ui->label_type->setFont(font);
    //label_sn
    font = ui->label_sn->font();
    font.setBold(true);
    ui->label_sn->setFont(font);
    //labelTypeText
    ui->labelTypeText->setText("none");
    font = ui->labelTypeText->font();
    font.setPointSize(8);
    ui->labelTypeText->setFont(font);
    //labelSnText
    ui->labelSnText->setText("none");
    font = ui->labelSnText->font();
    font.setPointSize(8);
    ui->labelSnText->setFont(font);
    ui->labelSnText->setWordWrap(true);
    ui->labelSnText->setScaledContents(true);
    ui->labelSnText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // ui->RadarScan->setStyleSheet("background-image: url(:/image/LB.png); background-repeat: no-repeat; background-position: center;");

    /************************从ini文件查询用户自定义设置***************************/
    //主窗口size
    Widget_width = getIni<int>(WWIDTH, Widget_width);
    Widget_height = getIni<int>(WHEIGHT, Widget_height);
    this->resize(Widget_width,Widget_height);
    //点云点像素大小
    m_point_pixel = getIni<float>(PPIXEL, m_point_pixel);
    //显示测量线及相关信息
    Show_indicator_line = (bool)getIni<int>(INDICATORLINE, (int)Show_indicator_line);
    Show_indicator_distance = (bool)getIni<int>(LINEDIS, (int)Show_indicator_distance);
    Show_indicator_angle = (bool)getIni<int>(LINEANGLE, (int)Show_indicator_angle);
    Show_indicator_confidence = (bool)getIni<int>(LINECONF, (int)Show_indicator_confidence);
    //滤波
    Enable_filte = (bool)getIni<int>(FLITER, (int)Enable_filte);
    filter_select._smooth = (uint16_t)getIni<int>(F_SMOOTH, filter_select._smooth);
    filter_select._bilateral = (uint16_t)getIni<int>(F_BILATERAL, filter_select._bilateral);
    filter_select._tail = (uint16_t)getIni<int>(F_TAIL, filter_select._tail);
    filter_select._intensity = (uint16_t)getIni<int>(F_INTENSITY, filter_select._intensity);
    filter_select._near = (uint16_t)getIni<int>(F_NEAR, filter_select._near);
    filter_select._noise = (uint16_t)getIni<int>(F_NOISE, filter_select._noise);
    filter_select._tine = (uint16_t)getIni<int>(F_TINE, filter_select._tine);
    filter_select._wander = (uint16_t)getIni<int>(F_WANDER, filter_select._wander);
    filter_select._shadows = (uint16_t)getIni<int>(F_SHADOWS, filter_select._shadows);
    filter_select._median = (uint16_t)getIni<int>(F_MEDIAN, filter_select._median);
    //点云颜色
    int pointColor = getIni<int>(PCOLOR, m_point_color.red()*1000000+m_point_color.green()*1000+m_point_color.blue());
    m_point_color.setRed(pointColor/1000000);
    m_point_color.setGreen((pointColor%1000000)/1000);
    m_point_color.setBlue(pointColor%1000);
    //测量线颜色
    int lineColor = getIni<int>(LINECOLOR, m_line_color.red()*1000000+m_line_color.green()*1000+m_line_color.blue());
    m_line_color.setRed(lineColor/1000000);
    m_line_color.setGreen((lineColor%1000000)/1000);
    m_line_color.setBlue(lineColor%1000);
    //画布背景颜色
    int backgroundColor = getIni<int>(BCOLOR, m_background_color.red()*1000000+m_background_color.green()*1000+m_background_color.blue());
    m_background_color.setRed(backgroundColor/1000000);
    m_background_color.setGreen((backgroundColor%1000000)/1000);
    m_background_color.setBlue(backgroundColor%1000);
    //控制区背景颜色
    int CSbackgroundColor = getIni<int>(CSCOLOR, CS_background_color.red()*1000000+CS_background_color.green()*1000+CS_background_color.blue());
    CS_background_color.setRed(CSbackgroundColor/1000000);
    CS_background_color.setGreen((CSbackgroundColor%1000000)/1000);
    CS_background_color.setBlue(CSbackgroundColor%1000);
    ChangeCSBackgroundColor(CS_background_color);
    //console set
    Devicebox_Idx = getIni<int>(DEVICE, Devicebox_Idx);
    BandBox_Idx = getIni<int>(BAND, BandBox_Idx);
    ui->DeviceBox->setCurrentIndex(Devicebox_Idx);
    ui->bandBox->setCurrentIndex(BandBox_Idx);
    ctrlAuto_flag = (bool)getIni<int>(AUTOCLOCE, (int)ctrlAuto_flag);
    ctrlAutoClose->setText((ctrlAuto_flag ? "⊢" : "⊬"));
    testOpen = getIni<int>(TESTON, testOpen);
    testOff = getIni<int>(TESTOFF, testOff);
    bool t_test_lock = test_lock;
    test_lock = (bool)getIni<int>(LOCK, (int)test_lock);
    my_ctrl = test_lock;
    if (test_lock == t_test_lock)
        setIni<int>(LOCK, (int)test_lock);
    int t_set_datanum = set_datanum;
    set_datanum = getIni<int>(DATANUM, set_datanum);
    if (set_datanum == t_set_datanum)
        setIni<int>(DATANUM, set_datanum);
    //极坐标设置
    p_flag = getIni<int>(CWCCW, p_flag);
    rot_angle = getIni<int>(ROTATE, rot_angle);
    //发送输入框历史信息导入
    string S_data;
    S_data = getIni<string>(SDATA, S_data);
    std::replace(S_data.begin(), S_data.end(), ',', ' ');
    for (size_t i = 0, si = 0, ei = 0; i < S_data.size(); i++)
    {
        ei = i;
        if (S_data[i] == '-')
        {
            if (si != ei)
            {
                string decompose_s(&S_data[si], &S_data[ei]);
                lineEditSlist.list.push_back(decompose_s);
            }
            si = ei + 1;
        }
    }
    if (lineEditSlist.list.size() > 0)
    {
        // ui->lineEdit_send->setText(QString::fromStdString(lineEditSlist.list.back()));
        lineEditSlist.now_idx = (int)lineEditSlist.list.size();
    }
    ui->lineEdit_send->clear();

    MyAnimationShow();

    VDATAcreatHor();
}

void Widget::ChangeCSBackgroundColor(QColor _color)
{
    // QPalette palette = QApplication::palette();
    // _color = palette.color(QPalette::Window);
    QColor reverse_Bcolor;
    reverse_Bcolor.setRgb((_color.red()^0xff), (_color.green()^0xff), (_color.blue()^0xff));
    // reverse_Bcolor.setRgb(abs(_color.red()-128), abs(_color.green()-128), abs(_color.blue()-128));
    QString set_color_str = QString("background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);")
                                 .arg(_color.red()).arg(_color.green()).arg(_color.blue())
                                 .arg(reverse_Bcolor.red()).arg(reverse_Bcolor.green()).arg(reverse_Bcolor.blue());
    QString turn_set_color_str = QString("background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);")
                                     .arg(reverse_Bcolor.red()).arg(reverse_Bcolor.green()).arg(reverse_Bcolor.blue())
                                     .arg(_color.red()).arg(_color.green()).arg(_color.blue());
    QString send_button_color_str = QString("QPushButton {background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);} "
                                            "QPushButton:pressed { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                            // "QPushButton:hover { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                            )
                                     .arg(reverse_Bcolor.red()).arg(reverse_Bcolor.green()).arg(reverse_Bcolor.blue())
                                     .arg(abs(_color.red()-50)).arg(abs(_color.green()-50)).arg(abs(_color.blue()-50))
                                     .arg(abs(reverse_Bcolor.red()-50)).arg(abs(reverse_Bcolor.green()-50)).arg(abs(reverse_Bcolor.blue()-50));
    ui->SCwidget->setStyleSheet(set_color_str);
    ui->Console->setStyleSheet(set_color_str);
    ui->SensorInf->setStyleSheet(set_color_str);
    ui->lineEdit_send->setStyleSheet(turn_set_color_str);
    ui->Button_send->setStyleSheet(send_button_color_str);
    ui->send_show->setStyleSheet(set_color_str);
    ui->label_openfile->setStyleSheet(set_color_str);
    ui->label_device->setStyleSheet(set_color_str);
    ui->label_com->setStyleSheet(set_color_str);
    ui->label_band->setStyleSheet(set_color_str);
    ui->labelTypeText->setStyleSheet(set_color_str);
    ui->labelSnText->setStyleSheet(set_color_str);
    ui->label_type->setStyleSheet(set_color_str);
    ui->label_sn->setStyleSheet(set_color_str);
    ui->comBox->setStyleSheet(turn_set_color_str);
    ui->bandBox->setStyleSheet(turn_set_color_str);
    ui->DeviceBox->setStyleSheet(turn_set_color_str);
    ctrl_o_color = turn_set_color_str;
    ctrl_c_color = set_color_str;
    ctrl->setStyleSheet((CSwindows_flag ? ctrl_o_color : ctrl_c_color));
    ctrlAutoClose->setStyleSheet(turn_set_color_str);

    SYButtonColor(_color, SYButton_stu);
    if (OpenedCom.isEmpty())
        OCButtonColor(_color, false);
    else
        OCButtonColor(_color, true);
}

void Widget::OCButtonColor(QColor _color, bool stu)
{
    // QPalette palette = QApplication::palette();
    // _color = palette.color(QPalette::Window);
    QColor m_Button_color(Qt::red);
    QColor reverse_Bcolor;
    reverse_Bcolor.setRgb((_color.red()^0xff), (_color.green()^0xff), (_color.blue()^0xff));
    // reverse_Bcolor.setRgb(abs(_color.red()-128), abs(_color.green()-128), abs(_color.blue()-128));
    QString turn_set_color_str = QString("QPushButton {background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);}"
                                         "QPushButton:pressed { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                         // "QPushButton:hover { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                         )
                                     .arg(reverse_Bcolor.red()).arg(reverse_Bcolor.green()).arg(reverse_Bcolor.blue())
                                     .arg(abs(_color.red()-50)).arg(abs(_color.green()-50)).arg(abs(_color.blue()-50))
                                     .arg(abs(m_Button_color.red()-50)).arg(abs(m_Button_color.green()-50)).arg(abs(m_Button_color.blue()-50));
    QString OCButton_set_color_str = QString("QPushButton {background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);}"
                                             "QPushButton:pressed { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                             // "QPushButton:hover { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                             )
                                         .arg(m_Button_color.red()).arg(m_Button_color.green()).arg(m_Button_color.blue())
                                         .arg(abs(_color.red()-50)).arg(abs(_color.green()-50)).arg(abs(_color.blue()-50))
                                         .arg(abs(m_Button_color.red()-50)).arg(abs(m_Button_color.green()-50)).arg(abs(m_Button_color.blue()-50));

    if (stu)
    {
        ui->Button_Open->setStyleSheet(OCButton_set_color_str);
        ui->Button_Close->setStyleSheet(turn_set_color_str);
    }
    else
    {
        ui->Button_Open->setStyleSheet(turn_set_color_str);
        ui->Button_Close->setStyleSheet(OCButton_set_color_str);
        statusPonit->setStyleSheet(QString("color: rgb(%1, %2, %3)").arg(m_Button_color.red()).arg(m_Button_color.green()).arg(m_Button_color.blue()));
    }
}

void Widget::SYButtonColor(QColor _color, SYB stu)
{
    // QPalette palette = QApplication::palette();
    // _color = palette.color(QPalette::Window);
    QColor m_Button_color(Qt::green);
    QColor reverse_Bcolor;
    reverse_Bcolor.setRgb((_color.red()^0xff), (_color.green()^0xff), (_color.blue()^0xff));
    // reverse_Bcolor.setRgb(abs(_color.red()-128), abs(_color.green()-128), abs(_color.blue()-128));
    QString turn_set_color_str = QString("QPushButton {background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);}"
                                         "QPushButton:pressed { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                         // "QPushButton:hover { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                         )
                                     .arg(reverse_Bcolor.red()).arg(reverse_Bcolor.green()).arg(reverse_Bcolor.blue())
                                     .arg(abs(_color.red()-50)).arg(abs(_color.green()-50)).arg(abs(_color.blue()-50))
                                     .arg(abs(m_Button_color.red()-50)).arg(abs(m_Button_color.green()-50)).arg(abs(m_Button_color.blue()-50));
    QString OCButton_set_color_str = QString("QPushButton {background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);}"
                                             "QPushButton:pressed { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                             // "QPushButton:hover { background-color: rgb(%7, %8, %9); color: rgb(%4, %5, %6);}"
                                             )
                                         .arg(m_Button_color.red()).arg(m_Button_color.green()).arg(m_Button_color.blue())
                                         .arg(abs(_color.red()-50)).arg(abs(_color.green()-50)).arg(abs(_color.blue()-50))
                                         .arg(abs(m_Button_color.red()-50)).arg(abs(m_Button_color.green()-50)).arg(abs(m_Button_color.blue()-50));

    switch (stu)
    {
    case SYB::none:
        ui->Button_Start->setStyleSheet(turn_set_color_str);
        ui->Button_Standby->setStyleSheet(turn_set_color_str);
        statusPonit->setStyleSheet(QString("color: rgb(%1, %2, %3)").arg(reverse_Bcolor.red()).arg(reverse_Bcolor.green()).arg(reverse_Bcolor.blue()));
        break;
    case SYB::START:
        ui->Button_Start->setStyleSheet(OCButton_set_color_str);
        ui->Button_Standby->setStyleSheet(turn_set_color_str);
        statusPonit->setStyleSheet(QString("color: rgb(%1, %2, %3)").arg(m_Button_color.red()).arg(m_Button_color.green()).arg(m_Button_color.blue()));
        break;
    case SYB::STANDBY:
        ui->Button_Start->setStyleSheet(turn_set_color_str);
        ui->Button_Standby->setStyleSheet(OCButton_set_color_str);
        statusPonit->setStyleSheet(QString("color: rgb(%1, %2, %3)").arg(reverse_Bcolor.red()).arg(reverse_Bcolor.green()).arg(reverse_Bcolor.blue()));
        break;
    default:
        break;
    }
}

void Widget::setDeviceSn(const device_sn_t devSn)
{
    QString qStr = QString::fromStdString(devSn.buff);
    QString strValue = QString::number(devSn.id);
    switch (devSn.id)
    {
    case TYPE_RADAR_LD:
        strValue += " -LD";//ledong
        break;
    case TYPE_RADAR_AOBI:
        strValue += " -AOB";//aobi
        break;
    case TYPE_RADAR_LANHAI:
        strValue += " -LH";//lanhai
        break;
    case TYPE_RADAR_GW:
        strValue += " -GW";//guangwei
        break;
    case TYPE_RADAR_AOBIK:
        strValue += " -AOBK";//aobiK
        break;
    case TYPE_RADAR_BZ:
        strValue += " -BZ";//buzhi
        break;
    case TYPE_RADAR_HC:
        strValue += " -HC";//huanchuang
        break;
    case TYPE_RADAR_DBZ:
        strValue += " -DBZ";//buzhi
        break;
    default:
        strValue += " -Other";
        break;
    }
    strValue += "(radar)";
    ui->labelTypeText->setText(strValue);
    ui->labelSnText->setText(qStr);
}
void Widget::setDeviceSn(const TYPE_LASER_ type, QString sn)
{
    linegetTypeTimeOut = 0;
    QString strValue = QString::number(type);
    switch (type)
    {
    case TYPE_LASER_EAI:
        strValue += " -EAI";//wanzhishang
        break;
    case TYPE_LASER_RS:
        strValue += " -RS";//ruishi
        break;
    case TYPE_LASER_RS_NEW:
        strValue += " -RS2";//ruishi2
        break;
    case TYPE_LASER_RS_XVB02:
        strValue += " -RSXVB02";//ruishiXVB02
        break;
    case TYPE_LASER_YX:
        strValue += " -YX";//yuanxingshikong
        break;
    case TYPE_LASER_LD:
        strValue += " -LD";//ledong
        break;
    default:
        strValue += " -Other";
        break;
    }
    strValue += "(line)";
    ui->labelTypeText->setText(strValue);
    ui->labelSnText->setText(sn);
}
void Widget::checkPorts()
{
    QStringList serialNamePort;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        //qDebug() << "串口检测到接入：" << info.portName();
        // 处理串口接入事件
        serialNamePort << info.portName();
    }

    //add new port
    for (auto &m : serialNamePort)
    {
        bool n_port_exist = false;
        for (int i = 0; i < ui->comBox->count(); i++)
        {
            if (m.compare(ui->comBox->itemText(i)) == 0)
                n_port_exist = true;
        }
        if (!n_port_exist)
            ui->comBox->addItem(m);
    }
    //remove absent port
    QStringList absent_port;
    for (int i = 0; i < ui->comBox->count(); i++)
    {
        bool n_port_exist = false;
        QString mv_port = ui->comBox->itemText(i);
        for (auto &m : serialNamePort)
        {
            if (m.compare(mv_port) == 0)
                n_port_exist = true;
        }
        if (!n_port_exist)
            absent_port.append(mv_port);
    }
    for (auto &m : absent_port)
    {
        int index = ui->comBox->findText(m);
        if (ui->comBox->findText(m) != -1)
        {
            ui->comBox->removeItem(index);
        }
        if (!OpenedCom.isEmpty())
        {
            if (OpenedCom.compare(m) == 0)
            {
                QMessageBox::warning(this, "提示",  "串口异常或已移除 "+OpenedCom);
                on_Button_Close_clicked();
            }
        }
    }

    if (linegetTypeTimeOut > 0)
    {
        linegetTypeTimeOut--;
        if (linegetTypeTimeOut == 0)
        {
            if (linelaser != nullptr)
                linelaser->outside_set_type(serialPort);
        }
    }

    // QDateTime currentTime = QDateTime::currentDateTime();
    // QString timeStr = currentTime.toString("yyyy/MM/dd hh:mm:ss");
    // setWindowTitle(QString("%1  -  %2").arg(MY_WINDOWTITLE, timeStr));
}
void Widget::DelLaserObj()
{
    if (linelaser != nullptr)
    {
        delete linelaser;
        linelaser = nullptr;
    }
    if (lidar != nullptr)
    {
        delete lidar;
        lidar = nullptr;
    }
    if (hclidar != nullptr)
    {
        delete hclidar;
        hclidar = nullptr;
    }
    if (Dbzlidar != nullptr)
    {
        delete Dbzlidar;
        Dbzlidar = nullptr;
    }
}
void Widget::on_Button_Open_clicked()
{
    serialPort->setPortName(ui->comBox->currentText());
    serialPort->setBaudRate(ui->bandBox->currentText().toInt());
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setParity(QSerialPort::NoParity);
    //serialPort->setFlowControl(QSerialPort::NoFlowControl);
    if (serialPort->open(QIODevice::ReadWrite))
    {
        OpenedCom = ui->comBox->currentText();
        OpenenType = ui->DeviceBox->currentText();
        ui->comBox->setEnabled(false);
        ui->bandBox->setEnabled(false);
        ui->DeviceBox->setEnabled(false);
        ui->Button_Open->setEnabled(false);
        ui->Button_Close->setEnabled(true);
        ui->Button_Start->setEnabled(true);
        ui->Button_Standby->setEnabled(true);
        // ui->Button_send->setEnabled(true);
        SYButton_stu = SYB::none;
        SYButtonColor(CS_background_color, SYButton_stu);
        OCButtonColor(CS_background_color, true);
        //QMessageBox::information(this, "提示",  "串口打开成功 "+ui->comBox->currentText());

        history_file_opened = false;
        history_file_data.data.clear();
        history_file_data.open_file_Tips.clear();
        ui->label_openfile->clear();
        //Resetdeviation();
        DelLaserObj();
        if (OpenenType.compare("LB_L") == 0)
        {
            linegetTypeTimeOut = 10;
            linelaser = new LLSDK();
            linelaser->setDeviceSnCallback([this](const TYPE_LASER_ type, QString sn)
                                          { this->setDeviceSn(type, sn); });
            linelaser->setDataCallback([this](std::vector<W_DataScan> data){ this->pushPoint(data); });
            linelaser->SetFilter(Enable_filte);
            linelaser->initLaserScan(serialPort);
        }
        else if (OpenenType.compare("LB_LOther") == 0)
        {
            linegetTypeTimeOut = 0;
            linelaser = new LLSDK();
            //linelaser->outside_set_type(serialPort, TYPE_LASER_YX);
            linelaser->setDeviceSnCallback([this](const TYPE_LASER_ type, QString sn)
                                           { this->setDeviceSn(type, sn); });
            linelaser->setDataCallback([this](std::vector<W_DataScan> data){ this->pushPoint(data); });
            linelaser->SetFilter(Enable_filte);
        }
        else if (OpenenType.compare("LB_RHC") == 0)
        {
            linegetTypeTimeOut = 0;
            hclidar = new HCLidar();
            hclidar->setDeviceSnCallback([this](const std::string vSn)
                                        {
                                            device_sn_t devSn;
                                            devSn.id = TYPE_RADAR_HC;
                                            devSn.buff = vSn;
                                            this->setDeviceSn(devSn);
                                        });
            hclidar->setDataCallback([this](std::vector<W_DataScan> data){ this->pushPoint(data); });

            if (Enable_filte)
                hclidar->SetFilter(*(uint16_t*)&filter_select);
            else
                hclidar->SetFilter(0);
        }
        else if (OpenenType.compare("LB_DBZ") == 0)
        {
            linegetTypeTimeOut = 0;
            Dbzlidar = new nvistar::LidarProtocol();
            Dbzlidar->setDeviceSnCallback([this](const std::string vSn)
                                          {
                                              device_sn_t devSn;
                                              devSn.id = TYPE_RADAR_DBZ;
                                              devSn.buff = vSn;
                                              this->setDeviceSn(devSn);
                                          });
            Dbzlidar->setDataCallback([this](std::vector<W_DataScan> data){ this->pushPoint(data); });
            Dbzlidar->DeviceClose(serialPort);
            Dbzlidar->MySleep(100);
            Dbzlidar->DeviceOpen(serialPort);
            SYButton_stu = SYB::START;
            SYButtonColor(CS_background_color, SYButton_stu);

            if (Enable_filte)
                Dbzlidar->SetFilter(*(uint16_t*)&filter_select);
            else
                Dbzlidar->SetFilter(0);
        }
        else
        {
            linegetTypeTimeOut = 0;
            lidar = new LPkg();
            lidar->setDeviceSnCallback([this](const device_sn_t devSn){ this->setDeviceSn(devSn); });
            lidar->setDataCallback([this](std::vector<W_DataScan> data){ this->pushPoint(data); });
            lidar->DeviceClose(serialPort);
            // QThread::usleep(100);
            lidar->MySleep(100);
            lidar->DeviceOpen(serialPort);
            SYButton_stu = SYB::START;
            SYButtonColor(CS_background_color, SYButton_stu);
            // filter_select = lidar->GetFilter();
            if (Enable_filte)
                lidar->SetFilter(*(uint16_t*)&filter_select);
            else
                lidar->SetFilter(0);
        }
    }
    else
    {
        if (serialPort->isOpen())
        {
            QMessageBox::information(this, "提示",  "串口已打开 "+ui->comBox->currentText());
        }
        else
        {
            QMessageBox::warning(this, "提示",  "串口打开失败");
            OpenedCom.clear();
            OpenenType.clear();
        }
    }
}

void Widget::on_Button_Close_clicked()
{
    if (serialPort->isOpen())
    {
        serialPort->close();
        ui->comBox->setEnabled(true);
        ui->bandBox->setEnabled(true);
        ui->DeviceBox->setEnabled(true);
        ui->Button_Open->setEnabled(true);
        //ui->Button_Close->setEnabled(false);
        ui->Button_Start->setEnabled(false);
        ui->Button_Standby->setEnabled(false);
        // ui->Button_send->setEnabled(false);
        ui->labelTypeText->setText("none");
        ui->labelSnText->setText("none");
        SYButton_stu = SYB::none;
        SYButtonColor(CS_background_color, SYButton_stu);
        OCButtonColor(CS_background_color, false);
    }
    Resetdeviation();
    DelLaserObj();
    OpenedCom.clear();
    OpenenType.clear();
}

void Widget::on_Button_Start_clicked()
{
    if (serialPort->isOpen())
    {
        if (OpenenType.compare("LB_L") == 0 || OpenenType.compare("LB_LOther") == 0)
        {
            if (linelaser != nullptr)
                linelaser->StartLaserScan(serialPort);
        }
        else if (OpenenType.compare("LB_DBZ") == 0)
        {
            if (Dbzlidar != nullptr)
                Dbzlidar->DeviceOpen(serialPort);
        }
        else
        {
            if (lidar != nullptr)
                lidar->DeviceOpen(serialPort);
        }

        if (linegetTypeTimeOut <= 0)
        {
            SYButton_stu = SYB::START;
            SYButtonColor(CS_background_color, SYButton_stu);
        }
    }
}

void Widget::on_Button_Standby_clicked()
{
    if (serialPort->isOpen())
    {
        if (OpenenType.compare("LB_L") == 0 || OpenenType.compare("LB_LOther") == 0)
        {
            if (linelaser != nullptr)
                linelaser->StopLaserScan(serialPort);
        }
        else if (OpenenType.compare("LB_DBZ") == 0)
        {
            if (Dbzlidar != nullptr)
                Dbzlidar->DeviceClose(serialPort);
        }
        else
        {
            if (lidar != nullptr)
                lidar->DeviceClose(serialPort);
        }

        if (linegetTypeTimeOut <= 0)
        {
            SYButton_stu = SYB::STANDBY;
            SYButtonColor(CS_background_color, SYButton_stu);
        }
    }
}

void Widget::on_Button_send_clicked()
{
    if (serialPort->isOpen())
    {
        QString lineEdit_send_bare = ui->lineEdit_send->text().trimmed();
        QString r_text = lineEdit_send_bare;
        r_text.remove(' ');

        if (!r_text.isEmpty())
        {
            if (r_text.size() % 2 != 0)
            {
                QMessageBox::warning(this->ui->lineEdit_send, "提示",  "输入格式有误,请修正");
                return;
            }

            std::string push_temp = lineEdit_send_bare.toStdString();
            for (std::vector<std::string>::iterator list_it = lineEditSlist.list.begin(); list_it != lineEditSlist.list.end();)
            {
                if (push_temp.compare((*list_it)) == 0)
                    lineEditSlist.list.erase(list_it);
                else
                    list_it++;
            }

            if (lineEditSlist.list.empty() ||
                (!lineEditSlist.list.empty() && push_temp.compare(lineEditSlist.list.back()) != 0))
            {
                if (lineEditSlist.list.size() >= 10)
                    lineEditSlist.list.erase(lineEditSlist.list.begin());
                lineEditSlist.list.push_back(push_temp);

                std::string set_str;
                for (auto &m : lineEditSlist.list)
                    set_str += (m + "-");
                std::replace(set_str.begin(), set_str.end(), ' ', ',');
                setIni<string>(SDATA, set_str);
            }
            lineEditSlist.now_idx = (int)lineEditSlist.list.size() - 1;
            ui->lineEdit_send->setText(QString::fromStdString(lineEditSlist.list[lineEditSlist.now_idx]));
        }
        else
        {
            ui->lineEdit_send->clear();
            QMessageBox::warning(this->ui->lineEdit_send, "提示",  "请输入数据后再发送");
            return;
        }

        QByteArray now_text = r_text.toUtf8();
        QByteArray sendData;
        uint8_t temp_ = 0;
        for (int i = 0; i < now_text.size(); i++)
        {
            uint8_t m_d = 0;
            if (now_text[i] >= '0' && now_text[i] <= '9')
                m_d = now_text[i] - '0';
            else if (now_text[i] >= 'a' && now_text[i] <= 'f')
                m_d = now_text[i] - 'a' + 10;
            else if (now_text[i] >= 'A' && now_text[i] <= 'F')
                m_d = now_text[i] - 'A' + 10;

            int bit_set;
            if (now_text.size() % 2 != 0)
                bit_set = i + 1;
            else
                bit_set = i;

            if (bit_set%2 == 0)
                temp_ = m_d << 4;
            else
            {
                temp_ |= m_d;
                sendData.append(temp_);
            }
        }
        qint64 ret = serialPort->write(sendData);
        qDebug() << Qt::hex << sendData << ret;
    }
    else
    {
        QString now_text = ui->lineEdit_send->text();
        now_text.remove(' ');
        if (now_text.size() >= 4)
        {
            if (now_text.compare("TEST") == 0)
            {
                my_ctrl = true;
                ui->send_show->setText(QString("T(%1,%2)").arg(testOpen).arg(testOff));
            }
            else if (now_text.compare("TESTFF") == 0)
            {
                my_ctrl = false;
                ui->send_show->clear();
            }
            else
            {
                QRegularExpression regex("^(TEST)(FF[0-9]{1,}){2}FF$");
                QRegularExpressionMatch match = regex.match(now_text);

                if (match.hasMatch())
                {
                    // qDebug() << "Match found:" << match.captured(0);
                    int O_data = 0, C_data = 0;
                    int pos0 = 4, pos1 = -1, pos2 = -1;
                    pos1 = now_text.indexOf("FF", pos0+2);
                    if (pos1 != -1)
                    {
                        qDebug() << "Keyword found at pos1:" << pos1;
                        pos2 = now_text.indexOf("FF", pos1+2);
                        if (pos2 != -1)
                            qDebug() << "Keyword found at pos2:" << pos2;
                    }

                    if (pos1 != -1 && pos2 != -1)
                    {
                        QByteArray re_text = now_text.toUtf8();

                        for (int idx = pos0+2; (idx < pos1 && idx < re_text.size()); idx++)
                        {
                            char c_t = re_text[idx];
                            if (c_t >= '0' && c_t <= '9')
                            {
                                O_data *= 10;
                                O_data += (c_t - '0');
                            }
                            else
                                break;
                        }
                        for (int idx = pos1+2; (idx < pos2 && idx < re_text.size()); idx++)
                        {
                            char c_t = re_text[idx];
                            if (c_t >= '0' && c_t <= '9')
                            {
                                C_data *= 10;
                                C_data += (c_t - '0');
                            }
                            else
                                break;
                        }
                    }
                    if (O_data > 0 && C_data >= 0)
                    {
                        testOpen = O_data;
                        testOff = C_data;
                        setIni<int>(TESTON, testOpen);
                        setIni<int>(TESTOFF, testOff);
                        if (my_ctrl)
                            ui->send_show->setText(QString("T(%1,%2)").arg(testOpen).arg(testOff));
                    }
                }
            }
        }
    }
}
