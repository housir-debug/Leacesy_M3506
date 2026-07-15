#pragma once
#include <QFrame>
#include <QElapsedTimer>
#include "auxiliary/battery_model.h"

namespace Ui {
    class batterycard; // = UI_digitalcard instead
}

class batterycard : public QFrame
{
    Q_OBJECT

public:
    explicit batterycard(QWidget *parent = nullptr);
    ~batterycard();

    bool getChstatus() const;
    quint8 getChmodel() const;

    void setChannel(quint8 ch);
    void setChannelState(bool state);

    void setSocValue(float value);
    void setOcvValue(float value);
    void setCapValue(float value);
    void setEsrValue(float value);
    void setModelValue(const QString &model);
    void setModel(quint8 index,const QSharedPointer<BatteryModel> &model);

    void updateSocValue(float current);
    void startupdateValue();

    float getcurrentOCV();
    float getcurrentESR();
    bool getcurrentisOver();

private:
    Ui::batterycard *ui;
    quint8 m_channel{0};
    quint8 m_modelindex{0};

    float m_capacityAH{2.4f};
    float m_progressSOC{0.0f};
    QElapsedTimer m_integralTimer;
    //QString m_batteryModel{"empty"};
    QSharedPointer<BatteryModel> m_activeModel;

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
