#include "global_static.h"
#include "signal_bus.h"
// --- QT package ---
#include <QNetworkInterface>
#include <QSettings>
#include <QtEndian>
#include <QFile>

/* widgets */
Q_LOGGING_CATEGORY(aMain,"[aMain]")
Q_LOGGING_CATEGORY(aPMain,"[aPMain]")
/* interaction */
Q_LOGGING_CATEGORY(inDeSeP,"[inDeSeP]")
Q_LOGGING_CATEGORY(inChSeP,"[inChSeP]")
Q_LOGGING_CATEGORY(inChPaG,"[inChPaG]")
/* component */
Q_LOGGING_CATEGORY(coReOvL,"[coReOvL]")
Q_LOGGING_CATEGORY(coNuKeP,"[coNuKeP]")
Q_LOGGING_CATEGORY(coChCaD,"[coChCaD]")
/* control */
Q_LOGGING_CATEGORY(canSer,"[canSer]")
Q_LOGGING_CATEGORY(r32Ser,"[r32Ser]")
Q_LOGGING_CATEGORY(vxiSer,"[vxiSer]")
Q_LOGGING_CATEGORY(webSer,"[webSer]")
/* channel */
Q_LOGGING_CATEGORY(canCh,"[canCh]")
Q_LOGGING_CATEGORY(scpi,"[scpi]")
Q_LOGGING_CATEGORY(uartCh,"[uartCh]")
/* auxiliary */
Q_LOGGING_CATEGORY(battMod,"[battMod]")
Q_LOGGING_CATEGORY(signBus,"[signBus]")
Q_LOGGING_CATEGORY(globalSt,"[globalSt]")

QStringList ConfigManager::customList= {
    "[scpi]:",
};
void ConfigManager::setLogRules(int level)
{
    QStringList ruleLines;

    switch (level){
        case LogLevel::Release:
            ruleLines << "*.debug=false";
            ruleLines << "*.info=false";
            ruleLines << "*.warning=false";
            break;
        case LogLevel::Debug:
            ruleLines << "*.debug=true";
            ruleLines << "*.info=true";
            ruleLines << "*.warning=true";
            break;
        case LogLevel::Custom:
            ruleLines << "*.debug=false";
            ruleLines << "*.info=false";
            ruleLines << "*.warning=false";

            for(const auto& cat :qAsConst(customList)){
                ruleLines << QString("%1.debug=true").arg(cat);
                ruleLines << QString("%1.info=true").arg(cat);
                ruleLines << QString("%1.warning=true").arg(cat);
            }
            break;
        case LogLevel::Normal: default: break; // rules="" -> default
    }

    QString rules = ruleLines.join('\n');
    QLoggingCategory::setFilterRules(rules);
}

std::atomic<int> ConfigManager::control_Mode{InterSt::LOCAL};
bool ConfigManager::isInteRemote(int Inte)
{
    if(ConfigManager::control_Mode.load() == Inte){
        return true;
    }else if (ConfigManager::control_Mode.load() == InterSt::LOCAL){
        emit ChannelBus::instance().switchRemote(Inte);
        return true;
    }

    qCDebug(globalSt)<<"(isVxiRemote): Device on Other Remote_mode";
    return false;
}

std::atomic<quint32> ConfigManager::s_CANid{0};
quint32 ConfigManager::s_rs232BaudIndex{0};
quint32 ConfigManager::s_canBaudIndex{0};

std::unique_ptr<QSettings> ConfigManager::s_settings;
bool ConfigManager::init(const QString &configDir)
{
    QString fullPath = configDir + "/instrument_config.ini";

    if (QFile::exists(fullPath)){
        if (getNetworkConfig()){
            if (!s_settings){
                s_settings = std::make_unique<QSettings>(fullPath, QSettings::IniFormat);

                s_CANid = s_settings->value(ConfAddress::CanId).toUInt();
                s_canBaudIndex = s_settings->value(ConfAddress::CanBaud).toUInt();
                if (s_canBaudIndex > 8) {s_canBaudIndex = 0;}
                s_rs232BaudIndex = s_settings->value(ConfAddress::R23Baud).toUInt();
                if (s_rs232BaudIndex > 7) {s_rs232BaudIndex = 0;}

                return true;
            }

            qCCritical(globalSt)<<"(init)already inital!!!";
            return false;
        }

        qCCritical(globalSt)<<"(init)getNetworkConfig failed!!!";
        return false;
    }

    qCCritical(globalSt)<<"(init)read config-file fail!!!";
    return false;
}
bool ConfigManager::saveNewValue(const QString &key,const QVariant &val)
{
    if (s_settings) {
        s_settings->setValue(key, val);
        return true;
    }

    qCWarning(globalSt)<<"(saveNewValue)settings not exist!";
    return false;
}
bool ConfigManager::getNetworkConfig()
{
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : qAsConst(interfaces)) {
        if (iface.name() == "eth0") {
            MAC = iface.hardwareAddress();

            IP = SM = "---.---.---.---";
            const auto entries = iface.addressEntries();
            for (const QNetworkAddressEntry& entry : qAsConst(entries)) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    SM = entry.netmask().toString();
                    IP = entry.ip().toString();
                    break;
                }
            }

            QFile file("/etc/network/interfaces");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                isDHCP = false;
                QTextStream in(&file);
                QString line = in.readLine(); // get frist rows of headline

                while (!in.atEnd()) {
                    line = in.readLine().trimmed(); // Remove first and end spaces
                    if (line.startsWith("iface eth0") && line.contains("dhcp")) {
                        isDHCP = true;
                        break; // if static, Avoid printing
                    }
                }

                QFile gatefile("/proc/net/route");
                if (gatefile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                   Gate = "---.---.---.---";
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

                qCWarning(globalSt)<<"(getNetworkConfig)/proc/net/route opening failed!";
                return false;
            }

            qCWarning(globalSt)<<"(getNetworkConfig)/etc/network/interfaces opening failed!";
            return false;
        }
    }

    qCWarning(globalSt)<<"(getNetworkConfig)not find target iface!";
    return false;
}

QString ConfigManager::Gate{""};
QString ConfigManager::MAC{""};
QString ConfigManager::SM{""};
QString ConfigManager::IP{""};
bool ConfigManager::isDHCP{false};
