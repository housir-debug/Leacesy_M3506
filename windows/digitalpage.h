// DigitalHomePage.h
#ifndef DIGITALHOMEPAGE_H
#define DIGITALHOMEPAGE_H

#include <QWidget>
#include <QList>
#include <QPropertyAnimation>

class DigitalCardWidget;

class DigitalHomePage : public QWidget
{
    Q_OBJECT

public:
    explicit DigitalHomePage(QWidget *parent = nullptr);
    ~DigitalHomePage();

signals:
    void toSettingPage();
    void toBatteryHomePage();
    void toFunctionPage(int channel);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void updateChannels();

private:
    void setupUI();
    void layoutCards();
    void createCard(int channel);
    void onCardClicked(int channel);
    void onCardPressAndHold(int channel);

    QList<DigitalCardWidget*> m_cards;
    QList<int> m_activeChannels;
    bool m_enclick{true};

    QPoint m_pressPos;
    bool m_swiping{false};
};

#endif // DIGITALHOMEPAGE_H
