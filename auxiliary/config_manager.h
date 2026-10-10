#pragma once
#include <QLoggingCategory>
#include <QSettings>

Q_DECLARE_LOGGING_CATEGORY(config)


class ConfigManager
{
public:
    static QString s_loglevel;
    static std::atomic<int> s_remoteSt;

    static bool s_enableCanMess;
    static bool s_enableUartMess;

    static bool s_enableUARTServer;
    static bool s_enableCANServer;
    static bool s_enableLANServer;
    static bool s_enableDisplay;

    static bool init(const QString &configDir);
    static bool setConfigValue(const QString &key, const QVariant &value);

    static QString s_serialNumber;
    static QString s_model;

    static std::atomic<quint32> s_CANid;
    static quint32 s_rs232BaudRate;
    static quint32 s_canBaudRate;

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

namespace InterSt {
    constexpr int LOCAL      = 0x00;
    constexpr int CAN        = 0x01;
    constexpr int RS232      = 0x02;
    constexpr int LAN_VXI    = 0x03;
    constexpr int LAN_WEB    = 0x04;
}

namespace Device {
    constexpr const char* SoftVer           = "1.0.0";
    constexpr const char* HardVer           = "1.0.0";
    constexpr const char* Manufacturer      = "Leacesy";
}
