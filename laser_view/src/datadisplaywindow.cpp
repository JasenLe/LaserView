#include "inc/datadisplaywindow.h"

DataDisplayWindow::DataDisplayWindow(QWidget *parent, QString title)
{
    outputLabel = new QTextEdit(this);
    outputLabel->setReadOnly(true);
    outputLabel->setFont(QFont("Courier New", 10));
    outputLabel->setUndoRedoEnabled(false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(outputLabel);
    this->setLayout(layout);

    this->resize(500, 400);

    // 设置窗口标题
    if (title == nullptr)
        this->setWindowTitle("Data display window");
    else
        this->setWindowTitle(title+" -> Data display window");
}

DataDisplayWindow::~DataDisplayWindow()
{
    delete outputLabel;
}

void DataDisplayWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    // menu.setStyleSheet("QMenu { border-radius: 5px; background-color: white; color: black; }");
    QAction *Clera_all = menu.addAction("Clear all");
    Clera_all->setCheckable(false);

    QAction *Turn_hex_char = menu.addAction("Hex/char");
    Turn_hex_char->setCheckable(false);

    QAction *Data_concatenation = menu.addAction("Data concatenation");
    Data_concatenation->setCheckable(true);
    if (Data_concatenation_flag)
        Data_concatenation->setChecked(true);
    else
        Data_concatenation->setChecked(false);

    QAction *selectedAction = menu.exec(event->globalPos());

    QMutexLocker lock(&insert_data_mutex);
    if (Clera_all !=nullptr && selectedAction == Clera_all)
    {
        outputLabel->clear();
    }
    else if (Turn_hex_char !=nullptr && selectedAction == Turn_hex_char)
    {
        _char_flag = !_char_flag;
    }
    else if (Data_concatenation !=nullptr && selectedAction == Data_concatenation)
    {
        Data_concatenation_flag = !Data_concatenation_flag;
        if (Data_concatenation_flag)
            Data_concatenation->setChecked(true);
        else
            Data_concatenation->setChecked(false);
    }
    lock.unlock();
}

void DataDisplayWindow::InsertData(const QByteArray &data)
{
    QMutexLocker lock(&insert_data_mutex);
    int Barlen_now_s = outputLabel->verticalScrollBar()->value();
    int Barlen_max_s = outputLabel->verticalScrollBar()->maximum();
    int pageStep_s = outputLabel->verticalScrollBar()->pageStep();
    QTextDocument *document = outputLabel->document();
    uint64_t rowCount = outputLabel->toPlainText().size() + data.size()*3;
    uint64_t maxRowNumber = 0x400 * 0x400 * 0x400;//设定字符最大容量
    if (Data_concatenation_flag)
        maxRowNumber = 0x8000;//30000
    if(rowCount > maxRowNumber)
    {
        int var_num = std::min(int(rowCount - maxRowNumber), (int)(outputLabel->toPlainText().size()));
        QTextCursor cursor = QTextCursor(document); // 创建光标对象
        cursor.movePosition(QTextCursor::Start); //移动到开头
        for (int var = 0; var < var_num; ++var)
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor); // 向下移动并选中当前行

        cursor.removeSelectedText();//删除选择的文本
    }

    if (Data_concatenation_flag)
    {
        if (_char_flag)
        {
            // static QByteArray data_temp;
            // data_temp += data;
            // if (data_temp.endsWith("\r\n") || data_temp.size() > 1024)
            // {
            //     QString str = QString::fromLocal8Bit(data_temp);
            //     outputLabel->insertPlainText(str);
            //     data_temp.clear();
            // }
            QString str = QString::fromLocal8Bit(data);
            outputLabel->insertPlainText(str);
        }
        else
        {
            // outputLabel->insertPlainText(" ");
            outputLabel->insertPlainText(" "+data.toHex(' '));
        }
    }
    else
    {
        if (_char_flag)
        {
            // static QByteArray data_temp;
            // data_temp += data;
            // if (data_temp.endsWith("\r\n") || data_temp.size() > 1024)
            // {
            //     QString str = QString::fromLocal8Bit(data_temp);
            //     outputLabel->insertPlainText(str);
            //     data_temp.clear();
            // }
            QString str = QString::fromLocal8Bit(data);
            outputLabel->append(str);
        }
        else
            outputLabel->append(data.toHex(' '));
    }

    int Barlen_now = outputLabel->verticalScrollBar()->value();
    int Barlen_max = outputLabel->verticalScrollBar()->maximum();
    int pageStep = outputLabel->verticalScrollBar()->pageStep();
    if ((Barlen_max - Barlen_now) <= pageStep || (Barlen_max_s - Barlen_now_s) <= pageStep_s)
       outputLabel->moveCursor(QTextCursor::End);

    outputLabel->update();
    lock.unlock();
    // qDebug() << "len " << Barlen_now << Barlen_max << outputLabel->toPlainText().size() ;
}
