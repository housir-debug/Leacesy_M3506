#pragma once
#include <QLoggingCategory>
#include <QSerialPort>
#include <QSettings>

Q_DECLARE_LOGGING_CATEGORY(config)

/* support uart channel number and config */
#define CHANNEL_COUNT \
    CHANNEL(1)  CHANNEL(2)  CHANNEL(3)  CHANNEL(4)  CHANNEL(5)  CHANNEL(6)  \
    CHANNEL(7)  CHANNEL(8)  CHANNEL(9)  CHANNEL(10) CHANNEL(11) CHANNEL(12) \
    CHANNEL(13) CHANNEL(14) CHANNEL(15) CHANNEL(16) CHANNEL(17) CHANNEL(18) \
    CHANNEL(19) CHANNEL(20) CHANNEL(21) CHANNEL(22) CHANNEL(23) CHANNEL(24) \
    CHANNEL(25) CHANNEL(26) CHANNEL(27) CHANNEL(28) CHANNEL(29) CHANNEL(30) \
    CHANNEL(31) CHANNEL(32) CHANNEL(33) CHANNEL(34) CHANNEL(35) CHANNEL(36)

struct UartConfig {
    QString port;
    QSerialPort::BaudRate baudRate;
    quint8 channel;
};
extern std::vector<UartConfig> configs;
/* ************************************** */

class ConfigManager {
public:
    static QString s_loglevel;
    static bool s_enablelogfile;

    static bool s_enableCanMess;
    static bool s_enableUartMess;

    static bool s_enableUARTServer;
    static bool s_enableCANServer;
    static bool s_enableLANServer;
    static bool s_enableWEBServer;
    static bool s_enableDisplay;

    static QString s_firmwareVersion;
    static QString s_hardwareVersion;
    static QString s_manufacturer;

    static std::atomic<int> s_remoteSt;
    static bool init(const QString &configDir);
    static bool setConfigValue(const QString &key, const QVariant &value);

    static QString s_serialNumber;
    static QString s_model;

    static std::atomic<quint8> s_CANid;
    static int s_rs232BaudRate;
    static int s_canBaudRate;

    static bool getNetworkConfig();
    static QString s_Gateway;
    static bool s_isDHCP;
    static QString s_MAC;
    static QString s_SM;
    static QString s_IP;

private:
    ConfigManager() = delete;       // Prohibition of construction
    ~ConfigManager() = delete;      // Prohibit destruction
    Q_DISABLE_COPY(ConfigManager)   // Prohibition of copying
    static QSettings* s_settings;
};
