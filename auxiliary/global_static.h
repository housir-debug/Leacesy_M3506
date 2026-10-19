#pragma once
#include <QLoggingCategory>

/* widgets */
Q_DECLARE_LOGGING_CATEGORY(aMain)
Q_DECLARE_LOGGING_CATEGORY(aPTest)
Q_DECLARE_LOGGING_CATEGORY(aPMain)
/* interaction */
Q_DECLARE_LOGGING_CATEGORY(inDeSeP)
Q_DECLARE_LOGGING_CATEGORY(inChSeP)
Q_DECLARE_LOGGING_CATEGORY(inChPaG)
/* component */
Q_DECLARE_LOGGING_CATEGORY(coReOvL)
Q_DECLARE_LOGGING_CATEGORY(coNuKeP)
Q_DECLARE_LOGGING_CATEGORY(coChCaD)

/* control */
Q_DECLARE_LOGGING_CATEGORY(canSer)
Q_DECLARE_LOGGING_CATEGORY(r32Ser)
Q_DECLARE_LOGGING_CATEGORY(vxiSer)
Q_DECLARE_LOGGING_CATEGORY(webSer)
/* channel */
Q_DECLARE_LOGGING_CATEGORY(canCh)
Q_DECLARE_LOGGING_CATEGORY(scpi)
Q_DECLARE_LOGGING_CATEGORY(uartCh)
/* auxiliary */
Q_DECLARE_LOGGING_CATEGORY(battMod)
Q_DECLARE_LOGGING_CATEGORY(signBus)
Q_DECLARE_LOGGING_CATEGORY(globalSt)
Q_DECLARE_LOGGING_CATEGORY(tirpLoad)

namespace Device {
    constexpr const char* SoftVer           = "1.0.0";
    constexpr const char* HardVer           = "1.0.0";
    constexpr const char* Identifying       = "SN-12306";
    constexpr const char* ModelNumber       = "66004";
    constexpr const char* Manufacturer      = "Leacesy";
}
namespace Module {
    constexpr const bool enable_CanCh        = false;
    constexpr const bool enable_UartCh       = false;
    /* control */
    constexpr const bool enable_UartSer      = false;
    constexpr const bool enable_CanSer       = true;
    constexpr const bool enable_VxiSer       = true;
    constexpr const bool enable_WebSer       = true;
    constexpr const bool enable_DisPlay      = true;
}

class QSettings;
class ConfigManager
{
public:
    static int log_Level;
    static quint32 s_canBaudIndex;
    static quint32 s_rs232BaudIndex;
    static std::atomic<quint32> s_CANid;
    static std::atomic<int> control_Mode;
    static bool init(const QString &configDir);
    static bool saveNewValue(const QString &key,const QVariant &val);
    static bool getNetworkConfig();
    static QString Gate;
    static QString MAC;
    static QString SM;
    static QString IP;
    static bool isDHCP;

private:
    ConfigManager() = delete;
    ~ConfigManager() = delete;
    Q_DISABLE_COPY(ConfigManager)
    static std::unique_ptr<QSettings> s_settings;
};

namespace InterSt {
    constexpr int LOCAL      = 0x00;
    constexpr int CAN        = 0x01;
    constexpr int RS232      = 0x02;
    constexpr int LAN_VXI    = 0x03;
    constexpr int LAN_WEB    = 0x04;
}

namespace ConfAddress {
    constexpr const char*  CanId      = "Device/CANID";
    constexpr const char*  CanBaud    = "Device/CANBaud";
    constexpr const char*  R23Baud    = "Device/RS232Baud";
    constexpr const char*  LogType    = "Device/LogLevel";
}
