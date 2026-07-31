QT += core widgets gui serialport websockets

CONFIG += c++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/thirdparty/scpi/inc
LIBS += -L$$PWD/thirdparty/scpi/lib -lscpi -lm

HEADERS += \
    auxiliary/battery_model.h \
    auxiliary/scpi_handle.h \
    auxiliary/simple_logger.h \
    auxiliary/config_manager.h \
    auxiliary/vxinamespace.h \
    channel/can_channel.h \
    channel/uart_channel.h \
    control/can_server.h \
    control/tcp_server.h \
    control/tirpc_loader.h \
    control/web_server.h \
    control/uart_server.h \
    #windows/digitalcardwidget.h \
    #windows/digitalpage.h \
 \    #windows/mainwindow.h
    widgets/batterycard.h \
    widgets/chstatusview.h \
    widgets/devicesetting.h \
    widgets/digitalcard.h \
    widgets/mainwindow.h \
    widgets/numberkeypad.h \
    widgets/remoteoverlay.h \
    widgets/test.h \
    widgets/versionview.h


SOURCES += \
    auxiliary/battery_model.cpp \
    auxiliary/scpi_handle.cpp \
    auxiliary/simple_logger.cpp \
    auxiliary/config_manager.cpp \
    channel/can_channel.cpp \
    channel/uart_channel.cpp \
    control/can_server.cpp \
    control/tcp_server.cpp \
    control/tirpc_loader.cpp \
    control/web_server.cpp \
    control/uart_server.cpp \
    main.cpp \
    #windows/digitalcardwidget.cpp \
    #windows/digitalpage.cpp \
 \    #windows/mainwindow.cpp
    widgets/batterycard.cpp \
    widgets/chstatusview.cpp \
    widgets/devicesetting.cpp \
    widgets/digitalcard.cpp \
    widgets/mainwindow.cpp \
    widgets/numberkeypad.cpp \
    widgets/remoteoverlay.cpp \
    widgets/test.cpp \
    widgets/versionview.cpp


RESOURCES += qml.qrc

DISTFILES += \
    auxiliary/instrument_config.ini

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /root/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/widgets

auxiliary_files.files = auxiliary/instrument_config.ini
auxiliary_files.path = /root/$${TARGET}/
INSTALLS += auxiliary_files

FORMS += \
    widgets/batterycard.ui \
    widgets/chstatusview.ui \
    widgets/devicesetting.ui \
    widgets/digitalcard.ui \
    widgets/mainwindow.ui \
    widgets/numberkeypad.ui \
    widgets/remoteoverlay.ui \
    widgets/test.ui \
    widgets/versionview.ui
