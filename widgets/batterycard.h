#pragma once
#include <QFrame>
#include <QElapsedTimer>
#include "auxiliary/battery_model.h"

namespace Ui {class batterycard;}

class batterycard : public QFrame
{
    Q_OBJECT

private:
    Ui::batterycard *ui;

    bool m_isPressed;
    QPoint m_pressPos;
    QTimer* m_longPressTimer;
    bool m_longPressTriggered;

    int m_channel{0};
    float m_soc{72.0f};
    float m_ocv{0.0f};
    float m_esr{0.0f};
    float m_cap{1.0f};
    QString m_model{""};
    quint8 m_modelindex{0};

    QElapsedTimer m_integralTimer;
    QSharedPointer<BatteryModel> m_activeModel;

public:
    explicit batterycard(QWidget *parent = nullptr);
    ~batterycard();

    QString getChmodelname() const;
    quint8 getChmodelindex() const;
    bool getcurrentisOver() const;
    float getcurrentOCV() const;
    float getcurrentESR() const;
    float getChSOCvalue() const;
    float getChOcvvalue() const;
    float getChEsrvalue() const;
    float getChCapvalue() const;
    bool getChstatus() const;

    void startupdateValue();
    void updateSocValue(float current);

    void setChannel(int ch);
    void setSocValue(float value);
    void setOcvValue(float value);
    void setCapValue(float value);
    void setEsrValue(float value);
    void setChannelState(bool state);
    void setModelValue(const QString &model);
    void setModel(quint8 index,const QSharedPointer<BatteryModel> &model);

signals:
    void longPressed(quint8 ch);
    void clicked(quint8 ch,bool status);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};
