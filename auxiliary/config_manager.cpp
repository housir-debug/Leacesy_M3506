#include "config_manager.h"
#include <QNetworkInterface>
#include <QBigEndianStorageType>
#include <QFile>

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
bool ConfigManager::s_enablelogfile = false;
QString ConfigManager::s_loglevel = "release";

// channel switch
bool ConfigManager::s_enableUartMess   = true;
bool ConfigManager::s_enableCanMess    = true;
// control switch
bool ConfigManager::s_enableLANServer  = true;
bool ConfigManager::s_enableWEBServer  = false;
bool ConfigManager::s_enableCANServer  = true;
bool ConfigManager::s_enableUARTServer = true;
bool ConfigManager::s_enableDisplay    = true;

// global variable - Internal fixation
QString ConfigManager::s_firmwareVersion = "1.0.0";
QString ConfigManager::s_hardwareVersion = "1.0.0";
QString ConfigManager::s_manufacturer = "Leacesy";
// global variable - System reading
QString ConfigManager::s_IP = "";
QString ConfigManager::s_SM = "";
QString ConfigManager::s_MAC = "";
QString ConfigManager::s_Gateway = "";
bool ConfigManager::s_isDHCP = false;

// global variable - config reading
QString ConfigManager::s_serialNumber = "SN-12306";
QString ConfigManager::s_model = "66004";
QString ConfigManager::s_GPIBid = "0";
QString ConfigManager::s_CANid = "0";

std::atomic<quint8> ConfigManager::s_remoteSt{0};

QSettings* ConfigManager::s_settings = nullptr;

bool ConfigManager::init(const QString &configDir)
{
    if (getNetworkConfig()){
        QString fullPath = configDir + "/instrument_config.ini";

        if (QFile::exists(fullPath) && !s_settings) {
            s_settings = new QSettings(fullPath, QSettings::IniFormat);

            s_loglevel = s_settings->value("Logger/logLevel").toString();
            s_enablelogfile = s_settings->value("Logger/EnablelogFile").toBool();

            // channel switch
            s_enableUartMess = s_settings->value("Channel/EnableUartMess").toBool();
            s_enableCanMess = s_settings->value("Channel/EnableCanMess").toBool();
            // control switch
            s_enableLANServer = s_settings->value("Control/EnableLANServer").toBool();
            s_enableWEBServer = s_settings->value("Control/EnableWEBServer").toBool();
            s_enableCANServer = s_settings->value("Control/EnableCANServer").toBool();
            s_enableUARTServer = s_settings->value("Control/EnableUARTServer").toBool();
            s_enableDisplay = s_settings->value("Control/EnableDisplay").toBool();

            // global variable
            s_serialNumber = s_settings->value("Device/SerialNumber").toString();
            s_model = s_settings->value("Device/Model").toString();
            s_GPIBid = s_settings->value("Device/GPIBID").toString();
            s_CANid = s_settings->value("Device/CANID").toString();

            return true;
        }

        qCWarning(config) << "[init]:Cannot open config file for writing";
    }
    return false;
}

bool ConfigManager::setConfigValue(const QString &key, const QVariant &value)
{
    if (s_settings) {
        QVariant oldValue = s_settings->value(key);
        if (oldValue != value && !value.isNull()) {
            // Automatically written at the end
            s_settings->setValue(key, value);
            // s_settings->sync(); // Write immediately to the file
            return true;
        }

        qCWarning(config) << "[setConfigValue]:config new value invalue.";
    }
    return false;
}

bool ConfigManager::getNetworkConfig() {
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : qAsConst(interfaces)) {
        if (iface.name() == "eth0") {
            s_Gateway = getGateway("eth0");
            s_isDHCP = getnetworkmode("eth0");
            s_MAC = iface.hardwareAddress();
            const auto entries = iface.addressEntries();

            for (const QNetworkAddressEntry& entry : qAsConst(entries)) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    s_IP = entry.ip().toString();
                    s_SM = entry.netmask().toString();
                    return true;
                }
            }

            qCWarning(config) << "[getNetworkConfig]:reading IP&SM failed!";
        }
    }
    return false;
}

QString ConfigManager::getGateway(const QString& interfaceName) {
    QFile file("/proc/net/route");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine(); // get frist rows of headline

        while (!in.atEnd()) {
            line = in.readLine();
            QStringList parts = line.split('\t', Qt::SkipEmptyParts);

            if (parts.size() > 3) {
                QString iface = parts[0];
                QString destination = parts[1];

                // default route : destination - 00000000
                if (iface == interfaceName && destination == "00000000") {
                    bool ok;
                    QString gatewayHex = parts[2];
                    quint32 gw = gatewayHex.toUInt(&ok, 16);

                    if (ok) {
                        quint32 networkOrder = qFromBigEndian<quint32>(gw);
                        QHostAddress addr(networkOrder); // to Big-endian order
                        return addr.toString();
                    }
                }
            }
        }

        qCWarning(config) << "[getGateway]failed to read gateways configature!";
        return QString();
    }

    qCWarning(config) << "[getGateway]Cannot open /proc/net/route";
    return QString();
}

bool ConfigManager::getnetworkmode(const QString& interfaceName) {
    QFile file("/etc/network/interfaces");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine(); // get frist rows of headline

        while (!in.atEnd()) {
            line = in.readLine().trimmed();

            if (line.startsWith("iface " + interfaceName)) {
                if (line.contains("dhcp", Qt::CaseInsensitive)) {
                    return true;
                }

                if (line.contains("static", Qt::CaseInsensitive)) {
                    return false;
                }
            }
        }
    }

    qCWarning(config) << "[getnetworkmode] Cannot open /etc/network/interfaces";
    return false; // static
}

bool ConfigManager::setinterfaces(bool isstatic,const QString& ip, const QString& netmask,const QString& gateway){
    QFile file("/etc/network/interfaces");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
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
                "    gateway " + gateway + "\n\n"
                "auto eth1\n"
                "iface eth1 inet static\n"
                "    address 192.168.2.136\n"
                "    netmask 255.255.255.0\n"
                "    gateway 192.168.2.1\n";
        }else{
            configContent =
                "# interface file auto-generated by buildroot\n"
                "auto lo\n"
                "iface lo inet loopback\n\n"
                "auto eth0\n"
                "iface eth0 inet dhcp\n\n"
                "auto eth1\n"
                "iface eth1 inet static\n"
                "    address 192.168.2.136\n"
                "    netmask 255.255.255.0\n"
                "    gateway 192.168.2.1\n";
        }

        // WriteOnly -> Clear the original content
        QTextStream out(&file);
        out << configContent;
        file.close();

        QString restartCmd = "/etc/init.d/S40network restart";
        if (system(restartCmd.toStdString().c_str()) == 0) {
            return true;
        }

        qCWarning(config) << "[refresh_interfaces]:/etc/init.d/S40network restart failed!";
        return false;
    }

    qCWarning(config) << "[refresh_interfaces]:Cannot open interfaces file for writing";
    return false;
}
