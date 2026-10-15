#include "interaction/channelsetpage.h"
#include "interaction/devicesetpage.h"
#include "interaction/channelpage.h"
#include "component/remoteoverlay.h"
#include "auxiliary/battery_model.h"
#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "channel/uart_protocol.h"
#include "channel/uart_channel.h"
#include "control/web_server.h"
#include "control/can_server.h"
#include "control/vxi_server.h"
#include "control/r32_server.h"
#include "ui_appmain.h"
#include "appmain.h"
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QThread>
#include <QDir>

Mainwindow::Mainwindow(QWidget *parent) : QWidget(parent), ui(new Ui::Mainwindow)
{
    QString configPath = QDir(QApplication::applicationDirPath()).filePath("..");
    if (QDir(configPath).exists() && ConfigManager::init(configPath)) {
        BatteryModelManager::instance().m_modelDirectory = configPath + "/battery_models";
        BatteryModelManager::instance().loadAllModels();

        initUi();
        ishow();
        initO();
    }
}
Mainwindow::~Mainwindow()
{
    delete ui;

    if (m_serThread) {
       m_serThread->quit();
       m_serThread->wait(1000); // wait 1s
       m_serThread->deleteLater();
       delete m_serThread;
   }
}

void Mainwindow::initUi()
{
    ui->setupUi(this);

    m_ChPage =  new channelpage(this);
    ui->stackedwidget->addWidget(m_ChPage);// 0

    m_ChSetPage = new channelsetting(this);
    ui->stackedwidget->addWidget(m_ChSetPage);// 1
    m_ChSetPage->setCombModel();

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
    QObject::connect(m_ChSetPage,&channelsetting::to_chSOC,m_ChPage,&channelpage::update_Soc,Qt::DirectConnection);
    QObject::connect(m_ChSetPage,&channelsetting::to_chCAP,m_ChPage,&channelpage::update_Cap,Qt::DirectConnection);
    QObject::connect(m_ChSetPage,&channelsetting::to_chModel,m_ChPage,&channelpage::update_Model,Qt::DirectConnection);
    QObject::connect(m_ChSetPage,&channelsetting::backRequested,this,[this]{ui->stackedwidget->setCurrentIndex(0);});
    QObject::connect(m_ChPage,&channelpage::jump_chsetpage,this,[this]{ui->stackedwidget->setCurrentIndex(1);});
    QObject::connect(m_ChPage,&channelpage::jump_devicesetpage,this,[this]{ui->stackedwidget->setCurrentIndex(2);});

    m_DeviceSetPage = new devicesetpage(this);
    ui->stackedwidget->addWidget(m_DeviceSetPage);// 2
    QObject::connect(m_DeviceSetPage,&devicesetpage::backRequested,this,[this]{ui->stackedwidget->setCurrentIndex(0);});
    QObject::connect(&ChannelBus::instance(),&ChannelBus::interAcUpdate,m_DeviceSetPage,&devicesetpage::responseUpdate,Qt::QueuedConnection);

    m_RemoteOverlay = new remoteoverlay(this);
    QObject::connect(m_RemoteOverlay, &remoteoverlay::exitRemote, this, [](){ConfigManager::control_Mode.store(InterSt::LOCAL);});
    QObject::connect(&ChannelBus::instance(),&ChannelBus::switchRemote,this,[this](int reface){
        if (ConfigManager::control_Mode.load() != reface){
            ConfigManager::control_Mode.store(reface);
            if (reface == InterSt::LOCAL){
                m_RemoteOverlay->hide();
            }else{
                m_RemoteOverlay->show();
            }
        }},Qt::QueuedConnection);
}

void Mainwindow::initO()
{
    if (Module::enable_UartCh){
        using ChannelBus_toUartCh = void (ChannelBus::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
        std::vector<ChannelBus_toUartCh> Bus_toUCh = {
            #define CHANNEL(n) static_cast<ChannelBus_toUartCh>(&ChannelBus::to_UartChannel##n),
            CHANNEL_COUNT
            #undef CHANNEL
        };
        using ChannelBus_toUartChReset = void (ChannelBus::*)();
        std::vector<ChannelBus_toUartChReset> Bus_toUChRe = {
            #define CHANNEL(n) static_cast<ChannelBus_toUartChReset>(&ChannelBus::to_UartChannel##n##Reset),
            CHANNEL_COUNT
            #undef CHANNEL
        };

        for (const auto& config : ChannelBus::instance().configs) {
            auto channel = std::make_unique<UartChannelManager>();
            channel->initUartchannel(config.channel, config.port, config.baudRate);

            QObject::connect(&ChannelBus::instance(),Bus_toUCh[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);
            QObject::connect(&ChannelBus::instance(),Bus_toUChRe[config.channel-1],channel.get(),&UartChannelManager::sendInitCommand,Qt::QueuedConnection);

            channels.push_back(std::move(channel));
        }
    }

    m_serThread = new QThread(this);
    m_serThread->setObjectName("ServerThread");
    QObject::connect(m_serThread, &QThread::started, this,[this](){
        if (Module::enable_UartSer){
            uartServer = new UartServerManager();
            QObject::connect(&ChannelBus::instance(),&ChannelBus::set_RS232Baud,uartServer,&UartServerManager::changeBaudRate,Qt::QueuedConnection);
        }

        if (Module::enable_CanSer){
            canServer = new CanServerManager();
            QObject::connect(&ChannelBus::instance(),&ChannelBus::to_CanServer,canServer,&CanServerManager::sendFrame,Qt::QueuedConnection);
            QObject::connect(&ChannelBus::instance(),&ChannelBus::set_canbaud,canServer,&CanServerManager::changebaud,Qt::QueuedConnection);
        }

        if (Module::enable_VxiSer){
            vxiServer = new VxiServerManager();
        }

        if (Module::enable_WebSer){
            webServer = new WebServerManager();
            webServer->m_ChPage = m_ChPage;
            QObject::connect(&ChannelBus::instance(),&ChannelBus::set_network,webServer,&WebServerManager::set_network,Qt::QueuedConnection);
        }});
    QObject::connect(m_serThread, &QThread::finished, this, [this](){
        if (uartServer){uartServer->deleteLater();}
        if (canServer){canServer->deleteLater();}
        if (vxiServer){vxiServer->deleteLater();}
        if (webServer){webServer->deleteLater();}
        });

    m_serThread->start();
}

void Mainwindow::ishow()
{
    if (Module::enable_DisPlay) {
        scene = new QGraphicsScene(this);
        view  = new QGraphicsView(scene);
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
