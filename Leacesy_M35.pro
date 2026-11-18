QT += core widgets serialport websockets

CONFIG += c++14

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# You can make your code fail to compile if it uses deprecated APIs.
# disables all the APIs deprecated before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

INCLUDEPATH += \
    $$PWD/thirdparty/scpi/inc \
    $$PWD/thirdparty/VXI-11 \
    /home/ryan/Downloads/M3506_sdk/buildroot/output/rockchip_M3506/host/arm-buildroot-linux-gnueabihf/sysroot/usr/include/tirpc

LIBS += -L$$PWD/thirdparty/scpi/lib \
    -L/home/ryan/Downloads/M3506_sdk/buildroot/output/rockchip_M3506/host/arm-buildroot-linux-gnueabihf/sysroot/usr/lib \
    -L/home/ryan/Downloads/M3506_sdk/buildroot/output/rockchip_M3506/host/arm-buildroot-linux-gnueabihf/sysroot/lib \
    -lscpi -lm -ltirpc

QMAKE_LFLAGS += -Wl,--sysroot=/home/ryan/Downloads/M3506_sdk/buildroot/output/rockchip_M3506/host/arm-buildroot-linux-gnueabihf/sysroot \
    -B/home/ryan/Downloads/M3506_sdk/buildroot/output/rockchip_M3506/host/arm-buildroot-linux-gnueabihf/sysroot/usr/lib

HEADERS += \
    auxiliary/battery_model.h \
    auxiliary/global_static.h \
    auxiliary/signal_bus.h \
    channel/can_channel.h \
    channel/scpi_handle.h \
    channel/uart_channel.h \
    channel/uart_protocol.h \
    control/can_server.h \
    control/r32_server.h \
    control/vxi11_server.h \
    control/vxi_protocol.h \
    control/web_server.h \
    widgets/component/channelcard.h \
    widgets/component/numberkeypad.h \
    widgets/component/remoteoverlay.h \
    widgets/interaction/channelpage.h \
    widgets/interaction/channelsetpage.h \
    widgets/interaction/devicesetpage.h \
    widgets/appmain.h

SOURCES += \
    auxiliary/battery_model.cpp \
    auxiliary/global_static.cpp \
    auxiliary/signal_bus.cpp \
    channel/can_channel.cpp \
    channel/scpi_handle.cpp \
    channel/uart_channel.cpp \
    control/can_protocol.cpp \
    control/can_server.cpp \
    control/r32_server.cpp \
    control/vxi11_server.cpp \
    control/web_server.cpp \
    widgets/component/channelcard.cpp \
    widgets/component/numberkeypad.cpp \
    widgets/component/remoteoverlay.cpp \
    widgets/interaction/channelpage.cpp \
    widgets/interaction/channelsetpage.cpp \
    widgets/interaction/devicesetpage.cpp \
    widgets/appmain.cpp \
    thirdparty/VXI-11/vxi11core_svc.c \
    thirdparty/VXI-11/vxi11core_xdr.c \
    main.cpp

FORMS += \
    widgets/component/channelcard.ui \
    widgets/component/numberkeypad.ui \
    widgets/component/remoteoverlay.ui \
    widgets/interaction/channelpage.ui \
    widgets/interaction/channelsetpage.ui \
    widgets/interaction/devicesetpage.ui \
    widgets/appmain.ui

RESOURCES += resources/master.qrc

DISTFILES += auxiliary/instrument_config.ini

auxiliary_files.files = auxiliary/instrument_config.ini
auxiliary_files.path = /root/$${TARGET}/
INSTALLS += auxiliary_files

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /root/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
