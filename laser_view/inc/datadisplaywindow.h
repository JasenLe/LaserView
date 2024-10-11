#ifndef DATADISPLAYWINDOW_H
#define DATADISPLAYWINDOW_H
#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMenu>
#include <QContextMenuEvent>
#include <QScrollBar>
#include <QMutex>

class DataDisplayWindow : public QWidget
{
    Q_OBJECT
public:
    DataDisplayWindow(QWidget *parent = nullptr, QString title = nullptr);
    ~DataDisplayWindow();

    void InsertData(const QByteArray &data);
    void callbackobjclose(std::function<void()> callback)
    {
        closeFun = callback;
    }

protected:
    void closeEvent(QCloseEvent *event)
    {
        if (closeFun != nullptr)
            closeFun();
        QWidget::closeEvent(event);
    }

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    QTextEdit *outputLabel = nullptr;
    std::function<void()> closeFun = nullptr;
    bool _char_flag{false};
    bool Data_concatenation_flag{false};
    QMutex insert_data_mutex;
};

#endif // DATADISPLAYWINDOW_H
