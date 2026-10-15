#include "global_static.h"
#include <QNetworkInterface>
#include <QSettings>
#include <QtEndian>
#include <QFile>

/* widgets */
Q_LOGGING_CATEGORY(aMain,"aMain:")
Q_LOGGING_CATEGORY(aPTest,"aPTest:")
Q_LOGGING_CATEGORY(aPMain,"aPMain:")
/* interaction */
Q_LOGGING_CATEGORY(inDeSeP,"inDeSeP:")
Q_LOGGING_CATEGORY(inChSeP,"inChSeP:")
Q_LOGGING_CATEGORY(inChPaG,"inChPaG:")
/* component */
Q_LOGGING_CATEGORY(coReOvL,"coReOvL:")
Q_LOGGING_CATEGORY(coNuKeP,"coNuKeP:")
Q_LOGGING_CATEGORY(coChCaD,"coChCaD:")

/* control */
Q_LOGGING_CATEGORY(canSer,"canSer:")
Q_LOGGING_CATEGORY(r32Ser,"r32Ser:")
Q_LOGGING_CATEGORY(vxiSer,"vxiSer:")
Q_LOGGING_CATEGORY(webSer,"webSer:")
/* channel */
Q_LOGGING_CATEGORY(canCh,"canCh:")
Q_LOGGING_CATEGORY(scpi,"scpi:")
Q_LOGGING_CATEGORY(uartCh,"uartCh:")
/* auxiliary */
Q_LOGGING_CATEGORY(battMod,"battMod:")
Q_LOGGING_CATEGORY(signBus,"signBus:")
Q_LOGGING_CATEGORY(globalSt,"globalSt:")
Q_LOGGING_CATEGORY(tirpLoad,"tirpLoad:")

std::unique_ptr<QSettings> ConfigManager::s_settings;
std::atomic<int> ConfigManager::control_Mode{InterSt::LOCAL};

namespace LogLevel {
    constexpr const int Release   = 0x00;
    constexpr const int Custom    = 0x01;
    constexpr const int Normal    = 0x02;
    constexpr const int Debug     = 0x03;
}
int ConfigManager::log_Level{LogLevel::Normal};
quint32 ConfigManager::s_canBaudIndex{0};
quint32 ConfigManager::s_rs232BaudIndex{0};
std::atomic<quint32> ConfigManager::s_CANid{0};

bool ConfigManager::init(const QString &configDir)
{
    QString fullPath = configDir + "/instrument_config.ini";

    if (QFile::exists(fullPath) && !s_settings && getNetworkConfig()){
        s_settings = std::make_unique<QSettings>(fullPath, QSettings::IniFormat);

        s_CANid = s_settings->value(ConfAddress::CanId).toUInt();
        s_canBaudIndex = s_settings->value(ConfAddress::CanBaud).toUInt();
        if (s_canBaudIndex > 8) {s_canBaudIndex = 0;}
        s_rs232BaudIndex = s_settings->value(ConfAddress::R23Baud).toUInt();
        if (s_rs232BaudIndex > 7) {s_rs232BaudIndex = 0;}

        log_Level = s_settings->value(ConfAddress::LogType).toUInt();// default-0
        if (log_Level != LogLevel::Normal){
            QString rules;
            switch (log_Level) {
                case LogLevel::Debug:rules = "*.debug=true\n*.info=true\n*.warning=true";break;
                case LogLevel::Release:rules = "*.debug=false\n*.info=false\n*.warning=false";break;
                case LogLevel::Custom:rules = "*.debug=false\n*.info=false\n*.warning=false\n"
                     "SCPI:.debug=true\n";break;
            }
            QLoggingCategory::setFilterRules(rules);
        }

        return true;
    }

    qCWarning(globalSt)<<"[init]:read config file fail!";
    return false;
}
bool ConfigManager::saveNewValue(const QString &key,const QVariant &val)
{
    if (s_settings) {
        s_settings->setValue(key, val);
        // s_settings->sync();
        return true;
    }

    qCWarning(globalSt)<<"[saveNewValue]:settings not exist!";
    return false;
}
bool ConfigManager::getNetworkConfig()
{
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : qAsConst(interfaces)) {
        if (iface.name() == "eth0") {
            MAC = iface.hardwareAddress();
            QFile file("/etc/network/interfaces");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QString line = in.readLine(); // get frist rows of headline

                isDHCP = false;
                while (!in.atEnd()) {
                    line = in.readLine().trimmed(); // Remove first and end spaces
                    if (line.startsWith("iface eth0") && line.contains("dhcp")) {
                        isDHCP = true;
                        break; // if static, Avoid printing
                    }
                }

                IP = SM = Gate = "---.---.---.---";
                const auto entries = iface.addressEntries();
                for (const QNetworkAddressEntry& entry : qAsConst(entries)) {
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                        SM = entry.netmask().toString();
                        IP = entry.ip().toString();
                        break;
                    }
                }

                QFile gatefile("/proc/net/route");
                if (gatefile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                   QTextStream gatein(&gatefile);
                   QString gateline = gatein.readLine(); // get frist rows of headline

                   while (!gatein.atEnd()) {
                       gateline = gatein.readLine();
                       QStringList fields = gateline.trimmed().split('\t', Qt::SkipEmptyParts);
                       if (fields.size() >= 3 && fields[0] == "eth0" && fields[1] == "00000000") {
                           bool ok; quint32 gw = fields[2].toUInt(&ok, 16);
                           if (ok) {
                               Gate = QHostAddress(qFromBigEndian(gw)).toString();
                               break;
                           }
                       }
                   }

                   return true;
                }

                qCWarning(globalSt)<<"[getNetworkConfig]:/proc/net/route opening failed!";
                return false;
            }

            qCWarning(globalSt)<<"[getNetworkConfig]:/etc/network/interfaces opening failed!";
            return false;
        }
    }

    qCWarning(globalSt)<<"[getNetworkConfig]:not find target iface!";
    return false;
}

QString ConfigManager::Gate{""};
QString ConfigManager::MAC{""};
QString ConfigManager::SM{""};
QString ConfigManager::IP{""};
bool ConfigManager::isDHCP{false};
