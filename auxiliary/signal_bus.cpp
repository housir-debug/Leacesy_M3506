#include "signal_bus.h"

ChannelBus& ChannelBus::instance()
{
    static ChannelBus inst;
    return inst;
}

ChannelBus::ChannelBus(QObject *parent) : QObject(parent)
{
    std::vector<UartConfig> configs = {
        //{"/dev/ttyS3",    QSerialPort::Baud38400, 0x01}, // debug-Uart
        /*{"/dev/ttyS4",    QSerialPort::Baud38400, 0x01},
        {"/dev/ttyS5",    QSerialPort::Baud38400, 0x02},
        {"/dev/ttyS7",    QSerialPort::Baud38400, 0x03},
        {"/dev/ttyS8",    QSerialPort::Baud38400, 0x04},
        {"/dev/ttyS9",    QSerialPort::Baud38400, 0x05},
        {"/dev/ttyWCH0",  QSerialPort::Baud38400, 0x06},
        {"/dev/ttyWCH1",  QSerialPort::Baud38400, 0x07},
        {"/dev/ttyWCH2",  QSerialPort::Baud38400, 0x08},
        {"/dev/ttyWCH3",  QSerialPort::Baud38400, 0x09},
        {"/dev/ttyWCH4",  QSerialPort::Baud38400, 0x0a},
        {"/dev/ttyWCH5",  QSerialPort::Baud38400, 0x0b},
        {"/dev/ttyWCH6",  QSerialPort::Baud38400, 0x0c},
        {"/dev/ttyWCH7",  QSerialPort::Baud38400, 0x0d},
        {"/dev/ttyWCH8",  QSerialPort::Baud38400, 0x0e},
        {"/dev/ttyWCH9",  QSerialPort::Baud38400, 0x0f},
        {"/dev/ttyWCH10", QSerialPort::Baud38400, 0x10},
        {"/dev/ttyWCH11", QSerialPort::Baud38400, 0x11},
        {"/dev/ttyWCH12", QSerialPort::Baud38400, 0x12},
        {"/dev/ttyWCH13", QSerialPort::Baud38400, 0x13},
        {"/dev/ttyWCH14", QSerialPort::Baud38400, 0x14},
        {"/dev/ttyWCH15", QSerialPort::Baud38400, 0x15},
        {"/dev/ttyWCH16", QSerialPort::Baud38400, 0x16},
        {"/dev/ttyWCH17", QSerialPort::Baud38400, 0x17},
        {"/dev/ttyWCH18", QSerialPort::Baud38400, 0x18},
        {"/dev/ttyWCH19", QSerialPort::Baud38400, 0x19},
        {"/dev/ttyWCH20", QSerialPort::Baud38400, 0x1a},
        {"/dev/ttyWCH21", QSerialPort::Baud38400, 0x1b},
        {"/dev/ttyWCH22", QSerialPort::Baud38400, 0x1c},
        {"/dev/ttyWCH23", QSerialPort::Baud38400, 0x1d},
        {"/dev/ttyWCH24", QSerialPort::Baud38400, 0x1e},
        {"/dev/ttyWCH25", QSerialPort::Baud38400, 0x1f},
        {"/dev/ttyWCH26", QSerialPort::Baud38400, 0x20},
        {"/dev/ttyWCH27", QSerialPort::Baud38400, 0x21},   // 33*/
        {"/dev/ttyS1",    QSerialPort::Baud38400, 0x01},   // test
    };
}

void ChannelBus::to_ChannelReset(int channel)
{
    switch(channel) {
        case 0:
            #define CHANNEL(n) emit to_UartChannel##n##Reset();
            CHANNEL_COUNT
            #undef CHANNEL
            return;
        #define CHANNEL(n) case n: return emit to_UartChannel##n##Reset();
        CHANNEL_COUNT
        #undef CHANNEL
        default:return;
    }
}

void ChannelBus::to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param,bool isScpi)
{
    switch(channel) {
        case 0:
            #define CHANNEL(n) emit to_UartChannel##n(cmd, func, param,isScpi);
            CHANNEL_COUNT
            #undef CHANNEL
            return;
        #define CHANNEL(n) case n: return emit to_UartChannel##n(cmd, func, param,isScpi);
        CHANNEL_COUNT
        #undef CHANNEL
        default:return;
    }
}
