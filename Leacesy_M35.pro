QT += core widgets serialport websockets

CONFIG += c++14

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# You can make your code fail to compile if it uses deprecated APIs.
# disables all the APIs deprecated before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

INCLUDEPATH += $$PWD/thirdparty/scpi/inc
LIBS += -L$$PWD/thirdparty/scpi/lib -lscpi -lm

HEADERS += \
    auxiliary/battery_model.h \
    auxiliary/scpi_handle.h \
    auxiliary/config_manager.h \
    auxiliary/uartProtocol.h \
    auxiliary/vxinamespace.h \
    channel/can_channel.h \
    channel/uart_channel.h \
    control/can_server.h \
    control/tirpc_loader.h \
    control/vxi_server.h \
    control/web_server.h \
    control/uart_server.h \
    widgets/appmain.h \
    widgets/component/channelcard.h \
    widgets/component/chstatusview.h \
    widgets/interaction/channelpage.h \
    widgets/interaction/channelsetting.h \
    widgets/interaction/devicesetpage.h \
    widgets/component/numberkeypad.h \
    widgets/component/remoteoverlay.h \
    widgets/test.h

SOURCES += \
    auxiliary/battery_model.cpp \
    auxiliary/scpi_handle.cpp \
    auxiliary/config_manager.cpp \
    channel/can_channel.cpp \
    channel/uart_channel.cpp \
    control/can_server.cpp \
    control/tirpc_loader.cpp \
    control/vxi_server.cpp \
    control/web_server.cpp \
    control/uart_server.cpp \
    main.cpp \
    widgets/appmain.cpp \
    widgets/component/channelcard.cpp \
    widgets/component/chstatusview.cpp \
    widgets/interaction/channelpage.cpp \
    widgets/interaction/channelsetting.cpp \
    widgets/interaction/devicesetpage.cpp \
    widgets/component/numberkeypad.cpp \
    widgets/component/remoteoverlay.cpp \
    widgets/test.cpp

FORMS += \
    widgets/appmain.ui \
    widgets/component/channelcard.ui \
    widgets/component/chstatusview.ui \
    widgets/interaction/channelpage.ui \
    widgets/interaction/channelsetting.ui \
    widgets/interaction/devicesetpage.ui \
    widgets/component/numberkeypad.ui \
    widgets/component/remoteoverlay.ui \
    widgets/test.ui

RESOURCES += resources/master.qrc

DISTFILES += auxiliary/instrument_config.ini

auxiliary_files.files = auxiliary/instrument_config.ini
auxiliary_files.path = /root/$${TARGET}/
INSTALLS += auxiliary_files

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /root/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
