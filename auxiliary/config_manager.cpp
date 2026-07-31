#include "config_manager.h"
#include <QNetworkInterface>
#include <QtCore>

Q_LOGGING_CATEGORY(config, "CONFIG:")

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

// log config
QString ConfigManager::s_loglevel = "info";
bool ConfigManager::s_enablelogfile = false;
// channel switch
bool ConfigManager::s_enableCanMess    = false;
bool ConfigManager::s_enableUartMess   = false;
// control switch
bool ConfigManager::s_enableUARTServer = false;
bool ConfigManager::s_enableCANServer  = true;
bool ConfigManager::s_enableLANServer  = true;
bool ConfigManager::s_enableWEBServer  = true;
bool ConfigManager::s_enableDisplay    = true;

std::atomic<int> ConfigManager::s_remoteSt{0};
QSettings* ConfigManager::s_settings = nullptr;

// global variable - System reading
QString ConfigManager::s_IP = "";
QString ConfigManager::s_SM = "";
QString ConfigManager::s_MAC = "";
QString ConfigManager::s_Gateway = "";
bool ConfigManager::s_isDHCP = false;

// global variable - Internal fixation
QString ConfigManager::s_firmwareVersion = "1.0.0";
QString ConfigManager::s_hardwareVersion = "1.0.0";
QString ConfigManager::s_manufacturer = "Leacesy";

// global variable - config file
QString ConfigManager::s_serialNumber = "SN-66004";
QString ConfigManager::s_model = "66004";
quint8 ConfigManager::s_GPIBid = 0;
quint8 ConfigManager::s_CANid = 0;

bool ConfigManager::init(const QString &configDir)
{
    if (getNetworkConfig()){
        QString fullPath = configDir + "/instrument_config.ini";
        if (QFile::exists(fullPath) && !s_settings) {
            s_settings = new QSettings(fullPath, QSettings::IniFormat);

            s_serialNumber = s_settings->value("Device/SerialNumber").toString();
            s_model = s_settings->value("Device/Model").toString();
            s_GPIBid = s_settings->value("Device/GPIBID").toUInt();
            s_CANid = s_settings->value("Device/CANID").toUInt();

            return true;
        }

        qCWarning(config) << "[init]:Cannot open config file for writing";
    }

    // getNetworkConfig have print information
    return false;
}

bool ConfigManager::getNetworkConfig() {
    static std::once_flag initFlag;

    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : qAsConst(interfaces)) {
        if (iface.name() == "eth0") {
            std::call_once(initFlag, [&]() {
                s_isDHCP = false;
                s_MAC = iface.hardwareAddress();
                QFile file("/etc/network/interfaces");
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    QString line = in.readLine(); // get frist rows of headline
                    while (!in.atEnd()) {
                        line = in.readLine().trimmed(); // Remove first and end spaces
                        if (line.startsWith("iface eth0") && line.contains("dhcp")) {
                            s_isDHCP = true;
                            break;
                        }
                    }

                    return;
                }

                qCWarning(config) << "[getNetworkConfig]:/etc/network/interfaces opening failed!";
                return;
            });

            s_IP = s_SM = s_Gateway = "---.---.---.---";
            const auto entries = iface.addressEntries();
            for (const QNetworkAddressEntry& entry : qAsConst(entries)) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    s_SM = entry.netmask().toString();
                    s_IP = entry.ip().toString();
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
                           s_Gateway = QHostAddress(qFromBigEndian(gw)).toString();
                           break;
                       }
                   }
               }

               return true;
            }

            qCWarning(config) << "[getNetworkConfig]:/proc/net/route opening failed!";
            return false;
        }
    }

    qCWarning(config) << "[getNetworkConfig]:not find target iface!";
    return false;
}

bool ConfigManager::setinterfaces(bool isstatic,const QString& ip, const QString& netmask,const QString& gateway){
    QFile file("/etc/network/interfaces");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // WriteOnly -> Clear the original content
        QTextStream out(&file);
        QString configContent;

        if(isstatic){
            configContent =
                "# interface file auto-generated by buildroot\n"
                "auto lo\n"
                "iface lo inet loopback\n\n"
                "auto eth0\n"
                "iface eth0 inet static\n"
                "    address " + ip + "\n"
                "    netmask " + netmask + "\n"
                "    gateway " + gateway;

            out << configContent;
            file.close();

            QString restartCmd = "/etc/init.d/S40network restart";
            if (system(restartCmd.toStdString().c_str()) == 0) {
                getNetworkConfig();
                return true;
            }

            qCWarning(config) << "[refresh_interfaces]:/etc/init.d/S40network restart failed!";
            return false;
        }else{
            configContent =
                "# interface file auto-generated by buildroot\n"
                "auto lo\n"
                "iface lo inet loopback\n\n"
                "auto eth0\n"
                "iface eth0 inet dhcp";

            out << configContent;
            file.close();

            QProcess *process = new QProcess(); // not parent class ,so I need delete it
            QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),[process](int exitCode, QProcess::ExitStatus) {
                 if (exitCode != 0) {qCWarning(config) << "[refresh_interfaces]:S40network restart failed!";}
                 process->deleteLater(); // After achieving success, the IP address have not yet been updated.
             });

            process->start("/etc/init.d/S40network", QStringList() << "restart");
            return true;
        }
    }

    qCWarning(config) << "[setinterfaces]:Cannot open interfaces file for writing";
    return false;
}

bool ConfigManager::setConfigValue(const QString &key, const QVariant &value)
{
    if (s_settings) {
        // Automatically written at the end
        s_settings->setValue(key, value);
        // s_settings->sync();
        return true;
    }

    qCWarning(config) << "[setConfigValue]:not setting exist!";
    return false;
}
