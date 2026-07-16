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
    float getChSOCvalue() const;
    float getChOcvvalue() const;
    float getChEsrvalue() const;
    float getChCapvalue() const;
    QString getChmodelname() const;
    quint8 getChmodelindex() const;

    void setChannel(quint8 ch);
    void setChannelState(bool state);

    void setSocValue(float value);
    void setOcvValue(float value);
    void setCapValue(float value);
    void setEsrValue(float value);
    void setModelValue(const QString &model);
    void setModel(quint8 index,const QSharedPointer<BatteryModel> &model);

    void startupdateValue();
    void updateSocValue(float current);

    float getcurrentOCV();
    float getcurrentESR();
    bool getcurrentisOver();

private:
    Ui::batterycard *ui;

    bool m_isPressed;
    QPoint m_pressPos;

    QTimer *m_longPressTimer;
    bool m_longPressTriggered;

    quint8 m_channel{0};
    float m_soc{72.0f};
    float m_ocv{0.0f};
    float m_esr{0.0f};
    float m_cap{1.0f};
    QString m_model{"empty"};

    QSharedPointer<BatteryModel> m_activeModel;
    QElapsedTimer m_integralTimer;
    quint8 m_modelindex{0};

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
