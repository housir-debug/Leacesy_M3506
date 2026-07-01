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

    void setChannelState(bool state);
    void setChannelName(const QString &name);
    QString getChannelName(){return m_ChannelName;}

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

    bool m_isPressed;
    QPoint m_pressPos;

    QTimer *m_longPressTimer;
    bool m_longPressTriggered;

    QString m_ChannelName;

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

