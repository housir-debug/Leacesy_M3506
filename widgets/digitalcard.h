#pragma once
#include <QFrame>

namespace Ui {
    class digitalcard; // = UI_digitalcard instead
}

class digitalcard : public QFrame
{
    Q_OBJECT

public:
    explicit digitalcard(QWidget *parent = nullptr);
    ~digitalcard();

    bool getChstatus() const;
    quint8 getChRange() const;

    void setChannel(quint8 ch);
    void setChannelState(bool state);
    void setChannelName(const QString &name);
    void setChannelRange(quint8 range);

    void setVoltage(float value);
    void setCurrent(float value);
    void setCurrentUnit(const QString &unit);

    void setCVChecked(bool checked);
    void setCvValue(float value);
    void setCCChecked(bool checked);
    void setCcValue(float value);
    void setOVPChecked(bool checked);
    void setOvpValue(float value);

private:
    Ui::digitalcard *ui;
    quint8 m_channel{0};
    quint8 m_range{0};

    bool m_isPressed;
    QPoint m_pressPos;

    QTimer *m_longPressTimer;
    bool m_longPressTriggered;

signals:
    void clicked(quint8 ch,bool status);
    void longPressed(quint8 ch);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    // void enterEvent(QEvent *event) override;
    // void leaveEvent(QEvent *event) override;
};

