// DigitalHomePage.cpp
#include "digitalpage.h"
#include "digitalcardwidget.h"
#include "auxiliary/qml_agency.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QMouseEvent>
#include <QTimer>

DigitalHomePage::DigitalHomePage(QWidget *parent): QWidget(parent)
{
    setupUI();
    updateChannels();

    // 定时更新通道数据
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DigitalHomePage::updateChannels);
    timer->start(100);
}

DigitalHomePage::~DigitalHomePage()
{
}

void DigitalHomePage::setupUI()
{
    setStyleSheet("background-color: #0d1b2a;");
}

void DigitalHomePage::updateChannels()
{
    // m_activeChannels = Uart_bridge.getActiveChannels();
    // 示例数据
    if (m_activeChannels.isEmpty()) {
        m_activeChannels = {1, 2, 3, 4};
    }

    // 如果卡片数量变化，重新创建
    if (m_cards.size() != m_activeChannels.size()) {
        // 清除旧卡片
        qDeleteAll(m_cards);
        m_cards.clear();

        // 创建新卡片
        const auto& channels = m_activeChannels;
        for (int channel : channels) {
            createCard(channel);
        }
        layoutCards();
    }

    // 更新卡片数据
    /*for (int i = 0; i < m_cards.size() && i < m_activeChannels.size(); ++i) {
        //int channel = m_activeChannels[i];
        DigitalCardWidget *card = m_cards[i];

        // 从Uart_bridge获取数据
        // card->setChannelOutput(Uart_bridge["ch" + channel + "_isOutput"]);
        // card->setVoltage(Uart_bridge["ch" + channel + "_Voltage"]);
        // card->setCurrent(Uart_bridge["ch" + channel + "_Current"]);
        // ...
    }*/
}

void DigitalHomePage::createCard(int channel)
{
    DigitalCardWidget *card = new DigitalCardWidget(this);
    card->setChannelName(QString("CH%1").arg(channel));
    card->setEnabled(m_enclick);

    connect(card, &DigitalCardWidget::clicked, this,[this, channel]() {
        onCardClicked(channel);
    });
    connect(card, &DigitalCardWidget::pressAndHold, this,[this, channel]() {
        onCardPressAndHold(channel);
    });

    m_cards.append(card);
}

void DigitalHomePage::layoutCards()
{
    if (m_cards.isEmpty()) return;

    // 清除之前的布局
    QLayout *oldLayout = layout();
    if (oldLayout) {
        delete oldLayout;
    }

    // 使用水平布局，居中显示，间距30px
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(30);
    layout->setContentsMargins(50, 50, 50, 50);
    layout->setAlignment(Qt::AlignCenter);

    const auto& cards = m_cards;
    for (DigitalCardWidget *card : cards) {
        layout->addWidget(card);
    }
}

void DigitalHomePage::onCardClicked(int channel)
{
    Q_UNUSED(channel)
    // 切换输出状态
    // bool currentOutput = Uart_bridge["ch" + channel + "_isOutput"];
    // Uart_bridge.setChannel_Output(channel, !currentOutput);
}

void DigitalHomePage::onCardPressAndHold(int channel)
{
    emit toFunctionPage(channel);
}

void DigitalHomePage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 调整卡片大小以适应窗口
    /*if (!m_cards.isEmpty()) {
        int cardWidth = 280;
        int cardHeight = 400;
        int availableWidth = width() - 100;
        int availableHeight = height() - 100;

        // 计算缩放比例
        qreal scaleX = qMin(1.0, (qreal)availableWidth / (m_cards.size() * (cardWidth + 30)));
        qreal scaleY = qMin(1.0, (qreal)availableHeight / cardHeight);
        qreal scale = qMin(scaleX, scaleY);

        // 应用缩放
        // for (DigitalCardWidget *card : m_cards) {
        //     card->setFixedSize(cardWidth * scale, cardHeight * scale);
        // }
    }*/
}

void DigitalHomePage::mousePressEvent(QMouseEvent *event)
{
    m_pressPos = event->pos();
    m_swiping = false;
}

void DigitalHomePage::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - m_pressPos;

    // 向下滑动 -> BatteryHomePage
    if (delta.y() > 81) {
        emit toBatteryHomePage();
    }
    // 向上滑动 -> SettingPage
    else if (delta.y() < -81) {
        emit toSettingPage();
    }
}
