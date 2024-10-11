#include "inc/widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle(MY_WINDOWTITLE);
    this->layout()->setContentsMargins(Side_head.x, Side_head.y, Side_end.x, Side_end.y);
    this->layout()->setSpacing(0); // 设置布局间没有空白间隔

    appPath = QCoreApplication::applicationDirPath();
    // qDebug() << appPath;
    SetInitFromMemory();

    serialPort = new QSerialPort(this);
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
            else
            {
                if (lidar != nullptr)
                    lidar->DataParsing(data);
            }

            if (window != nullptr)
                window->InsertData(data);
        }
    });

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));
    timer->start(10);

    // QProcess::startDetached("cmd", QStringList() << "/c" << appPath.replace("&", "^&") + "/laser_view使用说明.pdf");
}

Widget::~Widget()
{
    closeWindow();
    DelLaserObj();
    delete serialPort;
    delete timer;
    delete ui;
}

bool Widget::DeleteMemoryFile(void)
{
    QString filename = appPath+"/"+SOFTSC;
    // std::string openfile = filename.toLocal8Bit().toStdString();
    QFile m_file(filename);

    if (m_file.remove())
    {
        this->resize(800,560);
        ui->DeviceBox->setCurrentIndex(0);
        ui->bandBox->setCurrentIndex(0);
        QProcess::startDetached(QApplication::applicationFilePath(), QApplication::arguments());
        qApp->quit();
        return true;
    }

    return false;
}

void Widget::SetInitFromMemory(void)
{
    ui->label_openfile->setWordWrap(true);
    ui->Button_Start->setEnabled(false);
    ui->Button_Standby->setEnabled(false);
    ui->Button_Open->setEnabled(true);
    //ui->Button_Close->setEnabled(false);
    ui->Button_send->setEnabled(false);
    // ui->lineEdit_send->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9a-fA-F ]+$")));
    ui->lineEdit_send->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9a-fA-F]+([ ]?[0-9a-fA-F]+)*")));
    ui->lineEdit_send->setPlaceholderText("16进制内容/上下键翻历史");
    QFont font = ui->label_device->font();
    font.setBold(true);
    ui->label_device->setFont(font);
    font = ui->label_com->font();
    font.setBold(true);
    ui->label_com->setFont(font);
    font = ui->label_band->font();
    font.setBold(true);
    ui->label_band->setFont(font);
    font = ui->Console->font();
    font.setBold(true);
    ui->Console->setFont(font);
    font = ui->SensorInf->font();
    font.setBold(true);
    ui->SensorInf->setFont(font);
    ui->labelTypeText->setText("none");
    ui->labelSnText->setText("none");
    // ui->RadarScan->setStyleSheet("background-image: url(:/image/LB.png); background-repeat: no-repeat; background-position: center;");

    /***************************************************/
    Widget_width = getIniint(WWIDTH, Widget_width);
    Widget_height = getIniint(WHEIGHT, Widget_height);
    this->resize(Widget_width,Widget_height);
    m_point_pixel = getInifloat(PPIXEL, m_point_pixel);
    Show_indicator_line = (bool)getIniint(INDICATORLINE, (int)Show_indicator_line);
    Show_indicator_distance = (bool)getIniint(LINEDIS, (int)Show_indicator_distance);
    Show_indicator_angle = (bool)getIniint(LINEANGLE, (int)Show_indicator_angle);
    Show_indicator_confidence = (bool)getIniint(LINECONF, (int)Show_indicator_confidence);
    Enable_filte = (bool)getIniint(FLITER, (int)Enable_filte);
    filter_select = (uint16_t)getIniint(FLITER_all, (int)filter_select);
    int pointColor = getIniint(PCOLOR, m_point_color.red()*1000000+m_point_color.green()*1000+m_point_color.blue());
    m_point_color.setRed(pointColor/1000000);
    m_point_color.setGreen((pointColor%1000000)/1000);
    m_point_color.setBlue(pointColor%1000);
    int lineColor = getIniint(LINECOLOR, m_line_color.red()*1000000+m_line_color.green()*1000+m_line_color.blue());
    m_line_color.setRed(lineColor/1000000);
    m_line_color.setGreen((lineColor%1000000)/1000);
    m_line_color.setBlue(lineColor%1000);
    int backgroundColor = getIniint(BCOLOR, m_background_color.red()*1000000+m_background_color.green()*1000+m_background_color.blue());
    m_background_color.setRed(backgroundColor/1000000);
    m_background_color.setGreen((backgroundColor%1000000)/1000);
    m_background_color.setBlue(backgroundColor%1000);
    int CSbackgroundColor = getIniint(CSCOLOR, CS_background_color.red()*1000000+CS_background_color.green()*1000+CS_background_color.blue());
    CS_background_color.setRed(CSbackgroundColor/1000000);
    CS_background_color.setGreen((CSbackgroundColor%1000000)/1000);
    CS_background_color.setBlue(CSbackgroundColor%1000);
    ChangeCSBackgroundColor(CS_background_color);
    Devicebox_Idx = getIniint(DEVICE, Devicebox_Idx);
    BandBox_Idx = getIniint(BAND, BandBox_Idx);
    ui->DeviceBox->setCurrentIndex(Devicebox_Idx);
    ui->bandBox->setCurrentIndex(BandBox_Idx);
    string S_data;
    S_data = getString(SDATA, S_data);
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
    }
}

void Widget::SYButtonColor(QColor _color, uint8_t stu)
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
    case 0:
        ui->Button_Start->setStyleSheet(turn_set_color_str);
        ui->Button_Standby->setStyleSheet(turn_set_color_str);
        break;
    case 1:
        ui->Button_Start->setStyleSheet(OCButton_set_color_str);
        ui->Button_Standby->setStyleSheet(turn_set_color_str);
        break;
    case 2:
        ui->Button_Start->setStyleSheet(turn_set_color_str);
        ui->Button_Standby->setStyleSheet(OCButton_set_color_str);
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
        strValue += " -LD";//乐动
        break;
    case TYPE_RADAR_AOBI:
        strValue += " -AOB";//奥比
        break;
    case TYPE_RADAR_LANHAI:
        strValue += " -LH";//蓝海
        break;
    case TYPE_RADAR_GW:
        strValue += " -GW";//光为
        break;
    case TYPE_RADAR_AOBIK:
        strValue += " -AOBK";//奥比K
        break;
    case TYPE_RADAR_BZ:
        strValue += " -BZ";//不止
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
        strValue += " -EAI";
        break;
    case TYPE_LASER_RS:
        strValue += " -RS";//瑞识
        break;
    case TYPE_LASER_RS_NEW:
        strValue += " -RS2";//瑞识2
        break;
    case TYPE_LASER_YX:
        strValue += " -YX";//远形时空
        break;
    case TYPE_LASER_LD:
        strValue += " -LD";//乐动
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
        ui->Button_send->setEnabled(true);
        OCButtonColor(CS_background_color, true);
        SYButton_stu = 0;
        SYButtonColor(CS_background_color, SYButton_stu);
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
            SYButton_stu = 1;
            SYButtonColor(CS_background_color, SYButton_stu);
            // filter_select = lidar->GetFilter();
            if (Enable_filte)
                lidar->SetFilter(this->filter_select);
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
        ui->Button_send->setEnabled(false);
        ui->labelTypeText->setText("none");
        ui->labelSnText->setText("none");
        OCButtonColor(CS_background_color, false);
        SYButton_stu = 0;
        SYButtonColor(CS_background_color, SYButton_stu);
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
        else
        {
            if (lidar != nullptr)
                lidar->DeviceOpen(serialPort);
        }

        if (linegetTypeTimeOut <= 0)
        {
            SYButton_stu = 1;
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
        else
        {
            if (lidar != nullptr)
                lidar->DeviceClose(serialPort);
        }

        if (linegetTypeTimeOut <= 0)
        {
            SYButton_stu = 2;
            SYButtonColor(CS_background_color, SYButton_stu);
        }
    }
}

void Widget::on_Button_send_clicked()
{
    if (serialPort->isOpen())
    {
        QString lineEdit_send_bare = ui->lineEdit_send->text();
        lineEdit_send_bare.remove(' ');

        if (!lineEdit_send_bare.isEmpty())
        {
            std::string push_temp = lineEdit_send_bare.toStdString();
            if (push_temp.size() > 0)
            {
                if (push_temp.size() % 2 != 0)
                    push_temp = '0' + push_temp;

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
                    setString(SDATA, set_str);
                }
                lineEditSlist.now_idx = (int)lineEditSlist.list.size() - 1;
                ui->lineEdit_send->setText(QString::fromStdString(lineEditSlist.list[lineEditSlist.now_idx]));
            }
            else
                ui->lineEdit_send->clear();
        }
        else
            ui->lineEdit_send->clear();

        QByteArray now_text = ui->lineEdit_send->text().toUtf8();
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
}

