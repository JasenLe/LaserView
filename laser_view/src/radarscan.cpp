#include "inc/widget.h"
#include "inc/radarscan.h"

int Widget::getWidth()
{
    int m_Width_offset = Side_end.x;
    if (Width_offset < m_Width_offset)
        m_Width_offset = Width_offset;
    return (this->width() - (Width_offset + m_Width_offset));
}
int Widget::getHeight()
{
    return (this->height() - (Side_head.y + Side_end.y));
}
int Widget::getDiameter()
{
    return i_diameter*Display_factor;
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
    QPoint last_P = getPoint();

    point = QPoint( getWidth() / 2.0, getHeight() / 2.0);
    if (this->getWidth() > this->getHeight())
        i_diameter = getHeight() - 20;
    else
        i_diameter = getWidth() - 20;

    measure_point.x += getPoint().x() - last_P.x();
    measure_point.y += getPoint().y() - last_P.y();
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
        measure_point.x = (init_x - (init_x - measure_point.x)*(Display_factor/Display_factor_last));
        measure_point.y = (init_y - (init_y - measure_point.y)*(Display_factor/Display_factor_last));
        Display_factor_last = Display_factor;

        update();
        // qDebug() << "wheelEvent*" << numDegrees << numSteps;
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
        if (o_x > Width_offset && o_x < Width_offset + getWidth()  && o_y > Side_head.y && o_y < Side_head.y + getHeight())
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
            if (drag_L.flag &&
                drag_L.start.x != -1 && drag_L.start.y != -1 &&
                drag_L.end.x != -1 && drag_L.end.y != -1)
            {
                drag_L = {.start={-1, -1}, .end={-1, -1}, .flag=false};

                if (move_miss_flag)
                {
                    move_miss_flag = false;
                }
                else
                {
                    measure_point.x = init_x;
                    measure_point.y = init_y;
                }
            }
            else
            {
                measure_point.x = init_x;
                measure_point.y = init_y;
            }
        }
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

                drag_R = {.start={-1, -1}, .end={-1, -1}, .flag=false};

                if (hypot(fabs(now_diff_y), fabs(now_diff_x)) > 2)
                {
                    contextMenu_miss_flag = true;
                    point_deviation.x += now_diff_x;
                    point_deviation.y += now_diff_y;
                    measure_point.x += now_diff_x;
                    measure_point.y += now_diff_y;
                }
                else
                    contextMenu_miss_flag = false;
            }
            else
            {
                contextMenu_miss_flag = false;
            }
        }
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
                measure_point.x += now_diff_x;
                measure_point.y += now_diff_y;

                moving.x = init_x;
                moving.y = init_y;
                move_miss_flag = true;
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
            // menu.setStyleSheet("QMenu { background-color: white; color: black; }");
            /************************Background************************/
            QAction *action_Background_color = menu.addAction("Background color");
            action_Background_color->setCheckable(false);
            /************************Show indicator line************************/
            QAction *action_indicator = menu.addAction("Show indicator line");
            action_indicator->setCheckable(true); // 设置为可勾选
            if (this->Show_indicator_line)
                action_indicator->setChecked(true);
            else
                action_indicator->setChecked(false);

            /************************Indicator line items************************/
            QMenu *action_indicator_other = menu.addMenu("Indicator line items");
            QAction *indicator_other_distance = action_indicator_other->addAction("Show distance");
            QAction *indicator_other_angle = action_indicator_other->addAction("Show angle");
            QAction *indicator_other_confidence = action_indicator_other->addAction("Show confidence");
            QAction *line_color = action_indicator_other->addAction("line color");
            indicator_other_distance->setCheckable(true);
            indicator_other_angle->setCheckable(true);
            indicator_other_confidence->setCheckable(true);
            line_color->setCheckable(false);

            if (Show_indicator_distance)
                indicator_other_distance->setChecked(true);
            else
                indicator_other_distance->setChecked(false);

            if (Show_indicator_angle)
                indicator_other_angle->setChecked(true);
            else
                indicator_other_angle->setChecked(false);

            if (Show_indicator_confidence)
                indicator_other_confidence->setChecked(true);
            else
                indicator_other_confidence->setChecked(false);

            /************************Enable filter************************/
            QAction *action_filter = nullptr;
            if (!OpenenType.isEmpty())
            {
                action_filter = menu.addAction("Enable filter");
                action_filter->setCheckable(true);
                if (this->Enable_filte)
                    action_filter->setChecked(true);
                else
                    action_filter->setChecked(false);
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
            if (OpenenType.compare("LB_R0") == 0 || OpenenType.compare("LB_R1") == 0)
            {
                /************************filter_items************************/
                QMenu *action_filter_items =  menu.addMenu("Filter setting items");
                filter_items_smooth = action_filter_items->addAction("smooth filter");
                filter_items_bilateral = action_filter_items->addAction("bilateral filter");
                filter_items_tail = action_filter_items->addAction("tail filter");
                filter_items_intensity = action_filter_items->addAction("intensity filter");
                filter_items_Near = action_filter_items->addAction("near filter");
                filter_items_Noise = action_filter_items->addAction("noise filter");
                filter_items_Tine = action_filter_items->addAction("tine filter");
                filter_items_Wander = action_filter_items->addAction("wander filter");
                filter_items_Shadows = action_filter_items->addAction("shadows filter");
                filter_items_Median = action_filter_items->addAction("median filter");
                filter_items_smooth->setCheckable(true);
                filter_items_bilateral->setCheckable(true);
                filter_items_tail->setCheckable(true);
                filter_items_intensity->setCheckable(true);
                filter_items_Near->setCheckable(true);
                filter_items_Noise->setCheckable(true);
                filter_items_Tine->setCheckable(true);
                filter_items_Wander->setCheckable(true);
                filter_items_Shadows->setCheckable(true);
                filter_items_Median->setCheckable(true);

                if ((filter_select & (filter_smooth & 0xffff)) != 0)
                    filter_items_smooth->setChecked(true);
                else
                    filter_items_smooth->setChecked(false);

                if ((filter_select & (filter_bilateral & 0xffff)) != 0)
                    filter_items_bilateral->setChecked(true);
                else
                    filter_items_bilateral->setChecked(false);

                if ((filter_select & (filter_tail & 0xffff)) != 0)
                    filter_items_tail->setChecked(true);
                else
                    filter_items_tail->setChecked(false);

                if ((filter_select & (filter_intensity & 0xffff)) != 0)
                    filter_items_intensity->setChecked(true);
                else
                    filter_items_intensity->setChecked(false);

                if ((filter_select & (filter_near & 0xffff)) != 0)
                    filter_items_Near->setChecked(true);
                else
                    filter_items_Near->setChecked(false);

                if ((filter_select & (filter_noise & 0xffff)) != 0)
                    filter_items_Noise->setChecked(true);
                else
                    filter_items_Noise->setChecked(false);

                if ((filter_select & (filter_tine & 0xffff)) != 0)
                    filter_items_Tine->setChecked(true);
                else
                    filter_items_Tine->setChecked(false);

                if ((filter_select & (filter_wander & 0xffff)) != 0)
                    filter_items_Wander->setChecked(true);
                else
                    filter_items_Wander->setChecked(false);

                if ((filter_select & (filter_shadows & 0xffff)) != 0)
                    filter_items_Shadows->setChecked(true);
                else
                    filter_items_Shadows->setChecked(false);

                if ((filter_select & (filter_median & 0xffff)) != 0)
                    filter_items_Median->setChecked(true);
                else
                    filter_items_Median->setChecked(false);
            }

            /************************point pixel************************/
            QMenu *action_point_pixel = menu.addMenu("Point info items");
            QAction *point_pixel_0_5 = action_point_pixel->addAction("0.5x");
            QAction *point_pixel_1_0 = action_point_pixel->addAction("1.0x");
            QAction *point_pixel_1_5 = action_point_pixel->addAction("1.5x");
            QAction *point_pixel_2_0 = action_point_pixel->addAction("2.0x");
            QAction *point_pixel_3_0 = action_point_pixel->addAction("3.0x");
            QAction *point_pixel_3_5 = action_point_pixel->addAction("3.5x");
            QAction *point_color = action_point_pixel->addAction("point color");
            point_pixel_0_5->setCheckable(true);
            point_pixel_1_0->setCheckable(true);
            point_pixel_1_5->setCheckable(true);
            point_pixel_2_0->setCheckable(true);
            point_pixel_3_0->setCheckable(true);
            point_pixel_3_5->setCheckable(true);
            point_color->setCheckable(false);
            if (m_point_pixel == 0.5)
            {
                point_pixel_0_5->setChecked(true);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(false);
            }
            else if (m_point_pixel == 1.5)
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(true);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(false);
            }
            else if (m_point_pixel == 2.0)
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(true);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(false);
            }
            else if (m_point_pixel == 3.0)
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(true);
                point_pixel_3_5->setChecked(false);
            }
            else if (m_point_pixel == 3.5)
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(true);
            }
            else
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(true);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(false);
            }
            /************************Data display window************************/
            QAction *action_display_window = menu.addAction("Data display window");
            action_display_window->setCheckable(true);
            if (window != nullptr)
                action_display_window->setChecked(true);
            else
                action_display_window->setChecked(false);
            /************************other message************************/
            QString m_message;
            QAction *action_other_message = nullptr;
            if (OpenenType.compare("LB_L") == 0 || OpenenType.compare("LB_LOther") == 0)
            {
                if (linelaser != nullptr)
                    m_message = linelaser->GetOtherMessage();
            }
            else
            {
                if (lidar != nullptr)
                    m_message = lidar->GetOtherMessage();
            }
            if (!m_message.isEmpty())
            {
                action_other_message = menu.addAction("Other message");
                action_other_message->setCheckable(false);
            }
            /************************Check in history file************************/
            QAction *action_history_file = nullptr;
            if (OpenenType.isEmpty())
            {
                action_history_file = menu.addAction("Check in history file");
                action_history_file->setCheckable(true);
                if (history_file_opened)
                    action_history_file->setChecked(true);
                else
                    action_history_file->setChecked(false);
            }
            /************************Save file************************/
            QAction *action_save_file = nullptr;
            if (save_temp_data.data.size() > 1)
            {
                action_save_file = menu.addAction("Save current data");
                action_save_file->setCheckable(false);
            }

            // 显示菜单
            QAction *selectedAction = menu.exec(event->globalPos());

            if (action_Background_color != nullptr && selectedAction == action_Background_color)
            {
                QColor color = QColorDialog::getColor(m_background_color, this);
                if (color.isValid())
                {
                    m_background_color = color;
                    setIniint(BCOLOR, m_background_color.red()*1000000+m_background_color.green()*1000+m_background_color.blue());
                }
            }
            /*****************************************************************************************/
            else if (action_indicator !=nullptr && selectedAction == action_indicator)
            {
                this->Show_indicator_line = !this->Show_indicator_line;
                setIniint(INDICATORLINE, (int)this->Show_indicator_line);
                if (this->Show_indicator_line)
                    action_indicator->setChecked(true);
                else
                    action_indicator->setChecked(false);
            }
            else if (indicator_other_distance !=nullptr && selectedAction == indicator_other_distance)
            {
                this->Show_indicator_distance = !this->Show_indicator_distance;
                setIniint(LINEDIS, (int)this->Show_indicator_distance);
                if (this->Show_indicator_distance)
                    indicator_other_distance->setChecked(true);
                else
                    indicator_other_distance->setChecked(false);
            }
            else if (indicator_other_angle != nullptr && selectedAction == indicator_other_angle)
            {
                this->Show_indicator_angle = !this->Show_indicator_angle;
                setIniint(LINEANGLE, (int)this->Show_indicator_angle);
                if (this->Show_indicator_angle)
                    indicator_other_angle->setChecked(true);
                else
                    indicator_other_angle->setChecked(false);
            }
            else if (indicator_other_confidence != nullptr && selectedAction == indicator_other_confidence)
            {
                this->Show_indicator_confidence = !this->Show_indicator_confidence;
                setIniint(LINECONF, (int)this->Show_indicator_confidence);
                if (this->Show_indicator_confidence)
                    indicator_other_confidence->setChecked(true);
                else
                    indicator_other_confidence->setChecked(false);
            }
            else if (line_color != nullptr && selectedAction == line_color)
            {
                QColor color = QColorDialog::getColor(m_line_color, this);
                if (color.isValid())
                {
                    m_line_color = color;
                    setIniint(LINECOLOR, m_line_color.red()*1000000+m_line_color.green()*1000+m_line_color.blue());
                }
            }
            /*****************************************************************************************/
            else if (action_filter !=nullptr && selectedAction == action_filter)
            {
                this->Enable_filte = !this->Enable_filte;
                setIniint(FLITER, (int)this->Enable_filte);
                if (this->Enable_filte)
                {
                    action_filter->setChecked(true);
                    if (lidar != nullptr)
                        lidar->SetFilter(this->filter_select);
                }
                else
                {
                    action_filter->setChecked(false);
                    if (lidar != nullptr)
                        lidar->SetFilter(0);
                }

                if (linelaser != nullptr)
                    linelaser->SetFilter(this->Enable_filte);
            }
            else if (filter_items_smooth !=nullptr && selectedAction == filter_items_smooth)
            {
                if ((this->filter_select & (filter_smooth & 0xffff)) != 0)
                    this->filter_select &= ~(filter_smooth & 0xffff);
                else
                    this->filter_select |= (filter_smooth & 0xffff);

                if ((this->filter_select & (filter_smooth & 0xffff)) != 0)
                    filter_items_smooth->setChecked(true);
                else
                    filter_items_smooth->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            else if (filter_items_bilateral != nullptr && selectedAction == filter_items_bilateral)
            {
                if ((this->filter_select & (filter_bilateral & 0xffff)) != 0)
                    this->filter_select &= ~(filter_bilateral & 0xffff);
                else
                    this->filter_select |= (filter_bilateral & 0xffff);

                if ((this->filter_select & (filter_bilateral & 0xffff)) != 0)
                    filter_items_bilateral->setChecked(true);
                else
                    filter_items_bilateral->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            else if (filter_items_tail != nullptr && selectedAction == filter_items_tail)
            {
                if ((this->filter_select & (filter_tail & 0xffff)) != 0)
                    this->filter_select &= ~(filter_tail & 0xffff);
                else
                    this->filter_select |= (filter_tail & 0xffff);

                if ((this->filter_select & (filter_tail & 0xffff)) != 0)
                    filter_items_tail->setChecked(true);
                else
                    filter_items_tail->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            else if (filter_items_intensity != nullptr && selectedAction == filter_items_intensity)
            {
                if ((this->filter_select & (filter_intensity & 0xffff)) != 0)
                    this->filter_select &= ~(filter_intensity & 0xffff);
                else
                    this->filter_select |= (filter_intensity & 0xffff);

                if ((this->filter_select & (filter_intensity & 0xffff)) != 0)
                    filter_items_intensity->setChecked(true);
                else
                    filter_items_intensity->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            else if (filter_items_Near != nullptr && selectedAction == filter_items_Near)
            {
                if ((this->filter_select & (filter_near & 0xffff)) != 0)
                    this->filter_select &= ~((filter_near & 0xffff));
                else
                    this->filter_select |= (filter_near & 0xffff);

                if ((this->filter_select & (filter_near & 0xffff)) != 0)
                    filter_items_Near->setChecked(true);
                else
                    filter_items_Near->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            else if (filter_items_Noise != nullptr && selectedAction == filter_items_Noise)
            {
                if ((this->filter_select & (filter_noise & 0xffff)) != 0)
                    this->filter_select &= ~(filter_noise & 0xffff);
                else
                    this->filter_select |= (filter_noise & 0xffff);

                if ((this->filter_select & (filter_noise & 0xffff)) != 0)
                    filter_items_Noise->setChecked(true);
                else
                    filter_items_Noise->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            else if (filter_items_Tine != nullptr && selectedAction == filter_items_Tine)
            {
                if ((this->filter_select & (filter_tine & 0xffff)) != 0)
                    this->filter_select &= ~(filter_tine & 0xffff);
                else
                    this->filter_select |= (filter_tine & 0xffff);

                if ((this->filter_select & (filter_tine & 0xffff)) != 0)
                    filter_items_Tine->setChecked(true);
                else
                    filter_items_Tine->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            else if (filter_items_Wander != nullptr && selectedAction == filter_items_Wander)
            {
                if ((this->filter_select & (filter_wander & 0xffff)) != 0)
                    this->filter_select &= ~(filter_wander & 0xffff);
                else
                    this->filter_select |= (filter_wander & 0xffff);

                if ((this->filter_select & (filter_wander & 0xffff)) != 0)
                    filter_items_Wander->setChecked(true);
                else
                    filter_items_Wander->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            else if (filter_items_Shadows != nullptr && selectedAction == filter_items_Shadows)
            {
                if ((this->filter_select & (filter_shadows & 0xffff)) != 0)
                    this->filter_select &= ~(filter_shadows & 0xffff);
                else
                    this->filter_select |= (filter_shadows & 0xffff);

                if ((this->filter_select & (filter_shadows & 0xffff)) != 0)
                    filter_items_Shadows->setChecked(true);
                else
                    filter_items_Shadows->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            else if (filter_items_Median != nullptr && selectedAction == filter_items_Median)
            {
                if ((this->filter_select & (filter_median & 0xffff)) != 0)
                    this->filter_select &= ~(filter_median & 0xffff);
                else
                    this->filter_select |= (filter_median & 0xffff);

                if ((this->filter_select & (filter_median & 0xffff)) != 0)
                    filter_items_Median->setChecked(true);
                else
                    filter_items_Median->setChecked(false);

                if (lidar != nullptr && Enable_filte)
                    lidar->SetFilter(this->filter_select);
                else if (!Enable_filte)
                    lidar->SetFilter(0);

                setIniint(FLITER_all, (int)this->filter_select);
            }
            /*****************************************************************************************/
            else if (point_pixel_0_5 !=nullptr && selectedAction == point_pixel_0_5)
            {
                point_pixel_0_5->setChecked(true);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(false);
                this->m_point_pixel = 0.5;
                setInifloat(PPIXEL, 0.5);
            }
            else if (point_pixel_1_0 != nullptr && selectedAction == point_pixel_1_0)
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(true);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(false);
                this->m_point_pixel = 1.0;
                setInifloat(PPIXEL, 1.0);
            }
            else if (point_pixel_1_5 != nullptr && selectedAction == point_pixel_1_5)
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(true);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(false);
                this->m_point_pixel = 1.5;
                setInifloat(PPIXEL, 1.5);
            }
            else if (point_pixel_2_0 != nullptr && selectedAction == point_pixel_2_0)
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(true);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(false);
                this->m_point_pixel = 2.0;
                setInifloat(PPIXEL, 2.0);
            }
            else if (point_pixel_3_0 != nullptr && selectedAction == point_pixel_3_0)
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(true);
                point_pixel_3_5->setChecked(false);
                this->m_point_pixel = 3.0;
                setInifloat(PPIXEL, 3.0);
            }
            else if (point_pixel_3_5 != nullptr && selectedAction == point_pixel_3_5)
            {
                point_pixel_0_5->setChecked(false);
                point_pixel_1_0->setChecked(false);
                point_pixel_1_5->setChecked(false);
                point_pixel_2_0->setChecked(false);
                point_pixel_3_0->setChecked(false);
                point_pixel_3_5->setChecked(true);
                this->m_point_pixel = 3.5;
                setInifloat(PPIXEL, 3.5);
            }
            else if (point_color != nullptr && selectedAction == point_color)
            {
                QColor color = QColorDialog::getColor(m_point_color, this);
                if (color.isValid())
                {
                    m_point_color = color;
                    setIniint(PCOLOR, m_point_color.red()*1000000+m_point_color.green()*1000+m_point_color.blue());
                }
            }
            /*****************************************************************************************/
            else if (action_display_window != nullptr && selectedAction == action_display_window)
            {
                if (window != nullptr)
                {
                    closeWindow();
                    action_display_window->setChecked(false);
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
                    action_display_window->setChecked(true);
                }
            }
            /*****************************************************************************************/
            else if (action_other_message != nullptr && selectedAction == action_other_message)
            {
                QMessageBox::information(this, "Other message",  m_message);
            }
            /*****************************************************************************************/
            else if (action_history_file != nullptr && selectedAction == action_history_file)
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
                        history_file_data.open_file_Tips = "格式[D:%f A:%f C:%d],其中:{(非负值)D:Distance/mm - A:Angle/° - (非负值)C:conf/8bit}<br>已打开文件："
                                                            +fileName+"<br>注：可上下方向键切换多帧数据";
                        ui->label_openfile->setText(history_file_data.open_file_Tips);
                        HistoryData(fileName);
                    }
                }

                if (this->history_file_opened)
                    action_history_file->setChecked(true);
                else
                    action_history_file->setChecked(false);
            }
            /*****************************************************************************************/
            else if (action_save_file != nullptr && selectedAction == action_save_file)
            {
                QMutexLocker lock(&save_data_mutex);
                SaveNewFile(save_temp_data);
                lock.unlock();
                // save_temp_data.data.clear();
            }

        }
        else if (o_x > Side_head.x && o_x < Width_offset)
        {
            QMenu menu(this);
            // menu.setStyleSheet("QMenu { border-radius: 5px; background-color: white; color: black; }");
            /************************C&S Background************************/
            QAction *action_CS_Background_color = menu.addAction("C/S background color");
            action_CS_Background_color->setCheckable(false);
            QAction *action_Instructions = menu.addAction("Instructions(How to use)");
            action_Instructions->setCheckable(false);

            QAction *selectedAction = menu.exec(event->globalPos());
            if (action_CS_Background_color != nullptr && selectedAction == action_CS_Background_color)
            {
                QColor color = QColorDialog::getColor(CS_background_color, this);
                if (color.isValid())
                {
                    CS_background_color = color;
                    ChangeCSBackgroundColor(CS_background_color);
                    setIniint(CSCOLOR, CS_background_color.red()*1000000+CS_background_color.green()*1000+CS_background_color.blue());
                }
            }
            else if (action_Instructions != nullptr && selectedAction == action_Instructions)
            {
                // QProcess::startDetached("cmd", QStringList() << "/c" << appPath + "/laser_view使用说明.pdf");
                QProcess::startDetached("cmd", QStringList() << "/c" << appPath.replace("&", "^&") + "/laser_view使用说明.pdf");
            }
        }
    }
    else if (o_y > Side_head.y + getHeight())
    {
        QMenu menu(this);
        /************************Console and SensorInf windows************************/
        QAction *action_CSwindows = menu.addAction("Console and SensorInf window");
        action_CSwindows->setCheckable(true);
        action_CSwindows->setChecked(CSwindows_flag);
        /************************Recovery Default Style************************/
        QAction *action_Default_Style = menu.addAction("Recovery Default Style");
        action_Default_Style->setCheckable(false);

        QAction *selectedAction = menu.exec(event->globalPos());
        if (action_CSwindows != nullptr && selectedAction == action_CSwindows)
        {
            CSwindows_flag = !CSwindows_flag;
            action_Default_Style->setChecked(CSwindows_flag);
            if (CSwindows_flag)
            {
                Width_offset = 200 + Side_head.x;
                deviation = {.x=Width_offset, .y=Side_head.y};
                ui->SCwidget->show();
            }
            else
            {
                Width_offset = 0;
                deviation = {.x=Width_offset, .y=Side_head.y};
                ui->SCwidget->hide();
            }
            m_resizeEvent();
            // ui->SCwidget->setVisible(false);
        }
        else if (action_Default_Style != nullptr && selectedAction == action_Default_Style)
        {
            DeleteMemoryFile();
        }
    }
}

void Widget::closeWindow()
{
    if (window != nullptr)
    {
        MwindowSize = {(float)window->width(), (float)window->height()};
        MwindowPos = {(float) window->pos().x(), (float) window->pos().y()};
        delete window;
        window = nullptr;
    }
}

void Widget::timerTimeOut()
{
    timer->start(20);

    if (test_count > 0)
        test_count--;
    else
    {
        static QString last_text = "";
        QString now_text = ui->lineEdit_send->text();
        now_text.remove(' ');
        if (now_text.compare("74657374") == 0) //"test"
        {
            if (test_flag)
            {
                on_Button_Start_clicked();
                test_flag = false;
                test_count = 100;
            }
            else
            {
                if (test_datanum > 7)
                {
                    on_Button_Standby_clicked();
                    test_flag = true;
                }
                test_count = 50;
            }
            if (last_text.compare(now_text) != 0)
                ui->send_show->setText("test");
        }
        else
        {
            test_count = 50;
            if (last_text.compare(now_text) != 0)
                ui->send_show->clear();
        }
        test_datanum = 0;
        last_text = now_text;
    }

    d_angle += M_PI / 720;
    if (d_angle >= 2*M_PI)
    {
        d_angle = 0;
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
                    else if (now_data >= '.')
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
                    else if (now_data >= '.')
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
    test_datanum++;
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

void Widget::custom_drawText(QPainter *MPainter, int x, int y, int w, int h, float angle, const QString &str)
{
    MPainter->translate(x, y); // 移动到中心
    MPainter->rotate(angle);
    MPainter->translate(-x, -y); // 恢复到原来位置
    MPainter->drawText(x-w/2.0, y-h/2.0, w, h, Qt::AlignCenter, str);
    MPainter->translate(x, y);
    MPainter->rotate(-angle);
    MPainter->translate(-x, -y);
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
    p_painter_time.setPen(QPen(fontColor, 2, Qt::DashLine));
    p_painter_time.drawText(pixmap_time.width() - text_w, -2, text_w, text_h, Qt::AlignLeft, timeStr);

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
    // reverse_Bcolor.setAlpha(180);

    float rot_angle = -90; //极坐标角度旋转
    int LinelenAdd = 20*Display_factor;
    int Linelength = getDiameter() / 2.0 + (LinelenAdd > 50 ? 50 : LinelenAdd);
    QFont font = p_painter.font();
    font.setPointSize(11);
    p_painter.setFont(font);
    int text_w =100;
    int text_h = 30;
    int deviation = getDiameter()/2.0 + text_h/6.0;
    int identification_num = 12;//24
    for (int i = 0; i < identification_num; i++)
    {
        p_painter.setPen(QPen(m_line_color, 2, Qt::SolidLine));
        QString A_Text(QString::number(360/identification_num*i));
        A_Text += "°";
        custom_drawText(&p_painter, getPoint().x() + deviation*cos(A_TO_RAD(360/identification_num*i + rot_angle)),
                                    getPoint().y() + deviation*sin(A_TO_RAD(360/identification_num*i + rot_angle)),
                                    text_w, text_h, (360/identification_num*i + (rot_angle + 90)), A_Text);

        p_painter.setPen(QPen(reverse_Bcolor, 0.5, Qt::DashLine));
        p_painter.drawLine(getPoint().x() + Linelength * cos(A_TO_RAD(360/identification_num*i + rot_angle)),
                           getPoint().y() + Linelength * sin(A_TO_RAD(360/identification_num*i + rot_angle)),
                           getPoint().x() , getPoint().y());

        p_painter.setPen(QPen(reverse_Bcolor, 0.2, Qt::DashLine));
        p_painter.drawLine(getPoint().x() + getDiameter() / 2.0 * cos(A_TO_RAD(360/(identification_num*2)*(i*2+1) + rot_angle)),
                           getPoint().y() + getDiameter() / 2.0 * sin(A_TO_RAD(360/(identification_num*2)*(i*2+1) + rot_angle)),
                           getPoint().x() , getPoint().y());
    }

    int circle_num = 10;
    // p_painter.setPen(QPen(Qt::gray, 0.5, Qt::DashLine));
    p_painter.setPen(QPen(reverse_Bcolor, 0.5, Qt::DashLine));
    for (int i = 0; i <= circle_num; i++)
    {
        p_painter.drawEllipse(getPoint().x() - getDiameter()*1.0*i/circle_num/ 2, getPoint().y() - getDiameter()*1.0*i/circle_num / 2,
                                     getDiameter()*1.0*i/circle_num, getDiameter()*1.0*i/circle_num);
    }

    if (!m_scandata.empty())
    {
        W_DataScan mark_point;
        int Effective_num = 0;
        float Reduction_factor = 0;
        float rememberDiff = 1000;
        QString speedValue = "Speed: ";
        QString DatasumValue = "DataNum: ";
        if (OpenenType.compare("LB_L") == 0 || OpenenType.compare("LB_LOther") == 0)
        {
            line_speed.push_back(m_scandata.front().speed);
            if (line_speed.size() > 100)
                line_speed.erase(line_speed.begin());
            uint16_t sum = std::accumulate(line_speed.begin(), line_speed.end(), 0);
            uint16_t average = sum / line_speed.size();

            Reduction_factor = getDiameter()*1.0/2/500;
            speedValue += QString::number(m_scandata.front().speed);
            speedValue += " (";
            speedValue += QString::number(average);
            speedValue += ")";
            speedValue += " Hz";
        }
        else
        {
            Reduction_factor = getDiameter()*1.0/2/15000;
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

        p_painter.setPen(QPen(m_point_color, m_point_pixel, Qt::SolidLine));
        for (auto m : m_scandata)
        {
            // int drawpointX = getPoint().x()+m.y()*Reduction_factor;
            // int drawpointY = getPoint().y()-m.x()*Reduction_factor;
            int drawpointX = getPoint().x() + (m.x()*Reduction_factor*cos(A_TO_RAD(rot_angle)) - m.y()*Reduction_factor*sin(A_TO_RAD(rot_angle)));
            int drawpointY = getPoint().y() + (m.x()*Reduction_factor*sin(A_TO_RAD(rot_angle)) + m.y()*Reduction_factor*cos(A_TO_RAD(rot_angle)));
            float new_rad = A_TO_RAD(m.angles_+rot_angle);
            while (new_rad > M_PI)
                new_rad -= 2*M_PI;
            while (new_rad < -M_PI)
                new_rad += 2*M_PI;

            float target_rad = atan2((measure_point.y - getPoint().y()), (measure_point.x - getPoint().x()));
            float diff_rad = fabs(new_rad-target_rad);
            if (diff_rad > M_PI)
                diff_rad = 2*M_PI - diff_rad;
            if (rememberDiff > diff_rad)
            {
                mark_point = m;
                rememberDiff = diff_rad;
            }
            if (m.ranges_ > 0)
                Effective_num ++;

            p_painter.drawPoint(drawpointX, drawpointY);
        }
        DatasumValue += QString::number(Effective_num);
        DatasumValue += "/";
        DatasumValue += QString::number(m_scandata.size());
        text_w = 180;
        p_painter.setPen(QPen(m_line_color, 2, Qt::SolidLine));
        p_painter.drawText(pixmap.width() - text_w, 20, text_w, text_h, Qt::AlignLeft, speedValue);
        p_painter.drawText(pixmap.width() - text_w, 40, text_w, text_h, Qt::AlignLeft, DatasumValue);

        if (Show_indicator_line)
        {
            QString ranges_Value = QString::number(mark_point.ranges_, 'f', 1);
            QString angles_Value = QString::number(mark_point.angles_, 'f', 3);
            QString intensity_Value = QString::number(mark_point.intensity_);
            p_painter.setPen(QPen(m_line_color, 0.8, Qt::DashLine));
            p_painter.drawLine(getPoint().x(), getPoint().y(),
                getPoint().x() + (mark_point.x()*Reduction_factor*cos(A_TO_RAD(rot_angle)) - mark_point.y()*Reduction_factor*sin(A_TO_RAD(rot_angle))),
                getPoint().y() + (mark_point.x()*Reduction_factor*sin(A_TO_RAD(rot_angle)) + mark_point.y()*Reduction_factor*cos(A_TO_RAD(rot_angle))));

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
            QFont font = p_painter.font();
            font.setPointSize(16);
            p_painter.setFont(font);
            p_painter.drawText(getPoint().x()+target_dis*sin(target_rad) - text_w/2.0, getPoint().y()-target_dis*cos(target_rad) - text_h/2.0, text_w, text_h, Qt::AlignCenter, show_data);
        }

    }

    if (OpenedCom.isEmpty() && history_file_data.data.empty())
    {
        m_scandata.clear();
        line_speed.clear();

        // p_painter.setPen(QPen(Qt::gray, 0.5, Qt::DashLine));
        p_painter.setPen(QPen(reverse_Bcolor, 0.8, Qt::SolidLine));
        QConicalGradient conical_gradient(getPoint(), (2*M_PI - d_angle) / (2*M_PI) * 720);//定义圆心和渐变的角度
        conical_gradient.setColorAt(0, m_line_color);
        conical_gradient.setColorAt(0.2, QColor(255, 255, 255, 0));
        p_painter.setBrush(conical_gradient);
        p_painter.drawEllipse(getPoint().x() - getDiameter() / 2.0, getPoint().y() - getDiameter() / 2.0, getDiameter(), getDiameter());
    }

    return pixmap;

}

int Widget::getIniint(const string &key, int value)
{
    QString filename = appPath+"/"+SOFTSC;
    std::string openfile = filename.toLocal8Bit().toStdString();
    IniFileSTL m_file(openfile);

    /*******************WIDGET***********************/
    if (key.compare(WWIDTH) == 0 || key.compare(WHEIGHT) == 0)
    {
        return m_file.ReadInt(WIDGET, key, value);
    }
    /*******************CONSOLE***********************/
    else if (key.compare(DEVICE) == 0 || key.compare(BAND) == 0)
        return m_file.ReadInt(CONSOLE, key, value);
    /*******************BACKGROUND***********************/
    else if (key.compare(BCOLOR) == 0 || key.compare(CSCOLOR) == 0)
        return m_file.ReadInt(BACKGROUND, key, value);
    /*******************INDICATORLINE***********************/
    else if (key.compare(INDICATORLINE) == 0)
        return m_file.ReadInt(INDICATORLINE, "EN", value);
    else if (key.compare(LINEDIS) == 0 || key.compare(LINEANGLE) == 0 || key.compare(LINECONF) == 0 || key.compare(LINECOLOR) == 0)
        return m_file.ReadInt(INDICATORLINE, key, value);

    /*******************FLITER***********************/
    else if (key.compare(FLITER) == 0)
        return m_file.ReadInt(FLITER, "EN", value);
    else if (key.compare(FLITER_all) == 0)
    {
        uint16_t m_v = (uint16_t)value;
        uint16_t o_v = 0;
        o_v |= (m_file.ReadInt(FLITER, F_SMOOTH, (m_v & (filter_smooth & 0xffff))));
        o_v |= ((m_file.ReadInt(FLITER, F_BILATERAL, ((m_v & (filter_bilateral & 0xffff)) >> 1)) << 1) & 0xffff);
        o_v |= ((m_file.ReadInt(FLITER, F_TAIL, ((m_v & (filter_tail & 0xffff)) >> 2)) << 2) & 0xffff);
        o_v |= ((m_file.ReadInt(FLITER, F_INTENSITY, ((m_v & (filter_intensity & 0xffff)) >> 3)) << 3) & 0xffff);
        o_v |= ((m_file.ReadInt(FLITER, F_NEAR, ((m_v & (filter_near & 0xffff)) >> 4)) << 4) & 0xffff);
        o_v |= ((m_file.ReadInt(FLITER, F_NOISE, ((m_v & (filter_noise & 0xffff)) >> 5)) << 5) & 0xffff);
        o_v |= ((m_file.ReadInt(FLITER, F_TINE, ((m_v & (filter_tine & 0xffff)) >> 6)) << 6) & 0xffff);
        o_v |= ((m_file.ReadInt(FLITER, F_WANDER, ((m_v & (filter_wander & 0xffff)) >> 7)) << 7) & 0xffff);
        o_v |= ((m_file.ReadInt(FLITER, F_SHADOWS, ((m_v & (filter_shadows & 0xffff)) >> 8)) << 8) & 0xffff);
        o_v |= ((m_file.ReadInt(FLITER, F_MEDIAN, ((m_v & (filter_median & 0xffff)) >> 9)) << 9) & 0xffff);

        return (int)o_v;
    }
    /*******************PONITPIXEL***********************/
    else if (key.compare(PCOLOR) == 0)
        return m_file.ReadInt(PONITPIXEL, key, value);

    return value;
}
float Widget::getInifloat(const string &key, float value)
{
    QString filename = appPath+"/"+SOFTSC;
    std::string openfile = filename.toLocal8Bit().toStdString();
    IniFileSTL m_file(openfile);

    /*******************PONITPIXEL***********************/
    if (key.compare(PPIXEL) == 0)
        return m_file.ReadFloat(PONITPIXEL, key, value);

    return value;
}
string Widget::getString(const string &key, string value)
{
    QString filename = appPath+"/"+SOFTSC;
    std::string openfile = filename.toLocal8Bit().toStdString();
    IniFileSTL m_file(openfile);

    /*******************SEND***********************/
    if (key.compare(SDATA) == 0)
        return m_file.ReadString(SEND, key, value);

    return value;
}
bool Widget::setIniint(const string &key, int value)
{
    QString filename = appPath+"/"+SOFTSC;
    std::string openfile = filename.toLocal8Bit().toStdString();
    IniFileSTL m_file(openfile);

    /*******************WIDGET***********************/
    if (key.compare(WWIDTH) == 0 || key.compare(WHEIGHT) == 0)
    {
        m_file.WriteInt(WIDGET, key, value);
    }
    /*******************CONSOLE***********************/
    else if (key.compare(DEVICE) == 0 || key.compare(BAND) == 0)
        m_file.WriteInt(CONSOLE, key, value);
    /*******************BACKGROUND***********************/
    else if (key.compare(BCOLOR) == 0 || key.compare(CSCOLOR) == 0)
        m_file.WriteInt(BACKGROUND, key, value);
    /*******************INDICATORLINE***********************/
    else if (key.compare(INDICATORLINE) == 0)
        m_file.WriteInt(INDICATORLINE, "EN", value);
    else if (key.compare(LINEDIS) == 0 || key.compare(LINEANGLE) == 0 || key.compare(LINECONF) == 0 || key.compare(LINECOLOR) == 0)
        m_file.WriteInt(INDICATORLINE, key, value);

    /*******************FLITER***********************/
    else if (key.compare(FLITER) == 0)
        m_file.WriteInt(FLITER, "EN", value);
    else if (key.compare(FLITER_all) == 0)
    {
        uint16_t m_v = (uint16_t)value;
        m_file.WriteInt(FLITER, F_SMOOTH, (m_v & (filter_smooth & 0xffff)));
        m_file.WriteInt(FLITER, F_BILATERAL, ((m_v & (filter_bilateral & 0xffff)) >> 1));
        m_file.WriteInt(FLITER, F_TAIL, ((m_v & (filter_tail & 0xffff)) >> 2));
        m_file.WriteInt(FLITER, F_INTENSITY, ((m_v & (filter_intensity & 0xffff)) >> 3));
        m_file.WriteInt(FLITER, F_NEAR, ((m_v & (filter_near & 0xffff)) >> 4));
        m_file.WriteInt(FLITER, F_NOISE, ((m_v & (filter_noise & 0xffff)) >> 5));
        m_file.WriteInt(FLITER, F_TINE, ((m_v & (filter_tine & 0xffff)) >> 6));
        m_file.WriteInt(FLITER, F_WANDER, ((m_v & (filter_wander & 0xffff)) >> 7));
        m_file.WriteInt(FLITER, F_SHADOWS, ((m_v & (filter_shadows & 0xffff)) >> 8));
        m_file.WriteInt(FLITER, F_MEDIAN, ((m_v & (filter_median & 0xffff)) >> 9));
    }
    /*******************PONITPIXEL***********************/
    else if (key.compare(PCOLOR) == 0)
        m_file.WriteInt(PONITPIXEL, key, value);

    return m_file.WriteFile();
}
bool Widget::setInifloat(const string &key, float value)
{
    QString filename = appPath+"/"+SOFTSC;
    std::string openfile = filename.toLocal8Bit().toStdString();
    IniFileSTL m_file(openfile);

    /*******************PONITPIXEL***********************/
    if (key.compare(PPIXEL) == 0)
        m_file.WriteFloat(PONITPIXEL, key, value);

    return m_file.WriteFile();
}

bool Widget::setString(const string &key, string value)
{
    QString filename = appPath+"/"+SOFTSC;
    std::string openfile = filename.toLocal8Bit().toStdString();
    IniFileSTL m_file(openfile);

    /*******************SEND***********************/
    if (key.compare(SDATA) == 0)
        m_file.WriteString(SEND, key, value);

    return m_file.WriteFile();
}

void Widget::Resetdeviation()
{
    save_temp_data.data.clear();
    history_file_opened = false;
    history_file_data.data.clear();
    history_file_data.open_file_Tips.clear();
    ui->label_openfile->clear();
    m_scandata.clear();
    line_speed.clear();
    measure_point = {0,0};
    point_deviation = {0,0};
    Display_factor = 1.0;
    Display_factor_last = 1.0;
    drag_L = {.start={-1, -1}, .end={-1, -1}, .flag=false};
    drag_R = {.start={-1, -1}, .end={-1, -1}, .flag=false};
}
