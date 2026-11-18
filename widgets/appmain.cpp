#include "interaction/channelsetpage.h"
#include "interaction/devicesetpage.h"
#include "interaction/channelpage.h"
#include "component/remoteoverlay.h"
#include "auxiliary/battery_model.h"
#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "channel/uart_protocol.h"
#include "channel/uart_channel.h"
#include "control/vxi11_server.h"
#include "control/web_server.h"
#include "control/can_server.h"
#include "control/r32_server.h"
#include "ui_appmain.h"
#include "appmain.h"
// --- QT package ---
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QThread>
#include <QDir>

Mainwindow::Mainwindow(QWidget *parent) : QWidget(parent), ui(new Ui::Mainwindow)
{
    QString configPath = QDir(QApplication::applicationDirPath()).filePath("..");
    if (QDir(configPath).exists()) {
        ConfigManager::setLogRules(LogLevel::Normal);
        BatteryModelManager::instance().loadAllModels(configPath);

        if (ConfigManager::init(configPath)){
            initUi(); // testUi(); initUi();
            initO();
            return;
        }

        qCCritical(aPMain)<<"(Mainwindow)inital APP-config fail!!!";
        return;
    }

    qCCritical(aPMain)<<"(Mainwindow)APP parent path not exist!!!";
}
Mainwindow::~Mainwindow()
{
    // stop m_serThread -> finished signed
    if (m_serThread && m_serThread->isRunning()) {
        m_serThread->quit();
        m_serThread->wait(3000);
    }//inherit this -> auto delete

    for (auto& t : m_chThreads) {
        if (t && t->isRunning()) {
            t->quit();
            t->wait(3000);
        }
    }//inherit this -> auto delete
    m_chThreads.clear();

    for (UartChannelManager* ch : channels) {
        delete ch;
    }//not inherit, mannel delete
    channels.clear();

    delete view;
    delete ui;
}

void Mainwindow::initUi()
{
    ui->setupUi(this);

    m_ChPage =  new channelpage(this);
    ui->stackedwidget->addWidget(m_ChPage);// 0
    m_ChSetPage = new channelsetting(this);
    ui->stackedwidget->addWidget(m_ChSetPage);// 1
    m_DeviceSetPage = new devicesetpage(this);
    ui->stackedwidget->addWidget(m_DeviceSetPage);// 2

    QObject::connect(m_ChSetPage,&channelsetting::to_chSOC,m_ChPage,&channelpage::update_Soc,Qt::DirectConnection);
    QObject::connect(m_ChSetPage,&channelsetting::to_chCAP,m_ChPage,&channelpage::update_Cap,Qt::DirectConnection);
    QObject::connect(m_ChSetPage,&channelsetting::to_chpage,m_ChPage,&channelpage::update_Page,Qt::DirectConnection);
    QObject::connect(m_ChSetPage,&channelsetting::to_chModel,m_ChPage,&channelpage::update_Model,Qt::DirectConnection);
    QObject::connect(m_ChPage,&channelpage::jump_chsetpage,this,[this]{ui->stackedwidget->setCurrentIndex(1);}, Qt::DirectConnection);
    QObject::connect(m_ChPage,&channelpage::jump_devicesetpage,this,[this]{ui->stackedwidget->setCurrentIndex(2);}, Qt::DirectConnection);
    QObject::connect(m_ChSetPage,&channelsetting::backRequested,this,[this]{ui->stackedwidget->setCurrentIndex(0);}, Qt::DirectConnection);
    QObject::connect(m_DeviceSetPage,&devicesetpage::backRequested,this,[this]{ui->stackedwidget->setCurrentIndex(0);}, Qt::DirectConnection);

    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_svChanged,this,[this](int ch,const QString &ver){
        m_ChPage->update_SoftVer(ch,ver);
        m_ChSetPage->update_ChEn(ch);
        },Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_hvChanged,this,[this](int ch,const QString &ver){
        m_ChPage->update_HardVer(ch,ver);
        m_ChSetPage->update_ChEn(ch);
        },Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_VoltageChanged,m_ChPage,&channelpage::update_Voltage,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_CurrentAndUnitChanged,m_ChPage,&channelpage::update_CurrentAndUnit,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_RangeChanged,m_ChPage,&channelpage::update_Range,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_StatusChanged,m_ChPage,&channelpage::update_Status,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_cvChanged,m_ChPage,&channelpage::update_Cv,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_ccChanged,m_ChPage,&channelpage::update_Cc,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_ovpChanged,m_ChPage,&channelpage::update_Ovp,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_isOutputChanged,m_ChPage,&channelpage::update_IsOutput,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::CH_impChanged,m_ChPage,&channelpage::update_Imp,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::interModelUpdate,m_ChSetPage,&channelsetting::setCombModel,Qt::QueuedConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::interAcUpdate,m_DeviceSetPage,&devicesetpage::responseUpdate,Qt::QueuedConnection);

    m_RemoteOverlay = new remoteoverlay(this);
    QObject::connect(m_RemoteOverlay, &remoteoverlay::exitRemote, this, [](){
        ConfigManager::control_Mode.store(InterSt::LOCAL);
        }, Qt::DirectConnection);
    QObject::connect(&ChannelBus::instance(),&ChannelBus::switchRemote,this,[this](int reface){
        if (ConfigManager::control_Mode.load() != reface){
            ConfigManager::control_Mode.store(reface);
            if (reface == InterSt::LOCAL){
                m_RemoteOverlay->hide();
            }else{
                m_RemoteOverlay->show();
            }
        }},Qt::QueuedConnection);

    if (Module::enable_DisPlay) {
        scene = new QGraphicsScene(this);
        view  = new QGraphicsView(scene);//,this - don`t display
        QGraphicsProxyWidget* proxy = scene->addWidget(this);
        proxy->setFocusPolicy(Qt::NoFocus);
        proxy->setRotation(0);

        view->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, false);
        view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setDragMode(QGraphicsView::NoDrag);
        view->setFrameShape(QFrame::NoFrame);
        //view.rotate(0);
        //this->show();
        view->show();

        /* test UI view */
        QList<int> channels = {3, 1, 4,2,8,9,24,15,32,12,18};
        //for (int i = 1; i <= 36; i++) {channels.append(i);}
        std::sort(channels.begin(), channels.end());
        for (const int ch : channels) {
            if (m_ChPage && m_ChSetPage){
                m_ChPage->update_SoftVer(ch,"1.2.3.4");
                m_ChPage->update_HardVer(ch,"4.3.2.1");
                m_ChSetPage->update_ChEn(ch);
            }
        }
    }
}
void Mainwindow::testUi()
{
    ui->setupUi(this);

    channelpage* m_ChPage = new channelpage(this);
    ui->stackedwidget->addWidget(m_ChPage);

    /*channelsetting* m_ChSetPage = new channelsetting(this);
    ui->stackedwidget->addWidget(m_ChSetPage);*/

    QList<int> channels = {3, 1, 4,2,8,9,24,15,32,12,18};
    //for (int i = 1; i <= 36; i++) {channels.append(i);}
    std::sort(channels.begin(), channels.end());

    for (const int ch : channels) {
        m_ChPage->update_SoftVer(ch,"1.2.3.4");
        m_ChPage->update_HardVer(ch,"4.3.2.1");
        //m_ChSetPage->update_ChEn(ch);
    }

    /*devicesetpage* m_DeviceSetPage = new devicesetpage(this);
    ui->stackedwidget->addWidget(m_DeviceSetPage);// 2*/

    qCDebug(aPMain) << "objectName:" << m_ChPage->objectName();

    //numberkeypad *key = new numberkeypad(this);
    //key->setContentsMargins(0, 0, 0, 0);
}

void Mainwindow::initO()
{
    if (Module::enable_UartCh){
        using ChannelBus_toUartCh = void (ChannelBus::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
        std::vector<ChannelBus_toUartCh> Bus_toUCh =
        {
            #define CHANNEL(n) static_cast<ChannelBus_toUartCh>(&ChannelBus::to_UartChannel##n),
            CHANNEL_COUNT
            #undef CHANNEL
        };
        using ChannelBus_toUartChReset = void (ChannelBus::*)();
        std::vector<ChannelBus_toUartChReset> Bus_toUChRe =
        {
            #define CHANNEL(n) static_cast<ChannelBus_toUartChReset>(&ChannelBus::to_UartChannel##n##Reset),
            CHANNEL_COUNT
            #undef CHANNEL
        };

        QHash<QThread*, int> threadLoad;
        for (const auto& config : ChannelBus::instance().configs) {
            QThread* worker = nullptr;

            if (!m_chThreads.empty()) {
                QThread* latest = m_chThreads.back();
                if (threadLoad.value(latest, 0) < JV6xxx::Thread_CHSUM) {
                    worker = latest;
                }
            }

            if (!worker){
                worker = new QThread(this);
                worker->setObjectName(QString("UartChThread_%1").arg(m_chThreads.size()));
                worker->start();

                threadLoad.insert(worker, 0);
                m_chThreads.push_back(worker);
            }

            auto channel = new UartChannelManager();
            channel->moveToThread(worker);

            QMetaObject::invokeMethod(channel, [channel,config](){
                channel->initUartchannel(config.channel, config.port, config.baudRate);
                }, Qt::QueuedConnection);
            QObject::connect(&ChannelBus::instance(),Bus_toUCh[config.channel-1],channel,&UartChannelManager::writeFrame,Qt::QueuedConnection);
            QObject::connect(&ChannelBus::instance(),Bus_toUChRe[config.channel-1],channel,&UartChannelManager::sendInitCommand,Qt::QueuedConnection);

            threadLoad[worker] += 1;
            channels.push_back(channel);
        }
    }

    m_serThread = new QThread(this);

    QObject::connect(m_serThread, &QThread::started, this,[this](){
        if (Module::enable_UartSer){
            uartServer = new UartServerManager();
            QObject::connect(&ChannelBus::instance(),&ChannelBus::set_RS232Baud,uartServer,&UartServerManager::changeBaudRate,Qt::QueuedConnection);
        }

        if (Module::enable_CanSer){
            canServer = new CanServerManager();
            QObject::connect(&ChannelBus::instance(),&ChannelBus::set_canbaud,canServer,&CanServerManager::changebaud,Qt::QueuedConnection);
            QObject::connect(&ChannelBus::instance(),&ChannelBus::to_CanServer,canServer,&CanServerManager::sendFrame,Qt::QueuedConnection);
        }

        if (Module::enable_WebSer){
            webServer = new WebServerManager();
            webServer->m_ChPage = m_ChPage;
            QObject::connect(&ChannelBus::instance(),&ChannelBus::set_network,webServer,&WebServerManager::set_network,Qt::QueuedConnection);
        }

        if (Module::enable_VxiSer){
            vxi11_server::instance();
        }}, Qt::DirectConnection);
    QObject::connect(m_serThread, &QThread::finished, this, [this](){
        if (Module::enable_UartSer) { delete uartServer; uartServer = nullptr; }
        if (Module::enable_CanSer)  { delete canServer;  canServer  = nullptr; }
        if (Module::enable_WebSer)  { delete webServer;  webServer  = nullptr; }
        if (Module::enable_VxiSer)  { vxi11_server::instance().stop(); }
        }, Qt::DirectConnection);

    m_serThread->setObjectName("ServerThread");
    m_serThread->start();
}
