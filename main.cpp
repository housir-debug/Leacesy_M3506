#include <QDir>
#include <QThread>
#include <QWidget>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include "auxiliary/simple_logger.h"
#include "channel/uart_channel.h"
#include "control/tcp_server.h"
#include "control/web_server.h"
#include "control/can_server.h"
#include "control/uart_server.h"
#include "widgets/mainwindow.h"
#include "widgets/test.h"

Q_LOGGING_CATEGORY(application, "APP")

using CanSign_toUartCh = void (CanServerManager::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
std::vector<CanSign_toUartCh> can_signal = {
    #define CHANNEL(n) static_cast<CanSign_toUartCh>(&CanServerManager::to_UartChannel##n),
    CHANNEL_COUNT
    #undef CHANNEL
};

using QWidgetSign_toUartCh = void (Mainwindow::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
std::vector<QWidgetSign_toUartCh> qml_signal = {
    #define CHANNEL(n) static_cast<QWidgetSign_toUartCh>(&Mainwindow::to_UartChannel##n),
    CHANNEL_COUNT
    #undef CHANNEL
};

using ScpiSign_toUartCh = void (ScpiManager::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
std::vector<ScpiSign_toUartCh> scpi_signal = {
    #define CHANNEL(n) static_cast<ScpiSign_toUartCh>(&ScpiManager::to_UartChannel##n),
    CHANNEL_COUNT
    #undef CHANNEL
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Leacesy_Ryan");
    QApplication::setOverrideCursor(Qt::BlankCursor);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    // QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); -* Warning: It will deform.

    // load config
    const QString configPath = QDir(QApplication::applicationDirPath()).filePath("..");
    if (!QDir(configPath).exists() || !ConfigManager::init(configPath)) {
        qCWarning(application) << "Application initialization failed!";
        return 1;
    }

    // setting log config
    loggermanage(ConfigManager::s_loglevel, configPath);
    QObject::connect(&app, &QApplication::aboutToQuit, &shutdownLogger);

    std::shared_ptr<BatteryModelManager> BatteryModel_share = std::make_shared<BatteryModelManager>(configPath);

    // screen GUI engine and gui-bridge create
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    std::unique_ptr<test> testview;
    std::unique_ptr<Mainwindow> mainwindow;
    if (ConfigManager::s_enableDisplay){
        mainwindow = std::make_unique<Mainwindow>();
        mainwindow->m_modelManager = BatteryModel_share;
        mainwindow->show();

        /*scene.addWidget(mainwindow.get());
        QSize mainWinSize = mainwindow->size();
        view.setSceneRect(0, 0, mainWinSize.height(), mainWinSize.width());
        view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //view.fitInView(view.sceneRect(), Qt::KeepAspectRatio);
        view.rotate(90);
        view.show();*/

        //testview = std::make_unique<test>();
        //testview->show();
    }

    // CAN Server create
    std::unique_ptr<CanServerManager> canServer;
    if (ConfigManager::s_enableCANServer){
        canServer = std::make_unique<CanServerManager>();
        if (!canServer->startServer()) {
            qCWarning(application) << "canServer not Normal start!";
            return 1;
        }

        QObject::connect(canServer.get(),&CanServerManager::isRemote,mainwindow.get(),&Mainwindow::update_remotemodel,Qt::QueuedConnection);
        QObject::connect(mainwindow.get(),&Mainwindow::to_CANid,canServer.get(),&CanServerManager::change_canid,Qt::QueuedConnection);
    }

    // can channel create
    /*Not necessary for the time being.*/

    // SCPI parser and uart channel create
    std::shared_ptr<ScpiManager> Scpi_share = std::make_shared<ScpiManager>();
    std::vector<std::unique_ptr<UartChannelManager>> Channel_list;
    if (ConfigManager::s_enableUartMess){
        // config form config_manager
        for (const auto& config : configs) {
            auto channel = std::make_unique<UartChannelManager>();
            channel->m_scpiManager = Scpi_share;

            if (!channel->initSerialPort(config.port, config.baudRate)) {
                qCWarning(application) << "uart channel "<< config.port <<" Initialization failed!";
                return 1;
            }

            /* screen view slot function */
            QObject::connect(mainwindow.get(),qml_signal[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);

            QObject::connect(channel.get(),&UartChannelManager::CH_svChanged,mainwindow.get(),&Mainwindow::update_SoftVer,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_hvChanged,mainwindow.get(),&Mainwindow::update_HardVer,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_VoltageChanged,mainwindow.get(),&Mainwindow::update_Voltage,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_CurrentAndUnitChanged,mainwindow.get(),&Mainwindow::update_CurrentAndUnit,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_RangeChanged,mainwindow.get(),&Mainwindow::update_Range,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_StatusChanged,mainwindow.get(),&Mainwindow::update_Status,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_cvChanged,mainwindow.get(),&Mainwindow::update_Cv,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_ccChanged,mainwindow.get(),&Mainwindow::update_Cc,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_ovpChanged,mainwindow.get(),&Mainwindow::update_Ovp,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_isOutputChanged,mainwindow.get(),&Mainwindow::update_IsOutput,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_impChanged,mainwindow.get(),&Mainwindow::update_Imp,Qt::QueuedConnection);

            /* remote api slot function */
            QObject::connect(Scpi_share.get(),scpi_signal[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);

            if (ConfigManager::s_enableCANServer){
                QObject::connect(canServer.get(),can_signal[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::to_CanServer,canServer.get(),&CanServerManager::sendFrame,Qt::QueuedConnection);
            }

            Channel_list.push_back(std::move(channel)); // move set <channel> can move
        }
    }

    // LAN Server create
    std::unique_ptr<WebServerManager> webServer;
    if (ConfigManager::s_enableWEBServer){
        webServer = std::make_unique<WebServerManager>();
        webServer->m_BatteryManager = BatteryModel_share;
        webServer->m_scpiManager = Scpi_share;
        if (!webServer->startServer()) {
            qCWarning(application) << "WebServerManager not Normal start!";
            return 1;
        }

        QObject::connect(webServer.get(),&WebServerManager::isRemote,mainwindow.get(),&Mainwindow::update_remotemodel,Qt::QueuedConnection);
    }

    std::unique_ptr<TcpServerManager> vxiServer;
    if (ConfigManager::s_enableLANServer){
        vxiServer = std::make_unique<TcpServerManager>();
        vxiServer->m_scpiManager = Scpi_share;
        if (!vxiServer->startServer()) {
            qCWarning(application) << "TcpServer not Normal start!";
            return 1;
        }

        QObject::connect(vxiServer.get(),&TcpServerManager::isRemote,mainwindow.get(),&Mainwindow::update_remotemodel,Qt::QueuedConnection);
    }

    // UART Server create
    std::unique_ptr<UartServerManager> uartServer;
    if (ConfigManager::s_enableUARTServer){
        uartServer = std::make_unique<UartServerManager>();
        uartServer->m_scpiManager = Scpi_share;
        if (!uartServer->startServer("/dev/ttyWCH27",QSerialPort::Baud38400)) {
            qCWarning(application) << "UartServer not Normal start!";
            return 1;
        }

        QObject::connect(uartServer.get(),&UartServerManager::isRemote,mainwindow.get(),&Mainwindow::update_remotemodel,Qt::QueuedConnection);
    }

    // GPIB server create
    /*Not necessary for the time being.*/

    //QTimer::singleShot(9000, &app, &QGuiApplication::quit); // 9s
    return app.exec();
}

