#include "digitalcardpage.h"
#include "ui_digitalcardpage.h"
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QDebug>

DigitalCardPage::DigitalCardPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::digitalCardPage),
    m_cardWidth(200),
    m_cardSpacing(10)
{
    ui->setupUi(this);
    // m_cards 初始化
    calculateCardWidth();

}

DigitalCardPage::~DigitalCardPage()
{
    removeAllCards();
    delete ui;
}


// ========== 卡片创建 ==========

digitalcard* DigitalCardPage::createDefaultCard(const QString &channelName)
{
    digitalcard *card = new digitalcard();

    if (channelName.isEmpty()) {
        int nextNumber = m_cards.size() + 1;
        card->setChannelName(QString("CH-%1").arg(nextNumber, 2, 10, QChar('0')));
    } else {
        card->setChannelName(channelName);
    }

    return card;
}

int DigitalCardPage::addCard(const QString &channelName)
{
    digitalcard *card = createDefaultCard(channelName);
    return addCardWithData(channelName, 0.0, 0.0, false, false, false);
}

digitalcard* DigitalCardPage::addCardWithData(const QString &channelName,
                                               double voltage,
                                               double current,
                                               bool cvChecked,
                                               bool ccChecked,
                                               bool ovpChecked)
{
    digitalcard *card = createDefaultCard(channelName);

    // 设置数据
    card->setVoltage(voltage);
    card->setCurrent(current);
    card->setCVChecked(cvChecked);
    card->setCCChecked(ccChecked);
    card->setOVPChecked(ovpChecked);

    // 添加到布局
    m_cards.append(card);
    ui->scrollAreaWidgetContents->layout()->addWidget(card);
    card->setFixedWidth(m_cardWidth);

    // 连接信号
    connectCardSignals(card);

    // 重新计算卡片宽度
    calculateCardWidth();

    // 发射信号
    int index = m_cards.size() - 1;
    emit cardAdded(index, card);

    qDebug() << "Card added:" << card->channelName() << "Total:" << m_cards.size();

    return card;
}

void DigitalCardPage::addCards(int count)
{
    for (int i = 0; i < count; ++i) {
        addCard(QString());
    }
}

void DigitalCardPage::addCardsFromList(const QList<QString> &channelNames)
{
    for (const QString &name : channelNames) {
        addCard(name);
    }
}

// ========== 卡片删除 ==========

bool DigitalCardPage::removeCard(int index)
{
    if (index < 0 || index >= m_cards.size()) {
        return false;
    }

    digitalcard *card = m_cards.takeAt(index);
    disconnectCardSignals(card);
    ui->scrollAreaWidgetContents->layout()->removeWidget(card);
    card->deleteLater();

    emit cardRemoved(index);

    // 重新计算卡片宽度
    calculateCardWidth();

    qDebug() << "Card removed at index:" << index << "Remaining:" << m_cards.size();

    return true;
}

bool DigitalCardPage::removeCard(digitalcard *card)
{
    int index = m_cards.indexOf(card);
    if (index < 0) {
        return false;
    }
    return removeCard(index);
}

void DigitalCardPage::removeAllCards()
{
    if (m_cards.isEmpty()) return;

    for (auto *card : m_cards) {
        disconnectCardSignals(card);
        ui->scrollAreaWidgetContents->layout()->removeWidget(card);
        card->deleteLater();
    }

    m_cards.clear();
    emit allCardsRemoved();

    qDebug() << "All cards removed";
}

// ========== 卡片查询 ==========

digitalcard* DigitalCardPage::getCard(int index) const
{
    if (index < 0 || index >= m_cards.size()) {
        return nullptr;
    }
    return m_cards[index];
}

QList<digitalcard*> DigitalCardPage::getAllCards() const
{
    return m_cards;
}

int DigitalCardPage::cardCount() const
{
    return m_cards.size();
}

digitalcard* DigitalCardPage::findCardByChannelName(const QString &name) const
{
    for (auto *card : m_cards) {
        if (card->channelName() == name) {
            return card;
        }
    }
    return nullptr;
}

int DigitalCardPage::findCardIndexByChannelName(const QString &name) const
{
    for (int i = 0; i < m_cards.size(); ++i) {
        if (m_cards[i]->channelName() == name) {
            return i;
        }
    }
    return -1;
}

// ========== 状态管理 ==========

void DigitalCardPage::setAllCardsState(digitalcard::ChannelState state, bool animated)
{
    for (auto *card : m_cards) {
        card->setChannelState(state, animated);
    }
}

void DigitalCardPage::setCardState(int index, digitalcard::ChannelState state, bool animated)
{
    digitalcard *card = getCard(index);
    if (card) {
        card->setChannelState(state, animated);
    }
}

QList<bool> DigitalCardPage::getAllCardsState() const
{
    QList<bool> states;
    for (auto *card : m_cards) {
        states.append(card->channelState() == digitalcard::StateOpen);
    }
    return states;
}

QList<int> DigitalCardPage::getOpenCardIndices() const
{
    QList<int> indices;
    for (int i = 0; i < m_cards.size(); ++i) {
        if (m_cards[i]->channelState() == digitalcard::StateOpen) {
            indices.append(i);
        }
    }
    return indices;
}

// ========== 数据更新 ==========

void DigitalCardPage::updateCardData(int index, double voltage, double current)
{
    digitalcard *card = getCard(index);
    if (card) {
        card->setVoltage(voltage);
        card->setCurrent(current);
    }
}

void DigitalCardPage::updateCardCVCC(int index, bool cv, bool cc, bool ovp)
{
    digitalcard *card = getCard(index);
    if (card) {
        card->setCVChecked(cv);
        card->setCCChecked(cc);
        card->setOVPChecked(ovp);
    }
}

void DigitalCardPage::updateCardValues(int index, const QString &cvValue,
                                        const QString &ccValue,
                                        const QString &ovpValue)
{
    digitalcard *card = getCard(index);
    if (card) {
        card->setCvValue(cvValue);
        card->setCcValue(ccValue);
        card->setOvpValue(ovpValue);
    }
}

// ========== 滚动控制 ==========

void DigitalCardPage::scrollToCard(int index, bool animated)
{
    if (m_cards.isEmpty() || index < 0 || index >= m_cards.size()) {
        return;
    }

    // 计算目标位置（让卡片居中显示）
    int cardWidth = m_cardWidth + m_cardSpacing;
    int targetX = index * cardWidth;

    int viewWidth = ui->scrollArea->viewport()->width();
    int centerOffset = (viewWidth - m_cardWidth) / 2;
    targetX = targetX - centerOffset;
    targetX = qMax(0, targetX);

    // 限制最大滚动位置
    int totalWidth = m_cards.size() * cardWidth - m_cardSpacing + 20;
    int maxScroll = qMax(0, totalWidth - viewWidth);
    targetX = qMin(targetX, maxScroll);

    QScrollBar *hBar = ui->scrollArea->horizontalScrollBar();

    if (animated) {
        QPropertyAnimation *anim = new QPropertyAnimation(hBar, "value", this);
        anim->setDuration(300);
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        anim->setStartValue(hBar->value());
        anim->setEndValue(targetX);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        hBar->setValue(targetX);
    }
}

void DigitalCardPage::scrollToCard(digitalcard *card, bool animated)
{
    int index = m_cards.indexOf(card);
    if (index >= 0) {
        scrollToCard(index, animated);
    }
}

void DigitalCardPage::scrollToNext()
{
    if (m_cards.isEmpty()) return;

    QScrollBar *hBar = ui->scrollArea->horizontalScrollBar();
    int currentValue = hBar->value();
    int step = m_cardWidth + m_cardSpacing;
    int newValue = currentValue + step;

    int maxValue = hBar->maximum();
    if (newValue > maxValue) {
        newValue = maxValue;
    }

    if (newValue != currentValue) {
        hBar->setValue(newValue);
    }
}

void DigitalCardPage::scrollToPrevious()
{
    if (m_cards.isEmpty()) return;

    QScrollBar *hBar = ui->scrollArea->horizontalScrollBar();
    int currentValue = hBar->value();
    int step = m_cardWidth + m_cardSpacing;
    int newValue = currentValue - step;

    if (newValue < 0) {
        newValue = 0;
    }

    if (newValue != currentValue) {
        hBar->setValue(newValue);
    }
}

// ========== 卡片大小控制 ==========

void DigitalCardPage::setCardWidth(int width)
{
    if (width != m_cardWidth && width >= 100) {
        m_cardWidth = width;
        for (auto *card : m_cards) {
            card->setFixedWidth(m_cardWidth);
        }
    }
}

void DigitalCardPage::setCardSpacing(int spacing)
{
    if (spacing != m_cardSpacing && spacing >= 0) {
        m_cardSpacing = spacing;
        // 更新布局间距
        QLayout *layout = ui->scrollAreaWidgetContents->layout();
        if (layout) {
            layout->setSpacing(m_cardSpacing);
        }
    }
}

// ========== 可见性控制 ==========

void DigitalCardPage::setCardsVisible(bool visible)
{
    for (auto *card : m_cards) {
        card->setVisible(visible);
    }
}

// ========== 信号连接 ==========

void DigitalCardPage::connectCardSignals(digitalcard *card)
{
    connect(card, &digitalcard::clicked, this, &DigitalCardPage::onCardClicked);
    connect(card, &digitalcard::longPressed, this, &DigitalCardPage::onCardLongPressed);
    connect(card, &digitalcard::stateChanged, this, &DigitalCardPage::onCardStateChanged);
}

void DigitalCardPage::disconnectCardSignals(digitalcard *card)
{
    disconnect(card, &digitalcard::clicked, this, &DigitalCardPage::onCardClicked);
    disconnect(card, &digitalcard::longPressed, this, &DigitalCardPage::onCardLongPressed);
    disconnect(card, &digitalcard::stateChanged, this, &DigitalCardPage::onCardStateChanged);
}

// ========== 卡片事件处理 ==========

void DigitalCardPage::onCardClicked()
{
    digitalcard *card = qobject_cast<digitalcard*>(sender());
    if (card) {
        int index = m_cards.indexOf(card);
        if (index >= 0) {
            emit cardClicked(index, card);
            qDebug() << "Card clicked:" << index << card->channelName();
        }
    }
}

void DigitalCardPage::onCardLongPressed()
{
    digitalcard *card = qobject_cast<digitalcard*>(sender());
    if (card) {
        int index = m_cards.indexOf(card);
        if (index >= 0) {
            emit cardLongPressed(index, card);
            qDebug() << "Card long pressed:" << index << card->channelName();
        }
    }
}

void DigitalCardPage::onCardStateChanged(digitalcard::ChannelState state)
{
    digitalcard *card = qobject_cast<digitalcard*>(sender());
    if (card) {
        int index = m_cards.indexOf(card);
        if (index >= 0) {
            emit cardStateChanged(index, state);
            qDebug() << "Card state changed:" << index
                     << card->channelName()
                     << (state == digitalcard::StateOpen ? "OPEN" : "CLOSED");
        }
    }
}

// ========== 布局更新 ==========

void DigitalCardPage::updateCardLayout()
{
    // 重新计算卡片宽度
    calculateCardWidth();

    // 强制更新布局
    QLayout *layout = ui->scrollAreaWidgetContents->layout();
    if (layout) {
        layout->invalidate();
        layout->activate();
    }
}
