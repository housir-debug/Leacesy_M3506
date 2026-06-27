#ifndef DIGITALCARDPAGE_H
#define DIGITALCARDPAGE_H

#include <QWidget>
#include <QMap>
#include "digitalcard.h"

namespace Ui {
class digitalCardPage;
}

class DigitalCardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DigitalCardPage(QWidget *parent = nullptr);
    ~DigitalCardPage();

    int addCard(const QString &channelName = QString());
    digitalcard* addCardWithData(const QString &channelName,
                                  double voltage = 0.0,
                                  double current = 0.0,
                                  bool cvChecked = false,
                                  bool ccChecked = false,
                                  bool ovpChecked = false);

    // 批量添加卡片
    void addCards(int count);
    void addCardsFromList(const QList<QString> &channelNames);

    // 删除卡片
    bool removeCard(int index);
    bool removeCard(digitalcard *card);
    void removeAllCards();

    // 获取卡片
    digitalcard* getCard(int index) const;
    QList<digitalcard*> getAllCards() const;
    int cardCount() const;

    // 通过通道名称查找卡片
    digitalcard* findCardByChannelName(const QString &name) const;
    int findCardIndexByChannelName(const QString &name) const;

    // ========== 状态管理接口 ==========

    // 批量设置状态
    void setAllCardsState(bool state, bool animated = true);
    void setCardState(int index, bool state, bool animated = true);

    // 获取所有卡片的开启/关闭状态
    QList<bool> getAllCardsState() const;
    QList<int> getOpenCardIndices() const;

    // ========== 数据更新接口 ==========

    // 更新卡片数据（电压、电流等）
    void updateCardData(int index, double voltage, double current);
    void updateCardCVCC(int index, bool cv, bool cc, bool ovp);
    void updateCardValues(int index, const QString &cvValue,
                          const QString &ccValue,
                          const QString &ovpValue);

    // ========== 滚动控制接口 ==========

    void scrollToCard(int index, bool animated = true);
    void scrollToCard(digitalcard *card, bool animated = true);
    void scrollToNext();
    void scrollToPrevious();

    // ========== 卡片宽度自适应 ==========

    void setCardWidth(int width);
    int cardWidth() const { return m_cardWidth; }
    void setCardSpacing(int spacing);
    int cardSpacing() const { return m_cardSpacing; }

    // ========== 可见性控制 ==========

    void setCardsVisible(bool visible);
    void showCards() { setCardsVisible(true); }
    void hideCards() { setCardsVisible(false); }

signals:
    // 卡片事件信号
    void cardClicked(int index, digitalcard *card);
    void cardLongPressed(int index, digitalcard *card);
    void cardStateChanged(int index, bool state);

    // 卡片管理信号
    void cardAdded(int index, digitalcard *card);
    void cardRemoved(int index);
    void allCardsRemoved();

    // 滚动信号
    void scrollPositionChanged(int position);

private slots:
    // 卡片事件处理
    void onCardClicked();
    void onCardLongPressed();
    void onCardStateChanged(bool state);

private:
    Ui::digitalCardPage *ui;

    QList<digitalcard*> m_cards;
    int m_cardWidth;                     // 卡片宽度
    int m_cardSpacing;                  // 卡片间距

    void connectCardSignals(digitalcard *card);
    void disconnectCardSignals(digitalcard *card);
    digitalcard* createDefaultCard(const QString &channelName = QString());
    void updateCardLayout();
};

#endif // DIGITALCARDPAGE_H
