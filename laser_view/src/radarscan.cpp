#include "inc/widget.h"
#include "inc/radarscan.h"

float Widget::getWidth()
{
    int m_Width_offset = Side_end.x;
    if (Width_offset < m_Width_offset)
        m_Width_offset = Width_offset;
    return (this->width() - (Width_offset + m_Width_offset));
}
float Widget::getHeight()
{
    return (this->height() - (Side_head.y + Side_end.y));
}
float Widget::getDiameter()
{
    return i_diameter*Display_factor;
}
float Widget::getRadius()
{
    return getDiameter()/2.0;
}

QPoint Widget::getPoint(bool original)
{
    QPoint new_Point(point);

    if (!original)
    {
        new_Point.setX(point.x() + point_deviation.x);
        new_Point.setY(point.y() + point_deviation.y);
    }

    return new_Point;
}

void Widget::m_resizeEvent(void)
{
    point = QPoint( getWidth() / 2.0, getHeight() / 2.0);
    if (this->getWidth() > this->getHeight())
        i_diameter = getHeight() - 20;
    else
        i_diameter = getWidth() - 20;
}

void Widget::resizeEvent(QResizeEvent *event)
{
    m_resizeEvent();
}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter p_painter(this);
    //反锯齿
    p_painter.setRenderHint(QPainter::Antialiasing);
    p_painter.drawPixmap(Width_offset,Side_head.y,getWidth(),getHeight(),paintWidget());

    p_painter.drawPixmap(Width_offset,Side_head.y+getHeight(),getWidth(),Side_end.y,paintWidget_time());

    QWidget::paintEvent(event);
}

void Widget::wheelEvent(QWheelEvent* event)
{
    int init_x = event->scenePosition().x();
    int init_y = event->scenePosition().y();
    int o_x = event->position().x();
    int o_y = event->position().y();
    if ((o_x > Width_offset && o_x < Width_offset + getWidth() && o_x >= init_x) &&
        (o_y > Side_end.y && o_y < Side_head.y + getHeight() && o_y >= init_y))
    {
        deviation.x = o_x - init_x;
        deviation.y = o_y - init_y;
        int numDegrees = event->angleDelta().y() / 8.0;
        int numSteps = numDegrees / 15.0;

        if (numSteps > 0)
        {
            int m_DF = ((int)Display_factor) / 10;
            if (Display_factor > 10)
                Display_factor += m_DF;
            else
                Display_factor += 0.1;

        }
        else if (numSteps < 0)
        {
            int m_DF = ((int)Display_factor) / 10;
            if (Display_factor > 10)
                Display_factor -= m_DF;
            else
                Display_factor -= 0.1;
        }
        if (Display_factor > 300)
            Display_factor = 300;
        else if (Display_factor < 0.1)
            Display_factor = 0.1;

        point_deviation.x += ((init_x - getPoint().x()) - (init_x - getPoint().x())*(Display_factor/Display_factor_last));
        point_deviation.y += ((init_y - getPoint().y()) - (init_y - getPoint().y())*(Display_factor/Display_factor_last));
        Display_factor_last = Display_factor;

        update();
        // qDebug() << "wheelEvent*" << numDegrees << numSteps << event->angleDelta();
        // qDebug() << "wheelEvent" << init_x << init_y;
        // qDebug() << "wheelEvent" << o_x << o_y;
        // qDebug() << "wheelEvent" << deviation.x << deviation.y;
    }
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    float init_x, init_y;
    int o_x = event->position().x();
    int o_y = event->position().y();
    if (event->button() == Qt::LeftButton)
    {
        if ((o_x >= ctrlPoint.x && o_x <= ctrlSize.x+ctrlPoint.x) && (o_y >= ctrlPoint.y && o_y <= ctrlSize.y+ctrlPoint.y))
        {
            ctrlAuto_time = -1;
            CSwindows_flag = !CSwindows_flag;
            if (CSwindows_flag)
                MyAnimationShow();
            else
                MyAnimationRetract();
        }
        else if ((o_x >= ctrlPoint.x-ctrlAutoCloseSize.x-1 && o_x <= ctrlPoint.x-1) && (o_y >= ctrlPoint.y && o_y <= ctrlAutoCloseSize.y+ctrlPoint.y))
        {
            ctrlAuto_flag = !ctrlAuto_flag;
            if (ctrlAuto_flag)
                ctrlAutoClose->setText("⊢");
            else
                ctrlAutoClose->setText("⊬");
            setIni<int>(AUTOCLOCE, (int)ctrlAuto_flag);

        }
        else if (o_x > Width_offset && o_x < Width_offset + getWidth()  && o_y > Side_head.y && o_y < Side_head.y + getHeight())
        {
            init_x = o_x - deviation.x;
            init_y = o_y - deviation.y;
            drag_L = {.start={init_x, init_y}, .end={-1, -1}, .flag=true};
            moving.x = init_x;
            moving.y = init_y;
            move_miss_flag = false;
            // qDebug() << "mousePressEvent L" << o_x << o_y;
            // qDebug() << "mousePressEvent L" << init_x << init_y;
        }
        else
        {
            ui->lineEdit_send->setFocus();
        }
    }

    if (event->button() == Qt::RightButton)
    {
        if (o_x > Width_offset && o_x < Width_offset + getWidth()  && o_y > Side_head.y  && o_y < Side_head.y + getHeight())
        {
            init_x = o_x - deviation.x;
            init_y = o_y - deviation.y;
            drag_R = {.start={init_x, init_y}, .end={-1, -1}, .flag=true};
            // moving.x = init_x;
            // moving.y = init_y;
            // qDebug() << "mousePressEvent R" << o_x << o_y;
            // qDebug() << "mousePressEvent R" << init_x << init_y;
        }
    }

    if (o_x > Side_head.x && o_x < Width_offset && o_y > Side_head.y  && o_y < Side_head.y + getHeight())
    {
        ctrlAuto_time = -1;
    }
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    // 当鼠标释放时
    int init_x, init_y;
    int o_x = event->position().x();
    int o_y = event->position().y();
    if (event->button() == Qt::LeftButton)
    {
        // 处理鼠标左键释放
        if (o_x > Width_offset && o_x < Width_offset + getWidth()  && o_y > Side_head.y  && o_y < Side_head.y + getHeight())
        {
            init_x = o_x - deviation.x;
            init_y = o_y - deviation.y;
            drag_L.end.x = init_x;
            drag_L.end.y = init_y;

            float _factor = getRadius() /
                            ((OpenenType.compare("LB_L") == 0 || OpenenType.compare("LB_LOther") == 0) ? 500 : 15000);
            float _dis = hypot((init_y - getPoint().y()), (init_x - getPoint().x())) / _factor;
            float _rad = atan2((init_y - getPoint().y()), (init_x - getPoint().x()));
            float _angle =  p_flag ? 360 - (RAD_TO_A(_rad) - rot_angle) : (RAD_TO_A(_rad) - rot_angle);
            while (_angle < 0)
                _angle += 360;
            while (_angle > 360)
                _angle -= 360;

            if (drag_L.flag &&
                drag_L.start.x != -1 && drag_L.start.y != -1 &&
                drag_L.end.x != -1 && drag_L.end.y != -1)
            {
                if (move_miss_flag)
                {
                    move_miss_flag = false;
                }
                else
                {
                    measure_point.angles_ = _angle;
                    measure_point.ranges_ = _dis;
                    updata_select = true;
                }
            }
            else
            {
                measure_point.angles_ = _angle;
                measure_point.ranges_ = _dis;
                updata_select = true;
            }
            update();
        }
        drag_L = {.start={-1, -1}, .end={-1, -1}, .flag=false};
    }
    if (event->button() == Qt::RightButton)
    {
        if (o_x > Width_offset && o_x < Width_offset + getWidth()  && o_y > Side_head.y  && o_y < Side_head.y + getHeight())
        {
            init_x = o_x - deviation.x;
            init_y = o_y - deviation.y;
            drag_R.end.x = init_x;
            drag_R.end.y = init_y;
            if (drag_R.flag &&
                drag_R.start.x != -1 && drag_R.start.y != -1 &&
                drag_R.end.x != -1 && drag_R.end.y != -1)
            {
                float now_diff_x = drag_R.end.x - drag_R.start.x;
                float now_diff_y = drag_R.end.y - drag_R.start.y;

                if (hypot(fabs(now_diff_y), fabs(now_diff_x)) > 2)
                {
                    contextMenu_miss_flag = true;
                    point_deviation.x += now_diff_x;
                    point_deviation.y += now_diff_y;
                }
                else
                {
                    contextMenu_miss_flag = false;
                }
            }
            else
            {
                contextMenu_miss_flag = false;
            }
            update();
        }
        else
        {
            contextMenu_miss_flag = false;
        }

        drag_R = {.start={-1, -1}, .end={-1, -1}, .flag=false};
        if (!contextMenu_miss_flag)
            drag_L = {.start={-1, -1}, .end={-1, -1}, .flag=false};
    }
    // QWidget::mouseReleaseEvent(event);
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    int init_x, init_y;
    int o_x = event->position().x();
    int o_y = event->position().y();
    if (drag_L.flag)
    {
        if (o_x > Width_offset && o_x < Width_offset + getWidth()  && o_y > Side_head.y  && o_y < Side_head.y + getHeight())
        {
            init_x = o_x - deviation.x;
            init_y = o_y - deviation.y;
            float now_diff_x = init_x - moving.x;
            float now_diff_y = init_y - moving.y;
            if (hypot(fabs(now_diff_y), fabs(now_diff_x)) > 2)
            {
                point_deviation.x += now_diff_x;
                point_deviation.y += now_diff_y;

                moving.x = init_x;
                moving.y = init_y;
                move_miss_flag = true;
                update();
            }
        }
    }
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up)
    {
        if (history_file_opened)
        {
            if (!history_file_data.data.empty())
            {
                history_file_data.now_idx--;
                if (history_file_data.now_idx < 0)
                    history_file_data.now_idx = 0;

                m_scandata = history_file_data.data[history_file_data.now_idx];
                ui->label_openfile->setText(history_file_data.open_file_Tips +
                    " {" + QString::number(history_file_data.now_idx+1) + "/" + QString::number(history_file_data.data.size()) + "}");
                update();
            }
        }
        else if (CSwindows_flag)
        {
            if (!lineEditSlist.list.empty() && QApplication::focusWidget() == ui->lineEdit_send)
            {
                lineEditSlist.now_idx--;
                if (lineEditSlist.now_idx < 0)
                    lineEditSlist.now_idx = 0;

                ui->lineEdit_send->setText(QString::fromStdString(lineEditSlist.list[lineEditSlist.now_idx]));
                update();
            }
        }
    }
    else if (event->key() == Qt::Key_Down)
    {
        if (history_file_opened)
        {
            if (!history_file_data.data.empty())
            {
                history_file_data.now_idx++;
                if (history_file_data.now_idx > (int)history_file_data.data.size() - 1)
                    history_file_data.now_idx = (int)history_file_data.data.size() - 1;

                m_scandata = history_file_data.data[history_file_data.now_idx];
                ui->label_openfile->setText(history_file_data.open_file_Tips +
                    " {" + QString::number(history_file_data.now_idx+1) + "/" + QString::number(history_file_data.data.size()) + "}");
                update();
            }
        }
        else if (CSwindows_flag)
        {
            if (!lineEditSlist.list.empty() && QApplication::focusWidget() == ui->lineEdit_send)
            {
                lineEditSlist.now_idx++;
                if (lineEditSlist.now_idx > (int)lineEditSlist.list.size())
                    lineEditSlist.now_idx = (int)lineEditSlist.list.size();

                if (lineEditSlist.now_idx == (int)lineEditSlist.list.size())
                    ui->lineEdit_send->clear();
                else
                    ui->lineEdit_send->setText(QString::fromStdString(lineEditSlist.list[lineEditSlist.now_idx]));
                update();
            }
        }
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void Widget::contextMenuEvent(QContextMenuEvent *event)
{
    int o_x = event->pos().x();
    int o_y = event->pos().y();
    if (o_y > Side_head.y  && o_y < Side_head.y + getHeight())
    {
        if (!contextMenu_miss_flag && o_x > Width_offset && o_x < Width_offset + getWidth())
        {
            QMenu menu(this);
            std::list<std::function<bool (QAction *)>> action_list;
            // menu.setStyleSheet("QMenu { background-color: white; color: black; }");

            /************************Background************************/
            QAction *action_Background_color = menu.addAction("Background color");
            action_Background_color->setCheckable(false);
            action_list.push_back([this, &action_Background_color](QAction * sAction)->bool {
                if (action_Background_color != nullptr && sAction == action_Background_color)
                {
                    QColor color = QColorDialog::getColor(m_background_color, this);
                    if (color.isValid())
                    {
                        m_background_color = color;
                        setIni<int>(BCOLOR, m_background_color.red()*1000000+m_background_color.green()*1000+m_background_color.blue());
                    }

                    return true;
                }

                return false;
            });

            /************************Indicator line items************************/
            QMenu *action_polar_coordinates = menu.addMenu("Polar coordinates items");
            QAction *polar_clockwise = p_flag ? action_polar_coordinates->addAction("Turn to CW ↻")
                                              : action_polar_coordinates->addAction("Turn to CCW ↺");
            action_list.push_back([this, &polar_clockwise](QAction * sAction)->bool {
                if (polar_clockwise !=nullptr && sAction == polar_clockwise)
                {
                    p_flag = !p_flag;
                    setIni<int>(CWCCW, (int)p_flag);

                    return true;
                }

                return false;
            });
            //polar_cw90
            QAction *polar_cw90 = action_polar_coordinates->addAction("Rotate 90°CW ↷");
            action_list.push_back([this, &polar_cw90](QAction * sAction)->bool {
                if (polar_cw90 !=nullptr && sAction == polar_cw90)
                {
                    rot_angle += 90;
                    while (rot_angle > 180)
                        rot_angle -= 360;
                    while (rot_angle < -180)
                        rot_angle += 360;
                    setIni<int>(ROTATE, rot_angle);

                    return true;
                }

                return false;
            });
            //polar_ccw90
            QAction *polar_ccw90 = action_polar_coordinates->addAction("Rotate 90°CCW ↶");
            action_list.push_back([this, &polar_ccw90](QAction * sAction)->bool {
                if (polar_ccw90 !=nullptr && sAction == polar_ccw90)
                {
                    rot_angle -= 90;
                    while (rot_angle > 180)
                        rot_angle -= 360;
                    while (rot_angle < -180)
                        rot_angle += 360;
                    setIni<int>(ROTATE, rot_angle);

                    return true;
                }

                return false;
            });

            /************************Show indicator line************************/
            QAction *action_indicator = menu.addAction("Show indicator line");
            action_indicator->setCheckable(true); // 设置为可勾选
            action_indicator->setChecked(this->Show_indicator_line);
            action_list.push_back([this, &action_indicator](QAction * sAction)->bool {
                if (action_indicator !=nullptr && sAction == action_indicator)
                {
                    this->Show_indicator_line = !this->Show_indicator_line;
                    action_indicator->setChecked(Show_indicator_line);

                    setIni<int>(INDICATORLINE, (int)this->Show_indicator_line);

                    return true;
                }

                return false;
            });

            /************************Indicator line items************************/
            QMenu *action_indicator_other = menu.addMenu("Indicator line items");
            //
            QAction *indicator_other_distance = action_indicator_other->addAction("Show distance");
            indicator_other_distance->setCheckable(true);
            indicator_other_distance->setChecked(Show_indicator_distance);
            action_list.push_back([this, &indicator_other_distance](QAction * sAction)->bool {
                if (indicator_other_distance !=nullptr && sAction == indicator_other_distance)
                {
                    this->Show_indicator_distance = !this->Show_indicator_distance;
                    indicator_other_distance->setChecked(Show_indicator_distance);

                    setIni<int>(LINEDIS, (int)this->Show_indicator_distance);

                    return true;
                }

                return false;
            });
            //
            QAction *indicator_other_angle = action_indicator_other->addAction("Show angle");
            indicator_other_angle->setCheckable(true);
            indicator_other_angle->setChecked(Show_indicator_angle);
            action_list.push_back([this, &indicator_other_angle](QAction * sAction)->bool {
                if (indicator_other_angle != nullptr && sAction == indicator_other_angle)
                {
                    this->Show_indicator_angle = !this->Show_indicator_angle;
                    indicator_other_angle->setChecked(Show_indicator_angle);

                    setIni<int>(LINEANGLE, (int)this->Show_indicator_angle);

                    return true;
                }

                return false;
            });
            //
            QAction *indicator_other_confidence = action_indicator_other->addAction("Show confidence");
            indicator_other_confidence->setCheckable(true);
            indicator_other_confidence->setChecked(Show_indicator_confidence);
            action_list.push_back([this, &indicator_other_confidence](QAction * sAction)->bool {
                if (indicator_other_confidence != nullptr && sAction == indicator_other_confidence)
                {

                    this->Show_indicator_confidence = !this->Show_indicator_confidence;
                    indicator_other_confidence->setChecked(Show_indicator_confidence);

                    setIni<int>(LINECONF, (int)this->Show_indicator_confidence);

                    return true;
                }

                return false;
            });
            //
            QAction *line_color = action_indicator_other->addAction("line color");
            line_color->setCheckable(false);
            action_list.push_back([this, &line_color](QAction * sAction)->bool {
                if (line_color != nullptr && sAction == line_color)
                {
                    QColor color = QColorDialog::getColor(m_line_color, this);
                    if (color.isValid())
                    {
                        m_line_color = color;
                        setIni<int>(LINECOLOR, m_line_color.red()*1000000+m_line_color.green()*1000+m_line_color.blue());
                    }

                    return true;
                }

                return false;
            });



            /************************Enable filter************************/
            QAction *action_filter = nullptr;
            if (!OpenenType.isEmpty())
            {
                action_filter = menu.addAction("Enable filter");
                action_filter->setCheckable(true);
                action_filter->setChecked(Enable_filte);
                action_list.push_back([this, &action_filter](QAction * sAction) {
                    if (action_filter != nullptr  && sAction == action_filter)
                    {
                        this->Enable_filte = !this->Enable_filte;
                        action_filter->setChecked(Enable_filte);

                        set_filter(Enable_filte, filter_select);

                        if (linelaser != nullptr)
                            linelaser->SetFilter(this->Enable_filte);

                        setIni<int>(FLITER, (int)this->Enable_filte);

                        return true;
                    }

                    return false;
                });
            }

            QAction *filter_items_smooth = nullptr;
            QAction *filter_items_bilateral = nullptr;
            QAction *filter_items_tail = nullptr;
            QAction *filter_items_intensity = nullptr;
            QAction *filter_items_Near = nullptr;
            QAction *filter_items_Noise = nullptr;
            QAction *filter_items_Tine = nullptr;
            QAction *filter_items_Wander = nullptr;
            QAction *filter_items_Shadows = nullptr;
            QAction *filter_items_Median = nullptr;
            if (OpenenType.compare("LB_R0") == 0 || OpenenType.compare("LB_R1") == 0 ||
                OpenenType.compare("LB_RHC") == 0 || OpenenType.compare("LB_DBZ") == 0)
            {
                /************************filter_items************************/
                QMenu *action_filter_items =  menu.addMenu("Filter setting items");
                //
                filter_items_smooth = action_filter_items->addAction("smooth filter");
                filter_items_smooth->setCheckable(true);
                filter_items_smooth->setChecked((bool)filter_select._smooth);
                action_list.push_back([this, &filter_items_smooth](QAction * sAction)->bool {
                    if (filter_items_smooth !=nullptr && sAction == filter_items_smooth)
                    {
                        this->filter_select._smooth = ~(this->filter_select._smooth);
                        filter_items_smooth->setChecked((bool)filter_select._smooth);

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_SMOOTH, filter_select._smooth);

                        return true;
                    }

                    return false;
                });
                //
                filter_items_bilateral = action_filter_items->addAction("bilateral filter");
                filter_items_bilateral->setCheckable(true);
                filter_items_bilateral->setChecked((bool)(filter_select._bilateral));
                action_list.push_back([this, &filter_items_bilateral](QAction * sAction)->bool {
                    if (filter_items_bilateral != nullptr && sAction == filter_items_bilateral)
                    {
                        this->filter_select._bilateral = ~(this->filter_select._bilateral);
                        filter_items_bilateral->setChecked((bool)(filter_select._bilateral));

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_BILATERAL, filter_select._bilateral);

                        return true;
                    }

                    return false;
                });
                //
                filter_items_tail = action_filter_items->addAction("tail filter");
                filter_items_tail->setCheckable(true);
                filter_items_tail->setChecked((bool)(filter_select._tail));
                action_list.push_back([this, &filter_items_tail](QAction * sAction)->bool {
                    if (filter_items_tail != nullptr && sAction == filter_items_tail)
                    {
                        this->filter_select._tail = ~(this->filter_select._tail);
                        filter_items_tail->setChecked((bool)(filter_select._tail));

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_TAIL, filter_select._tail);

                        return true;
                    }

                    return false;
                });
                //
                filter_items_intensity = action_filter_items->addAction("intensity filter");
                filter_items_intensity->setCheckable(true);
                filter_items_intensity->setChecked((bool)(filter_select._intensity));
                action_list.push_back([this, &filter_items_intensity](QAction * sAction)->bool {
                    if (filter_items_intensity != nullptr && sAction == filter_items_intensity)
                    {
                        this->filter_select._intensity = ~(this->filter_select._intensity);
                        filter_items_intensity->setChecked((bool)(filter_select._intensity));

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_INTENSITY, filter_select._intensity);

                        return true;
                    }

                    return false;
                });
                //
                filter_items_Near = action_filter_items->addAction("near filter");
                filter_items_Near->setCheckable(true);
                filter_items_Near->setChecked((bool)(filter_select._near));
                action_list.push_back([this, &filter_items_Near](QAction * sAction)->bool {
                    if (filter_items_Near != nullptr && sAction == filter_items_Near)
                    {
                        this->filter_select._near = ~(this->filter_select._near);
                        filter_items_Near->setChecked((bool)(filter_select._near));

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_NEAR, filter_select._near);

                        return true;
                    }

                    return false;
                });
                //
                filter_items_Noise = action_filter_items->addAction("noise filter");
                filter_items_Noise->setCheckable(true);
                filter_items_Noise->setChecked((bool)(filter_select._noise));
                action_list.push_back([this, &filter_items_Noise](QAction * sAction)->bool {
                    if (filter_items_Noise != nullptr && sAction == filter_items_Noise)
                    {
                        this->filter_select._noise = ~(this->filter_select._noise);
                        filter_items_Noise->setChecked((bool)(filter_select._noise));

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_NOISE, filter_select._noise);

                        return true;
                    }

                    return false;
                });
                //
                filter_items_Tine = action_filter_items->addAction("tine filter");
                filter_items_Tine->setCheckable(true);
                filter_items_Tine->setChecked((bool)(filter_select._tine));
                action_list.push_back([this, &filter_items_Tine](QAction * sAction)->bool {
                    if (filter_items_Tine != nullptr && sAction == filter_items_Tine)
                    {
                        this->filter_select._tine = ~(this->filter_select._tine);
                        filter_items_Tine->setChecked((bool)(filter_select._tine));

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_TINE, filter_select._tine);

                        return true;
                    }

                    return false;
                });
                //
                filter_items_Wander = action_filter_items->addAction("wander filter");
                filter_items_Wander->setCheckable(true);
                filter_items_Wander->setChecked((bool)(filter_select._wander));
                action_list.push_back([this, &filter_items_Wander](QAction * sAction)->bool {
                    if (filter_items_Wander != nullptr && sAction == filter_items_Wander)
                    {
                        this->filter_select._wander = ~(this->filter_select._wander);
                        filter_items_Wander->setChecked((bool)(filter_select._wander));

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_WANDER, filter_select._wander);

                        return true;
                    }

                    return false;
                });
                //
                filter_items_Shadows = action_filter_items->addAction("shadows filter");
                filter_items_Shadows->setCheckable(true);
                filter_items_Shadows->setChecked((bool)(filter_select._shadows));
                action_list.push_back([this, &filter_items_Shadows](QAction * sAction)->bool {
                    if (filter_items_Shadows != nullptr && sAction == filter_items_Shadows)
                    {
                        this->filter_select._shadows = ~(this->filter_select._shadows);
                        filter_items_Shadows->setChecked((bool)(filter_select._shadows));

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_SHADOWS, filter_select._shadows);

                        return true;
                    }

                    return false;
                });
                //
                filter_items_Median = action_filter_items->addAction("median filter");
                filter_items_Median->setCheckable(true);
                filter_items_Median->setChecked((bool)(filter_select._median));
                action_list.push_back([this, &filter_items_Median](QAction * sAction)->bool {
                    if (filter_items_Median != nullptr && sAction == filter_items_Median)
                    {
                        this->filter_select._median = ~(this->filter_select._median);
                        filter_items_Median->setChecked((bool)(filter_select._median));

                        set_filter(Enable_filte, filter_select);

                        setIni<int>(F_MEDIAN, filter_select._median);

                        return true;
                    }

                    return false;
                });
            }

            /************************point pixel************************/
            QMenu *action_point_pixel = menu.addMenu("Point info items");

            std::vector<QAction *> p_pixel_;
            for (int i = 0; i < 7; i++)
            {
                float now_f = 0.5 * (i+1);
                QString a_id = QString::number(now_f, 'f', 1) + "x";
                QAction *point_pixel_ = action_point_pixel->addAction(a_id);
                point_pixel_->setCheckable(true);
                point_pixel_->setChecked((m_point_pixel == now_f));
                p_pixel_.push_back(point_pixel_);
            }
            action_list.push_back([this, &p_pixel_](QAction * sAction)->bool {
                for (size_t i = 0; i < p_pixel_.size(); i++)
                {
                    if (p_pixel_[i] != nullptr && sAction == p_pixel_[i])
                    {
                        this->m_point_pixel = 0.5 * (i+1);
                        // for (int j = 0; j < p_pixel_.size(); j++)
                        //     p_pixel_[j]->setChecked((m_point_pixel ==  (0.5 * (j+1))));

                        setIni<float>(PPIXEL, m_point_pixel);

                        return true;
                    }
                }

                return false;
            });

            //
            QAction *point_color = action_point_pixel->addAction("point color");
            point_color->setCheckable(false);
            action_list.push_back([this, &point_color](QAction * sAction)->bool {
                if (point_color != nullptr && sAction == point_color)
                {
                    QColor color = QColorDialog::getColor(m_point_color, this);
                    if (color.isValid())
                    {
                        m_point_color = color;
                        setIni<int>(PCOLOR, m_point_color.red()*1000000+m_point_color.green()*1000+m_point_color.blue());
                    }

                    return true;
                }

                return false;
            });

            /************************Data display window************************/
            QAction *action_display_window = menu.addAction("Data display window");
            action_display_window->setCheckable(true);
            action_display_window->setChecked((window != nullptr));
            action_list.push_back([this, &action_display_window](QAction * sAction)->bool {
                if (action_display_window != nullptr && sAction == action_display_window)
                {
                    if (window != nullptr)
                    {
                        closeWindow();
                    }
                    else
                    {
                        window = new DataDisplayWindow(this, MY_WINDOWTITLE);
                        window->callbackobjclose([this](){ this->closeWindow(); });
                        if (MwindowPos.x != -1 && MwindowPos.y != -1)
                            window->move(MwindowPos.x ,MwindowPos.y);
                        if (MwindowSize.x != -1 && MwindowSize.y != -1)
                            window->resize(MwindowSize.x ,MwindowSize.y);
                        // window->setStyleSheet("background-image: url(:/image/LB.png); background-size: 100% 100%; background-repeat: no-repeat; background-position: center;");
                        window->show();
                    }
                    action_display_window->setChecked((window != nullptr));

                    return true;
                }

                return false;
            });

            /************************other message************************/
            QString m_message;
            if (OpenenType.compare("LB_L") == 0 || OpenenType.compare("LB_LOther") == 0)
            {
                if (linelaser != nullptr)
                    m_message = linelaser->GetOtherMessage();
            }
            else if (OpenenType.compare("LB_RHC") == 0)
            {
                if (hclidar != nullptr)
                    m_message = QString::fromStdString(hclidar->GetOtherMessage());
            }
            else if (OpenenType.compare("LB_DBZ") == 0)
            {
                if (Dbzlidar != nullptr)
                    m_message = Dbzlidar->GetOtherMessage();
            }
            else
            {
                if (lidar != nullptr)
                    m_message = lidar->GetOtherMessage();
            }
            QAction *action_other_message = nullptr;
            if (!m_message.isEmpty())
            {
                action_other_message = menu.addAction("Other message");
                action_other_message->setCheckable(false);
                action_list.push_back([this, &action_other_message, &m_message](QAction * sAction)->bool {
                    if (action_other_message != nullptr && sAction == action_other_message)
                    {
                        QMessageBox::information(this, "Other message",  m_message);

                        return true;
                    }

                    return false;
                });
            }
            /************************Check in history file************************/
            QAction *action_history_file = nullptr;
            if (OpenenType.isEmpty())
            {
                action_history_file = menu.addAction("Check in history file");
                action_history_file->setCheckable(true);
                action_history_file->setChecked(history_file_opened);
                action_list.push_back([this, &action_history_file](QAction * sAction)->bool {
                    if (action_history_file != nullptr && sAction == action_history_file)
                    {
                        ui->lineEdit_send->setFocus();
                        history_file_data.data.clear();
                        history_file_data.open_file_Tips.clear();
                        if (this->history_file_opened)
                        {
                            this->history_file_opened = false;
                            ui->label_openfile->clear();
                        }
                        else
                        {
                            QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open File"),
                                                                            "data", QObject::tr("All Files (*)"));
                            if (!fileName.isEmpty())
                            {
                                this->history_file_opened = true;
                                history_file_data.open_file_Tips = "已打开文件：" + fileName +
                                                                   "<br><br>注：详文件格式请往说明.<br>可上下方向键切换多帧数据";
                                ui->label_openfile->setText(history_file_data.open_file_Tips);
                                HistoryData(fileName);
                            }
                        }
                        action_history_file->setChecked(history_file_opened);

                        return true;
                    }

                    return false;
                });
            }
            /************************Save file************************/
            QAction *action_save_file = nullptr;
            if (save_temp_data.data.size() > 1)
            {
                action_save_file = menu.addAction("Save current data");
                action_save_file->setCheckable(false);
                action_list.push_back([this, &action_save_file](QAction * sAction)->bool {
                    if (action_save_file != nullptr && sAction == action_save_file)
                    {
                        QMutexLocker lock(&save_data_mutex);
                        SaveNewFile(save_temp_data);
                        lock.unlock();
                        // save_temp_data.data.clear();

                        return true;
                    }

                    return false;
                });
            }

            // 显示菜单
            QAction *selectedAction = menu.exec(event->globalPos());
            for ( auto& ac : action_list )
                ac(selectedAction);

        }
        else if (o_x > Side_head.x && o_x < Width_offset)
        {
            QMenu menu(this);
            std::list<std::function<bool (QAction *)>> action_list;
            // menu.setStyleSheet("QMenu { border-radius: 5px; background-color: white; color: black; }");
            /************************C&S Background************************/
            QAction *action_CS_Background_color = menu.addAction("C/S background color");
            action_CS_Background_color->setCheckable(false);
            action_list.push_back([this, &action_CS_Background_color](QAction * sAction)->bool {
                if (action_CS_Background_color != nullptr && sAction == action_CS_Background_color)
                {
                    QColor color = QColorDialog::getColor(CS_background_color, this);
                    if (color.isValid())
                    {
                        CS_background_color = color;
                        ChangeCSBackgroundColor(CS_background_color);
                        setIni<int>(CSCOLOR, CS_background_color.red()*1000000+CS_background_color.green()*1000+CS_background_color.blue());
                    }

                    return true;
                }

                return false;
            });
            //
            QAction *action_Instructions = menu.addAction("Instructions(How to use)");
            action_Instructions->setCheckable(false);
            action_list.push_back([this, &action_Instructions](QAction * sAction)->bool {
                if (action_Instructions != nullptr && sAction == action_Instructions)
                {
                    // QProcess::startDetached("cmd", QStringList() << "/c" << appPath + "/laser_view使用说明.pdf");
                    QProcess::startDetached("cmd", QStringList() << "/c" << appPath.replace("&", "^&") + "/laser_view使用说明.pdf");

                    return true;
                }

                return false;
            });
            /************************Recovery Default Style************************/
            QAction *action_Default_Style = menu.addAction("Recovery Default Style");
            action_Default_Style->setCheckable(false);
            action_list.push_back([this, &action_Default_Style](QAction * sAction)->bool {
                if (action_Default_Style != nullptr && sAction == action_Default_Style)
                {
                    DeleteMemoryFile();

                    return true;
                }

                return false;
            });

            QAction *selectedAction = menu.exec(event->globalPos());
            for ( auto& ac : action_list )
                ac(selectedAction);
        }
    }
    else if (o_y > Side_head.y + getHeight())
    {
        QMenu menu(this);
        std::list<std::function<bool (QAction *)>> action_list;
        // /************************Console and SensorInf windows************************/
        // QAction *action_CSwindows = menu.addAction("Console and SensorInf window");
        // action_CSwindows->setCheckable(true);
        // action_CSwindows->setChecked(CSwindows_flag);
        // action_list.push_back([this, &action_CSwindows](QAction * sAction)->bool {
        //     if (action_CSwindows != nullptr && sAction == action_CSwindows)
        //     {
        //         CSwindows_flag = !CSwindows_flag;
        //         action_CSwindows->setChecked(CSwindows_flag);
        //         if (CSwindows_flag)
        //         {
        //             // Width_offset = 200 + Side_head.x;
        //             // deviation = {.x=Width_offset, .y=Side_head.y};
        //             // ui->SCwidget->show();
        //             MyAnimationShow();
        //         }
        //         else
        //         {
        //             // Width_offset = 0;
        //             // deviation = {.x=Width_offset, .y=Side_head.y};
        //             // ui->SCwidget->hide();
        //             MyAnimationRetract();
        //         }
        //         m_resizeEvent();
        //         // ui->SCwidget->setVisible(false);

        //         return true;
        //     }

        //     return false;
        // });

        QAction *selectedAction = menu.exec(event->globalPos());
        for ( auto& ac : action_list )
            ac(selectedAction);
    }
}

void Widget::closeWindow()
{
    if (window != nullptr)
    {
        if (window->windowState() & Qt::WindowMaximized)
        {
            qDebug() << "window窗口处于最大化状态";
        }
        else
        {
            MwindowSize = {(float)window->width(), (float)window->height()};
            MwindowPos = {(float) window->pos().x(), (float) window->pos().y()};
        }
        delete window;
        window = nullptr;
    }
}

void Widget::timerTimeOut()
{
    if (test_count > 0)
        test_count--;
    else
    {
        if (my_ctrl) //"test"
        {
            if (testOff == 0)
            {
                if (test_datanum > set_datanum || SYButton_stu != SYB::START)
                {
                    if (ui->send_show->text().compare(QString("T(%1,%2)").arg(testOpen).arg(testOff)) != 0)
                        ui->send_show->setText(QString("T(%1,%2)").arg(testOpen).arg(testOff));
                }
                else
                {
                    if (/*!OpenedCom.isEmpty() && */ui->send_show->text().compare(QString("T warning")) != 0)
                        ui->send_show->setText(QString("T warning"));
                }
                test_count = testOpen*1000/timerTime;
            }
            else
            {
                if (SYButton_stu == SYB::STANDBY)
                {
                    on_Button_Start_clicked();
                    test_count = testOpen*1000/timerTime;
                }
                else
                {
                    if (test_datanum > set_datanum || OpenedCom.isEmpty())
                    {
                        on_Button_Standby_clicked();
                        if (ui->send_show->text().compare(QString("T(%1,%2)").arg(testOpen).arg(testOff)) != 0)
                            ui->send_show->setText(QString("T(%1,%2)").arg(testOpen).arg(testOff));
                    }
                    else
                    {
                        if (/*!OpenedCom.isEmpty() && */ui->send_show->text().compare(QString("T warning")) != 0)
                            ui->send_show->setText(QString("T warning"));
                    }
                    test_count = testOff*1000/timerTime;
                }
            }

        }
        else
        {
            test_count = 1000/timerTime;
            if (!ui->send_show->text().isEmpty())
                ui->send_show->clear();
        }
        test_datanum = 0;
    }
#ifdef SCAN_SET
    if (my_ctrl && !drag_L.flag)
    {
        d_angle += M_PI / 720 * (p_flag ? 1 : -1);
        if (d_angle > 2*M_PI)
            d_angle = 0;
        if (d_angle < 0)
            d_angle = 2*M_PI;
    }
#endif

    if (ctrlAuto_flag)
    {
        if (ctrlAuto_time < 10*1000/timerTime)
        {
            ctrlAuto_time++; //$timerTime ms
        }
        else
        {
            if (CSwindows_flag)
            {
                CSwindows_flag = false;
                MyAnimationRetract();
            }
        }

    }
    else
    {
        ctrlAuto_time = -1;
    }

    update();
}

bool Widget::SaveNewFile(H_file_data_ data)
{
    QFileDialog dialog;

    QString filePath = dialog.getSaveFileName(this,
                                              "Save File",
                                              "data",
                                              "Text Files (*.data)");

    if (!filePath.isEmpty())
    {
        QFile file(filePath);
        // if (!file.exists())
        {
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QMessageBox::critical(0, "错误", "无法创建文件");
                return false;
            }

            QTextStream out(&file);
            for (auto &m : data.data)
            {
                for (auto n : m)
                    out << "D:" << n.ranges_ << " A:" << n.angles_ << " C:" << n.intensity_ << '\n';

                out << '\n';
            }

            file.close();
        }

        return true;
    }

    return false;
}

void Widget::HistoryData(QString fileName)
{
    std::vector<std::string> m_vctLine;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            m_vctLine.push_back(line.toStdString());
            // qDebug() << line;
        }
        file.close();
    }

    if (!m_vctLine.empty())
    {
        std::vector<W_DataScan> circle_data;
        for (auto m : m_vctLine)
        {
            bool not_circle = false;
            char fine_head = 0;
            bool fine_num = false;
            bool negative = false;
            float fraction_times = 1;
            float Distance = -1, Angle = 1000;
            uint8_t Confidence = 0;
            for (size_t i = 0; i < m.size(); i++)
            {
                char now_data = m[i];

                if (now_data >= '0' && now_data <= '9')
                    not_circle = true;

                if (fine_head == 0)
                {
                    if (i >= 1 && (now_data == ':') &&
                        ((m[i-1] == 'D') || (m[i-1] == 'A') || (m[i-1] == 'C')))
                    {
                        fine_head = m[i-1];
                        fine_num = false;
                        // negative = false;
                        fraction_times = 1;
                    }
                    continue;
                }

                switch (fine_head)
                {
                case 'D':
                    if (now_data >= '0' && now_data <= '9')
                    {
                        fine_num = true;
                        if (fraction_times > 1)
                        {
                            Distance += ((now_data - '0') * 1.0 / fraction_times);
                            fraction_times *= 10;
                        }
                        else
                        {
                            if (Distance == -1)
                                Distance = 0;
                            Distance *= 10;
                            Distance += (now_data - '0');
                        }
                    }
                    else if (now_data == '.')
                    {
                        if (fraction_times > 1)
                        {
                            fine_head = 0;
                            fine_num = false;
                            fraction_times = 1;
                        }
                        else
                            fraction_times = 10;
                    }
                    else
                    {
                        if (fine_num)
                        {
                            fine_head = 0;
                            fine_num = false;
                            fraction_times = 1;
                        }
                    }
                    break;
                case 'A':
                    if (now_data >= '0' && now_data <= '9')
                    {
                        fine_num = true;
                        if (fraction_times > 1)
                        {
                            Angle += ((now_data - '0') * 1.0 / fraction_times);
                            fraction_times *= 10;
                        }
                        else
                        {
                            if (fabs(Angle) == 1000)
                                Angle = 0;
                            Angle *= 10;
                            Angle += (now_data - '0');
                        }
                        if (m[i-1] == '-')
                        {
                            negative = true;
                        }
                    }
                    else if (now_data == '.')
                    {
                        if (fraction_times > 1)
                        {
                            fine_head = 0;
                            fine_num = false;
                            fraction_times = 1;
                        }
                        else
                            fraction_times = 10;
                    }
                    else
                    {
                        if (fine_num)
                        {
                            fine_head = 0;
                            fine_num = false;
                            fraction_times = 1;
                        }
                    }
                    break;
                case 'C':
                    if (now_data >= '0' && now_data <= '9')
                    {
                        fine_num = true;
                        Confidence *= 10;
                        Confidence += (now_data - '0');
                    }
                    else
                    {
                        if (fine_num)
                        {
                            fine_head = 0;
                            fine_num = false;
                            fraction_times = 1;
                        }
                    }
                    break;
                default:
                    fine_head = 0;
                    fine_num = false;
                    fraction_times = 1;
                    break;
                }
            }
            if (!not_circle)
            {
                if (!circle_data.empty())
                    history_file_data.data.push_back(circle_data);
                circle_data.clear();
            }
            if (Distance != -1 && fabs(Angle) != 1000)
            {
                W_DataScan m_disData;
                m_disData.ranges_ = Distance;
                m_disData.angles_ = negative ? -Angle : Angle;
                m_disData.intensity_ = Confidence;
                circle_data.push_back(m_disData);
                // qDebug() << Distance << Angle << Confidence << m_disData.angles_ << negative;
            }
        }
        if (!circle_data.empty())
            history_file_data.data.push_back(circle_data);

        if (!history_file_data.data.empty())
        {
            m_scandata = history_file_data.data.front();
            history_file_data.now_idx = 0;

            // char openfile_T[256];
            // sprintf(openfile_T, " {%d/%d}", history_file_data.now_idx+1, history_file_data.data.size());
            // ui->label_openfile->setText(history_file_data.open_file_Tips + openfile_T);
            ui->label_openfile->setText(history_file_data.open_file_Tips +
                " {" + QString::number(history_file_data.now_idx+1) + "/" + QString::number(history_file_data.data.size()) + "}");
        }
    }
    // qDebug() << fileName << history_file_opened << m_vctLine.size();
}

void Widget::pushPoint(std::vector<W_DataScan> data)
{
    m_scandata = data;
    if (test_datanum <= set_datanum)
        test_datanum++;

    if (OpenenType.compare("LB_L") == 0 || OpenenType.compare("LB_LOther") == 0)
    {
        line_speed.push_back(m_scandata.front().speed);
        if (line_speed.size() > 60)
            line_speed.erase(line_speed.begin());
    }

    if (save_data_mutex.tryLock())
    {
        save_temp_data.data.push_back(data);
        if (save_temp_data.data.size() > 100)
            save_temp_data.data.erase(save_temp_data.data.begin());

        save_data_mutex.unlock();
    }
    else
    {
        // qDebug() << "Failed to acquire lock. Doing something else.";
    }

    update();
}

void Widget::normal_drawText(QPainter *MPainter, float  x, float y, float w, float h, float font_s, const QString &str, bool fl)
{
    float x_y0 = -1, x_ym = -1, y_x0 = -1, y_xm = -1;
    float line_rad = atan2((getPoint(fl).y() - y), (getPoint(fl).x() - x));

    if (fabs(line_rad) < M_PI/2 && x <= 0)//与left边相交
        y_x0 = (0-getPoint(fl).x()) /(x-getPoint(fl).x()) * (y-getPoint(fl).y()) + getPoint(fl).y();
    else if (fabs(line_rad) > M_PI/2 && x >= getWidth())//与right边相交
        y_xm = (getWidth()-getPoint(fl).x()) /(x-getPoint(fl).x()) * (y-getPoint(fl).y()) + getPoint(fl).y();

    if (line_rad > 0 && line_rad < M_PI && y <= 0 )//与top边相交
        x_y0 = (0-getPoint(fl).y()) /(y-getPoint(fl).y()) * (x-getPoint(fl).x()) + getPoint(fl).x();
    else if (line_rad < 0 && line_rad > -M_PI && y >= getHeight())//与bottom边相交
        x_ym = (getHeight()-getPoint(fl).y()) /(y-getPoint(fl).y()) * (x-getPoint(fl).x()) + getPoint(fl).x();

    QFont font = MPainter->font();
    font.setPointSize(font_s);
    MPainter->setFont(font);
    float dis_h = font_s*10;
    float dis_hy = font_s*16;

    if (y_x0 >= 0 && y_x0 <= getHeight() && getPoint(fl).x() >= 0)
    {
        float dis_t = hypot(getPoint(fl).x(), getPoint(fl).y() - y_x0);
        if (dis_t < dis_h)
        {
            float _f = font_s*dis_t/dis_h;
            if (_f < 1)
                _f = 1;
            font.setPointSize(_f);
            MPainter->setFont(font);
        }
        MPainter->drawText(0, y_x0 - h/2.0, w, h, Qt::AlignLeft|Qt::AlignVCenter, str);
    }
    else if (y_xm >= 0 && y_xm <= getHeight() && getPoint(fl).x() <= getWidth())
    {
        float dis_t = hypot(getPoint(fl).x() - getWidth(), getPoint(fl).y() - y_xm);
        if (dis_t < dis_h)
        {
            float _f = font_s*dis_t/dis_h;
            if (_f < 1)
                _f = 1;
            font.setPointSize(_f);
            MPainter->setFont(font);
        }
        MPainter->drawText(getWidth()-w, y_xm - h/2.0,w, h, Qt::AlignRight|Qt::AlignVCenter, str);
    }
    else if (x_y0 >= 0 && x_y0 <= getWidth() && getPoint(fl).y() >= 0)
    {
        float dis_t = hypot(getPoint(fl).x() - x_y0, getPoint(fl).y());
        if (dis_t < dis_hy)
        {
            float _f = font_s*dis_t/dis_hy;
            if (_f < 1)
                _f = 1;
            font.setPointSize(_f);
            MPainter->setFont(font);
        }
        MPainter->drawText(x_y0 - w/2.0, 0,w, h, Qt::AlignTop|Qt::AlignHCenter, str);
    }
    else if (x_ym >= 0 && x_ym <= getWidth() && getPoint(fl).y() <= getHeight())
    {
        float dis_t = hypot(getPoint(fl).x() - x_ym, getPoint(fl).y() - getHeight());
        if (dis_t < dis_hy)
        {
            float _f = font_s*dis_t/dis_hy;
            if (_f < 1)
                _f = 1;
            font.setPointSize(_f);
            MPainter->setFont(font);
        }
        MPainter->drawText(x_ym - w/2.0, getHeight()-h,w, h, Qt::AlignBottom|Qt::AlignHCenter, str);
    }
}

bool Widget::custom_drawText(QPainter *MPainter, float  x, float y, float w, float h, float angle, float font_s, const QString &str)
{
    bool res = false;

    QFont font = MPainter->font();
    font.setPointSize(font_s);
    MPainter->setFont(font);

    if (x > 0 && (x < getWidth())  && y > 0  && (y < getHeight()))
    {
        float dis_h = font_s*16;
        float dis_t = hypot(getPoint().x() - x, getPoint().y() - y);
        if (dis_t < dis_h)
        {
            float _f = font_s*dis_t/dis_h;
            if (_f < 1)
                _f = 1;
            font.setPointSize(_f);
            MPainter->setFont(font);
        }
        MPainter->translate(x, y); // 移动到中心
        MPainter->rotate(angle);
        // MPainter->translate(-x, -y); // 恢复到原来位置
        MPainter->drawText(-w/2.0, -h/2.0, w, h, Qt::AlignCenter, str);
        // MPainter->translate(x, y);
        MPainter->rotate(-angle);
        MPainter->translate(-x, -y);
        res = true;
    }
    else
    {
        normal_drawText(MPainter, x, y, w, h, font_s, str);
        res = false;
    }

    return res;
}

QPen Widget::MDotLinePen(const QColor &color, qreal width,
                  Qt::PenCapStyle c, Qt::PenJoinStyle j)
{
    QPen pen;
    pen.setColor(color);
    pen.setWidthF(width);
    // pen.setStyle(Qt::DotLine);
    pen.setDashPattern(QVector<qreal>() << 1 << 3);
    pen.setCapStyle(c);
    pen.setJoinStyle(j);

    return pen;
}

QPixmap Widget::paintWidget_time()
{
    QPixmap pixmap_time(getWidth(), Side_end.y);
    QPainter p_painter_time(&pixmap_time);
    p_painter_time.setRenderHint(QPainter::Antialiasing);
    QPalette palette = this->palette();
    QColor backgroundColor = palette.color(QPalette::Window); // 对于窗口背景使用Window角色
    QColor fontColor = palette.color(QPalette::WindowText);
    pixmap_time.fill(backgroundColor);

    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("yyyy/MM/dd hh:mm:ss");
    int text_w =130;
    int text_h = 20;
    QFont font = p_painter_time.font();
    font.setPointSize(8);
    p_painter_time.setFont(font);
    p_painter_time.setPen(QPen(fontColor, 2, Qt::SolidLine));
    p_painter_time.drawText(pixmap_time.width() - text_w, -2, text_w, text_h, Qt::AlignLeft, timeStr);

    statusPonit->move(25, Side_head.y+getHeight());

    return pixmap_time;
}

QPixmap Widget::paintWidget()
{
    QPixmap pixmap(getWidth(), getHeight());
    QPainter p_painter(&pixmap);
    // QPen pen;
    // pen.setColor(Qt::green);
    // p_painter.setPen(pen);

    //反锯齿
    p_painter.setRenderHint(QPainter::Antialiasing);
    pixmap.fill(m_background_color);
    QColor reverse_Bcolor;
    reverse_Bcolor.setRgb(abs(m_background_color.red()-128), abs(m_background_color.green()-128), abs(m_background_color.blue()-128));
    QColor reverse_Bcolor_mkline;
    reverse_Bcolor_mkline.setRgb(abs(m_background_color.red()-255), abs(m_background_color.green()-255), abs(m_background_color.blue()-255));
    // reverse_Bcolor.setAlpha(180);

    QFont font = p_painter.font();
    font.setPointSize(9);
    p_painter.setFont(font);
    int text_w =100;
    int text_h = 30;
    float Reduction_ = 15000;
    if (ui->DeviceBox->currentText().compare("LB_L") == 0 || ui->DeviceBox->currentText().compare("LB_LOther") == 0)
        Reduction_ = 500;

    /***********************极坐标角度***************************/
    float LinelenAdd = 20*Display_factor;
    float Linelength = getRadius() + (LinelenAdd > 10 ? 10 : LinelenAdd);
    float deviation = getRadius() + text_h/6.0;
    int identification_num = 36;//24
    bool outside_flag = false;
    for (int i = 0; i < identification_num; i++)
    {
        p_painter.setPen(QPen(m_line_color, 1, Qt::SolidLine));
        QString A_Text(QString::number(360/identification_num*i));
        A_Text += "°";
        float draw_X = p_flag ? getPoint().x() + deviation*cos(A_TO_RAD(-360/identification_num*i + rot_angle))
                              : getPoint().x() + deviation*cos(A_TO_RAD(360/identification_num*i + rot_angle));
        float draw_Y = p_flag ? getPoint().y() + deviation*sin(A_TO_RAD(-360/identification_num*i + rot_angle))
                              : getPoint().y() + deviation*sin(A_TO_RAD(360/identification_num*i + rot_angle));
        float draw_T = p_flag ? (-360/identification_num*i + (rot_angle + 90))
                              : (360/identification_num*i + (rot_angle + 90));
        bool m_flag = custom_drawText(&p_painter, draw_X, draw_Y, text_w, text_h, draw_T, 9, A_Text);
        if (!outside_flag)
            outside_flag = m_flag;

        if (i == 0)
            p_painter.setPen(QPen(reverse_Bcolor_mkline, 0.3, Qt::SolidLine));
        else if (((int)(360/identification_num*i))%30 == 0)
            p_painter.setPen(QPen(reverse_Bcolor, 0.2, Qt::SolidLine));
        else
            p_painter.setPen(MDotLinePen(reverse_Bcolor, 0.2));
        p_painter.drawLine(getPoint().x() + Linelength * cos(A_TO_RAD(360/identification_num*i + rot_angle)),
                           getPoint().y() + Linelength * sin(A_TO_RAD(360/identification_num*i + rot_angle)),
                           getPoint().x() , getPoint().y());
    }
    /***********************边界***************************/
    if (!outside_flag && !(getPoint().x() > 0 && getPoint().x() < getWidth()  && getPoint().y() > 0  && getPoint().y() < getHeight()))
    {
        p_painter.setPen(QPen(m_line_color, 2, Qt::SolidLine));
        float O_pdis = hypot(getPoint().x()-getPoint(true).x(), getPoint().y()-getPoint(true).y());
        normal_drawText(&p_painter, getPoint().x(),getPoint().y(), text_w, text_h, 9, "P["+QString::number(O_pdis, 'f', 0)+"]", true);
    }
    p_painter.setPen(QPen(m_line_color, 1, Qt::SolidLine));
    if (getPoint().x() < 0)
        p_painter.drawLine(0, 0, 0, getHeight());
    else if (getPoint().x() > getWidth())
        p_painter.drawLine(getWidth(), 0, getWidth(), getHeight());
    if (getPoint().y() < 0)
        p_painter.drawLine(0, 0, getWidth(), 0);
    else if (getPoint().y() > getHeight())
        p_painter.drawLine(0, getHeight(), getWidth(), getHeight());

    /***********************极坐标距离***************************/
    int R_n = (int)getRadius() / 60;
    int circle_num = 1;
    for (int n = 5; n <= Reduction_; n+=5)
    {
        if ((Reduction_ / R_n) < n && (int)Reduction_ % n == 0)
        {
            circle_num = 5*Reduction_/n;
            break;
        }
    }

    for (int i = 1; i <= circle_num; i++)
    {
        if (i == circle_num)
            p_painter.setPen(QPen(reverse_Bcolor, 0.5, Qt::SolidLine));
        else if (i % 5 == 0)
            p_painter.setPen(MDotLinePen(reverse_Bcolor, 0.35));
        else
            p_painter.setPen(MDotLinePen(reverse_Bcolor, 0.2));
        p_painter.drawEllipse(getPoint().x() - getRadius()*i/circle_num, getPoint().y() - getRadius()*i/circle_num,
                              getDiameter()*i/circle_num, getDiameter()*i/circle_num);

        if (i % 5 == 0 || circle_num < 2)
        {
            p_painter.setPen(QPen(reverse_Bcolor_mkline, 1, Qt::SolidLine));
            font.setPointSize(9);
            p_painter.setFont(font);
            QString mark_d = QString::number(static_cast<int>(Reduction_/circle_num*i));
            p_painter.drawText(getPoint().x()+cos(A_TO_RAD(rot_angle))*(getRadius()*i/circle_num),
                               getPoint().y()+sin(A_TO_RAD(rot_angle))*(getRadius()*i/circle_num),mark_d);
            // custom_drawText(&p_painter, getPoint().x()+cos(A_TO_RAD(rot_angle))*(getRadius()*i/circle_num),
            //                             getPoint().y()+sin(A_TO_RAD(rot_angle))*(getRadius()*i/circle_num), text_w, text_h, 0, 9, mark_d);

            p_painter.setPen(QPen(reverse_Bcolor_mkline, 1, Qt::SolidLine));
            // p_painter.drawPoint(getPoint().x()+getRadius()*cos(A_TO_RAD(rot_angle))*i/circle_num,
            //                     getPoint().y()+getRadius()*sin(A_TO_RAD(rot_angle))*i/circle_num);
            p_painter.drawLine(getPoint().x()+getRadius()*cos(A_TO_RAD(rot_angle))*i/circle_num,
                               getPoint().y()+getRadius()*sin(A_TO_RAD(rot_angle))*i/circle_num,
                               getPoint().x()+getRadius()*cos(A_TO_RAD(rot_angle))*i/circle_num - 5*cos(A_TO_RAD(rot_angle+90)),
                               getPoint().y()+getRadius()*sin(A_TO_RAD(rot_angle))*i/circle_num - 5*sin(A_TO_RAD(rot_angle+90)));
        }
        else
        {
            p_painter.setPen(QPen(reverse_Bcolor_mkline, 0.3, Qt::SolidLine));
            p_painter.drawLine(getPoint().x()+getRadius()*cos(A_TO_RAD(rot_angle))*i/circle_num,
                               getPoint().y()+getRadius()*sin(A_TO_RAD(rot_angle))*i/circle_num,
                               getPoint().x()+getRadius()*cos(A_TO_RAD(rot_angle))*i/circle_num - 3*cos(A_TO_RAD(rot_angle+90)),
                               getPoint().y()+getRadius()*sin(A_TO_RAD(rot_angle))*i/circle_num - 3*sin(A_TO_RAD(rot_angle+90)));
        }
    }

    /***********************点云***************************/
    if (!m_scandata.empty())
    {
        VDATAcreatRows(m_scandata);

        W_DataScan mark_point;
        int mark_id = -1;
        int Effective_num = 0;
        float rememberDiff = 1000;
        float rememberDiff_d = 0x8fff;
        QString speedValue = "Speed: ";
        QString DatasumValue = "DataNum: ";

        if (OpenenType.compare("LB_L") == 0 || OpenenType.compare("LB_LOther") == 0)
        {
            uint16_t average = 0;
            if (line_speed.size() > 0)
                average = std::accumulate(line_speed.begin(), line_speed.end(), 0) / line_speed.size();

            speedValue += QString::number(m_scandata.front().speed);
            speedValue += " (";
            speedValue += QString::number(average);
            speedValue += ")";
            speedValue += " Hz";
        }
        else if (OpenenType.compare("LB_RHC") == 0 || OpenenType.compare("LB_DBZ") == 0)
        {
            speedValue += QString::number(m_scandata.front().speed/60.0, 'f', 2);
            speedValue += "/6 Hz";
        }
        else
        {
            speedValue += QString::number(m_scandata.front().speed/360.0, 'f', 2);
            speedValue += "/10 Hz";
        }

        if (!history_file_data.data.empty())
        {
            speedValue = "DLocation: ";
            speedValue +=  QString::number(history_file_data.now_idx+1);
            speedValue += "/";
            speedValue +=  QString::number(history_file_data.data.size());
        }

        float Reduction_factor = getRadius() / Reduction_;
        p_painter.setPen(QPen(m_point_color, m_point_pixel, Qt::SolidLine));
        int counts = -1;
        for (auto m : m_scandata)
        {
            counts++;

            int drawpointX = (p_flag) ? getPoint().x() + (m.x()*Reduction_factor*cos(A_TO_RAD(rot_angle-2*m.angles_)) - m.y()*Reduction_factor*sin(A_TO_RAD(rot_angle-2*m.angles_)))
                                      : getPoint().x() + (m.x()*Reduction_factor*cos(A_TO_RAD(rot_angle)) - m.y()*Reduction_factor*sin(A_TO_RAD(rot_angle)));
            int drawpointY = (p_flag) ? getPoint().y() + (m.x()*Reduction_factor*sin(A_TO_RAD(rot_angle-2*m.angles_)) + m.y()*Reduction_factor*cos(A_TO_RAD(rot_angle-2*m.angles_)))
                                      : getPoint().y() + (m.x()*Reduction_factor*sin(A_TO_RAD(rot_angle)) + m.y()*Reduction_factor*cos(A_TO_RAD(rot_angle)));
            float new_angle = m.angles_;
            while (new_angle > 360)
                new_angle -= 360;
            while (new_angle < 0)
                new_angle += 360;

            float diff_angle = fabs(new_angle-measure_point.angles_);
            if (diff_angle > 180)
                diff_angle = 360 - diff_angle;
            float diff_dis = fabs(m.ranges_-measure_point.ranges_);

            if (rememberDiff > diff_angle ||
                (rememberDiff == diff_angle && rememberDiff_d > diff_dis))
            {
                mark_point = m;
                mark_id = counts;
                rememberDiff = diff_angle;
                rememberDiff_d = diff_dis;
            }

            if (m.ranges_ > 0)
                Effective_num ++;

            p_painter.drawPoint(drawpointX, drawpointY);
        }
        DatasumValue += QString::number(Effective_num);
        DatasumValue += "/";
        DatasumValue += QString::number(m_scandata.size());
        text_w = 180;
        font.setPointSize(11);
        p_painter.setFont(font);
        p_painter.setPen(QPen(m_line_color, 2, Qt::SolidLine));
        p_painter.drawText(pixmap.width() - text_w, 20, text_w, text_h, Qt::AlignLeft, speedValue);
        p_painter.drawText(pixmap.width() - text_w, 40, text_w, text_h, Qt::AlignLeft, DatasumValue);

        if (Show_indicator_line)
        {
            QString ranges_Value = QString::number(mark_point.ranges_, 'f', 1);
            QString angles_Value = QString::number(mark_point.angles_, 'f', 2);
            QString intensity_Value = QString::number(mark_point.intensity_);
            p_painter.setPen(QPen(m_line_color, 0.8, Qt::DashLine));
            int draw_endX = (p_flag) ? getPoint().x() + (mark_point.x()*Reduction_factor*cos(A_TO_RAD(rot_angle-2*mark_point.angles_)) - mark_point.y()*Reduction_factor*sin(A_TO_RAD(rot_angle-2*mark_point.angles_)))
                                     : getPoint().x() + (mark_point.x()*Reduction_factor*cos(A_TO_RAD(rot_angle)) - mark_point.y()*Reduction_factor*sin(A_TO_RAD(rot_angle)));
            int draw_endY = (p_flag) ? getPoint().y() + (mark_point.x()*Reduction_factor*sin(A_TO_RAD(rot_angle-2*mark_point.angles_)) + mark_point.y()*Reduction_factor*cos(A_TO_RAD(rot_angle-2*mark_point.angles_)))
                                     :getPoint().y() + (mark_point.x()*Reduction_factor*sin(A_TO_RAD(rot_angle)) + mark_point.y()*Reduction_factor*cos(A_TO_RAD(rot_angle)));
            p_painter.drawLine(getPoint().x(), getPoint().y(), draw_endX, draw_endY);

            float target_rad = A_TO_RAD(mark_point.angles_);
            float target_dis = mark_point.ranges_ * Reduction_factor + 10;
            QString show_data;
            if (Show_indicator_distance)
            {
                if (!show_data.isEmpty())
                    show_data += ", ";
                show_data += ranges_Value;
            }
            if (Show_indicator_angle)
            {
                if (!show_data.isEmpty())
                    show_data += ", ";
                show_data += angles_Value;
            }
            if (Show_indicator_confidence)
            {
                if (!show_data.isEmpty())
                    show_data += ", ";
                show_data += intensity_Value;
            }
            text_w =250;
            p_painter.setPen(QPen(m_line_color, 2, Qt::SolidLine));
            float mfsize = 10;
            mfsize *= (sqrt(getRadius())/18);
            if (mfsize < 5)
                mfsize = 5;
            else if (mfsize > 16)
                mfsize = 16;
            font.setPointSize(mfsize);
            p_painter.setFont(font);
            int draw_TextX = (p_flag) ? getPoint().x()+target_dis*cos(A_TO_RAD(rot_angle)-target_rad) - text_w/2.0
                                      : getPoint().x()+target_dis*cos(A_TO_RAD(rot_angle)+target_rad) - text_w/2.0;
            int draw_TextY = (p_flag) ? getPoint().y()+target_dis*sin(A_TO_RAD(rot_angle)-target_rad) - text_h/2.0
                                      : getPoint().y()+target_dis*sin(A_TO_RAD(rot_angle)+target_rad) - text_h/2.0;;
            p_painter.drawText(draw_TextX, draw_TextY, text_w, text_h, Qt::AlignCenter, show_data);

            if (updata_select && mark_id != -1 && ui->tableWidget_scan->rowCount() >= mark_id+1)
            {
                updata_select = false;
                ui->tableWidget_scan->setCurrentCell(mark_id, 0);
                // ui->tableWidget_scan->setFocus();
                ui->tableWidget_scan->clearSelection();
                selectionModel = ui->tableWidget_scan->selectionModel();
                QItemSelection rowSelection(ui->tableWidget_scan->model()->index(mark_id, 0),
                                            ui->tableWidget_scan->model()->index(mark_id, ui->tableWidget_scan->columnCount() - 1));
                // 设置选中这个区域
                selectionModel->blockSignals(true); // 阻止信号
                selectionModel->select(rowSelection, QItemSelectionModel::Select | QItemSelectionModel::Rows); // QItemSelectionModel::NoUpdate
                selectionModel->blockSignals(false); // 恢复信号
            }
        }

    }

    if (OpenedCom.isEmpty() && history_file_data.data.empty())
    {
        m_scandata.clear();
        line_speed.clear();
        updata_select = false;
        VDATAcreatRows(m_scandata);

        QString w_text = "欢 迎 使 用";
#ifdef SCAN_SET
        if (my_ctrl)
        {
            p_painter.setPen(QPen(reverse_Bcolor, 0.8, Qt::SolidLine));
            float sector = 0.2;//扇形大小
            QConicalGradient conical_gradient(getPoint(), d_angle / (2*M_PI) * 720);//定义圆心和渐变的角度
            if (p_flag)
            {
                std::reverse(w_text.begin(), w_text.end());
                conical_gradient.setColorAt(1, m_line_color);
                conical_gradient.setColorAt((1-sector), QColor(255, 255, 255, 0));
            }
            else
            {
                conical_gradient.setColorAt(0, m_line_color);
                conical_gradient.setColorAt(sector, QColor(255, 255, 255, 0));
            }
            p_painter.setBrush(conical_gradient);
            p_painter.drawEllipse(getPoint().x() - getRadius(), getPoint().y() - getRadius(), getDiameter(), getDiameter());
            p_painter.setPen(QPen(m_background_color, 2, Qt::SolidLine));
        }
        else
#endif
        {
            if (p_flag)
                std::reverse(w_text.begin(), w_text.end());
            p_painter.setPen(QPen(m_line_color, 2, Qt::SolidLine));
        }
        int m_f = 15;
        m_f *= (sqrt(getRadius())/18);
        if (m_f > 40)
            m_f = 40;
        font.setPointSize(m_f);
        p_painter.setFont(font);
        p_painter.drawText(getPoint().x()-m_f*3.25, getPoint().y()-getRadius()/2, w_text);
    }

    return pixmap;

}

void Widget::Resetdeviation()
{
    // my_ctrl = false;
    // ui->send_show->clear();
    updata_select = false;
    save_temp_data.data.clear();
    history_file_opened = false;
    history_file_data.data.clear();
    history_file_data.open_file_Tips.clear();
    ui->label_openfile->clear();
    m_scandata.clear();
    line_speed.clear();
    measure_point = {0,0,0,0};
    point_deviation = {0,0};
    Display_factor = 1.0;
    Display_factor_last = 1.0;
    drag_L = {.start={-1, -1}, .end={-1, -1}, .flag=false};
    drag_R = {.start={-1, -1}, .end={-1, -1}, .flag=false};
}
