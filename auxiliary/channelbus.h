#pragma once
#include <QObject>
#include <QSerialPort>

#define CHANNEL_COUNT \
    CHANNEL(1)  CHANNEL(2)  CHANNEL(3)  CHANNEL(4)  CHANNEL(5)  CHANNEL(6)  \
    CHANNEL(7)  CHANNEL(8)  CHANNEL(9)  CHANNEL(10) CHANNEL(11) CHANNEL(12) \
    CHANNEL(13) CHANNEL(14) CHANNEL(15) CHANNEL(16) CHANNEL(17) CHANNEL(18) \
    CHANNEL(19) CHANNEL(20) CHANNEL(21) CHANNEL(22) CHANNEL(23) CHANNEL(24) \
    CHANNEL(25) CHANNEL(26) CHANNEL(27) CHANNEL(28) CHANNEL(29) CHANNEL(30) \
    CHANNEL(31) CHANNEL(32) CHANNEL(33) CHANNEL(34) CHANNEL(35) CHANNEL(36)

class ChannelBus : public QObject
{
    Q_OBJECT

signals:
    #define CHANNEL(n) void to_UartChannel##n(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
    CHANNEL_COUNT
    #undef CHANNEL

public:
    struct UartConfig {
        QString port;
        QSerialPort::BaudRate baudRate;
        quint8 channel;
    };
    std::vector<UartConfig> configs;

    static ChannelBus* instance();
    void to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param);

private:
    explicit ChannelBus(QObject *parent = nullptr);
    ~ChannelBus() = default;
    Q_DISABLE_COPY(ChannelBus)
};
