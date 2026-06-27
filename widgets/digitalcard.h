#pragma once
#include <QWidget>
#include <QPropertyAnimation>

namespace Ui {
    class digitalCard;
}

class digitalcard : public QWidget
{
    Q_OBJECT

public:
    explicit digitalcard(QWidget *parent = nullptr);
    ~digitalcard();

    void setChannelState(bool state);
    void setChannelName(const QString &name);

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
    Ui::digitalCard *ui;
    QPropertyAnimation *m_scaleAnimation;

    bool m_isPressed;
    QPoint m_pressPos;

    QTimer *m_longPressTimer;
    bool m_longPressTriggered;

signals:
    void clicked();
    void longPressed();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    // void enterEvent(QEvent *event) override;
    // void leaveEvent(QEvent *event) override;
};

