QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
# LIBS += -L$$PWD/lib/libldlidar_driver_ssl_shared.so
# LIBS += -L$$PWD/lib/libldlidar_driver_ssl.a

SOURCES += \
    linelaser/eailine.cpp \
    linelaser/ldline.cpp \
    linelaser/llsdk.cpp \
    linelaser/rsline.cpp \
    linelaser/yxline.cpp \
    radar/fullscanfilter.cpp \
    radar/lpkg.cpp \
    radar/morefilter.cpp \
    src/datadisplaywindow.cpp \
    src/inifile.cpp \
    src/main.cpp \
    src/radarscan.cpp \
    src/widget.cpp \

HEADERS += \
    inc/comreadthread.hpp \
    inc/datadisplaywindow.h \
    inc/inifile.h \
    inc/radarscan.h \
    inc/widget.h \
    lib/pointdata.h \
    linelaser/eailine.h \
    linelaser/ldline.h \
    linelaser/llsdk.h \
    linelaser/llsdk_base.h \
    linelaser/rsline.h \
    linelaser/rsline_tool.h \
    linelaser/yxline.h \
    radar/fullscanfilter.h \
    radar/lpkg.h \
    radar/morefilter.h \

FORMS += \
    widget.ui

RC_ICONS = LB.ico
#RC_ICONS = Linkbey.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    lib/libldlidar_driver_ssl.a \
    lib/libldlidar_driver_ssl_shared.so

RESOURCES += \
    rec.qrc
