#pragma once
#include <QObject>

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
    /* control,UI -> channel */
    #define CHANNEL(n) \
        void to_UartChannel##n(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi); \
        void to_UartChannel##n##Reset();
    CHANNEL_COUNT
    #undef CHANNEL
    /* channel -> control */
    void to_CanServer(quint8 ch,quint16 uart,const QByteArray &param);
    /* channel -> UI */
    void CH_svChanged(int ch,const QString &ver);
    void CH_hvChanged(int ch,const QString &ver);
    void CH_VoltageChanged(int ch,float voltage);
    void CH_CurrentAndUnitChanged(int ch,float current);
    void CH_RangeChanged(int ch,quint8 range);
    void CH_StatusChanged(int ch,quint16 status);
    void CH_cvChanged(int ch,float cv);
    void CH_ccChanged(int ch,float cc);
    void CH_ovpChanged(int ch,float ovp);
    void CH_isOutputChanged(int ch,bool status);
    void CH_impChanged(int ch,float imp);

    /* UI -> control */
    void set_canbaud();
    void set_RS232Baud();
    void set_network(bool isstatic);
    /* control -> UI */
    void interAcUpdate();
    void interModelUpdate();
    void switchRemote(int reface);

public:
    struct UartConfig {
        QString port;
        int baudRate;
        quint8 channel;
    };
    static const std::vector<UartConfig> configs;

    static ChannelBus& instance();
    void to_ChannelReset(int channel);
    void to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param, bool isScpi);

private:
    explicit ChannelBus(QObject *parent = nullptr);
    ~ChannelBus() = default;
    Q_DISABLE_COPY(ChannelBus)
};
