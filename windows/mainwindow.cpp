#include "mainwindow.h"
#include "digitalpage.h"
//#include "BatteryHomePage.h"
//#include "SettingPage.h"
//#include "FunctionPage.h"
#include <QApplication>
#include <QScreen>
#include <QGraphicsOpacityEffect>
#include <QStyle>

// 实例初始化

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setupUI();
    setupConnections();
    createLockOverlay();

    showFullScreen();

    setStyleSheet("QMainWindow { background-color: #0d1b2a; }");
}

MainWindow::~MainWindow()
{
}

// 绘制 UI

void MainWindow::setupUI()
{
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setStyleSheet("background-color: #0d1b2a;");

    m_digitalHomePage = new DigitalHomePage(this);
    //m_batteryHomePage = new BatteryHomePage(this);
    //m_settingPage = new SettingPage(this);
    //m_functionPage = new FunctionPage(this);

    // 添加到StackedWidget
    m_stackedWidget->addWidget(m_digitalHomePage);   // Index: 0
    //m_stackedWidget->addWidget(m_batteryHomePage);   // Index: 1
    //m_stackedWidget->addWidget(m_settingPage);       // Index: 2
    //m_stackedWidget->addWidget(m_functionPage);      // Index: 3

    setCentralWidget(m_stackedWidget);
}

void MainWindow::setupConnections()
{
    // DigitalHomePage 信号连接
    DigitalHomePage *digitalPage = qobject_cast<DigitalHomePage*>(m_digitalHomePage);
    if (digitalPage) {
        connect(digitalPage, &DigitalHomePage::toSettingPage,
                this, &MainWindow::showSettingPage);
        connect(digitalPage, &DigitalHomePage::toBatteryHomePage,
                this, &MainWindow::showBatteryHomePage);
        connect(digitalPage, &DigitalHomePage::toFunctionPage,
                this, &MainWindow::showFunctionPage);
    }

    // BatteryHomePage 信号连接
    /*BatteryHomePage *batteryPage = qobject_cast<BatteryHomePage*>(m_batteryHomePage);
    if (batteryPage) {
        connect(batteryPage, &BatteryHomePage::toDigitalHomePage,
                this, &MainWindow::showDigitalHomePage);
        connect(batteryPage, &BatteryHomePage::toSettingPage,
                this, &MainWindow::showSettingPage);
        connect(batteryPage, &BatteryHomePage::toFunctionPage,
                this, &MainWindow::showFunctionPage);
    }

    // SettingPage 信号连接
    SettingPage *settingPage = qobject_cast<SettingPage*>(m_settingPage);
    if (settingPage) {
        connect(settingPage, &SettingPage::toDigitalHomePage,
                this, &MainWindow::showDigitalHomePage);
        connect(settingPage, &SettingPage::toBatteryHomePage,
                this, &MainWindow::showBatteryHomePage);
    }

    // FunctionPage 信号连接
    FunctionPage *functionPage = qobject_cast<FunctionPage*>(m_functionPage);
    if (functionPage) {
        connect(functionPage, &FunctionPage::backRequested,
                this, &MainWindow::onBackRequested);
    }*/
}

void MainWindow::createLockOverlay()
{
    m_lockOverlay = new QWidget(this);
    m_lockOverlay->setStyleSheet("background-color: #90000000;");
    m_lockOverlay->setGeometry(0, 0, width(), height());
    //m_lockOverlay->setEnabled(false);
    m_lockOverlay->hide();

    m_logoLabel = new QLabel(m_lockOverlay);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setGeometry(0, 0, width(), height());
    m_logoLabel->setPixmap(QPixmap(":/web/web/icon/leacesyicon.png")
                 .scaled(width() * 0.81, height() * 0.81,Qt::KeepAspectRatio, Qt::SmoothTransformation));


    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect;
    opacityEffect->setOpacity(0.69);
    m_logoLabel->setGraphicsEffect(opacityEffect);

    m_unlockButton = new QPushButton("解锁", m_lockOverlay);
    m_unlockButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4A6FA5;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 20px;"
        "    padding: 10px 30px;"
        "    font-size: 18px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #5B8BC5;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3A5F85;"
        "}"
    );
    m_unlockButton->setCursor(Qt::PointingHandCursor);
    m_unlockButton->setGeometry(width()/2 - 60, height() - 80, 120, 40);

    connect(m_unlockButton, &QPushButton::clicked, this, [this]() {
        // Uart_bridge.update_remotemodel(0)
        m_lockOverlay->hide();
    });
}

// 槽函数 - 后续删除掉

void MainWindow::showDigitalHomePage()
{
    m_stackedWidget->setCurrentIndex(0);
}

void MainWindow::showBatteryHomePage()
{
    m_homePageModel = 0;
    m_stackedWidget->setCurrentIndex(1);
}

void MainWindow::showSettingPage()
{
    m_stackedWidget->setCurrentIndex(2);
}

void MainWindow::showFunctionPage(int channel)
{
    m_homePageModel = 0;
    m_functionChannel = channel;
    m_stackedWidget->setCurrentIndex(3);
}

void MainWindow::onBackRequested()
{
    if (m_homePageModel == 0) {
        m_stackedWidget->setCurrentIndex(0);
    } else {
        m_stackedWidget->setCurrentIndex(1);
    }
}


void MainWindow::onUartBridgeChanged()
{
    // 根据Uart_bridge.reface状态显示/隐藏锁覆盖层
    // 这里需要根据实际的Uart_bridge实现来判断
}
