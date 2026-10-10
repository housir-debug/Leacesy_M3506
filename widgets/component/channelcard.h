#pragma once
#include <QElapsedTimer>
#include <QStackedWidget>
#include "auxiliary/battery_model.h"

namespace Ui {class channelcard;}

class channelcard : public QStackedWidget
{
    Q_OBJECT

private:
    bool m_isPressed;
    QPoint m_pressPos;
    Ui::channelcard *ui;
    QTimer *m_longPressTimer;
    bool m_longPressTriggered;

signals:
    void longPressed(quint8 ch);
    void clicked(quint8 ch,bool status);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public:
    explicit channelcard(QWidget *parent = nullptr);
    ~channelcard();

    /* digital */
    std::atomic<float> m_voltage{0.0f};
    void setVoltage(float value);
    std::atomic<float> m_current{0.0f};
    void setCurrent(float value);
    QString getChunit() const;
    void setCurrentUnit(const QString &unit);
    bool getChCVChecked() const;
    void setCVChecked(bool checked);
    std::atomic<float> m_cv{0.0f};
    void setCvValue(float value);
    bool getChCCChecked() const;
    void setCCChecked(bool checked);
    std::atomic<float> m_cc{1.0f};
    void setCcValue(float value);
    bool getChOVPChecked() const;
    void setOVPChecked(bool checked);
    std::atomic<float> m_ovp{8.0f};
    void setOvpValue(float value);

    /* battery */
    std::atomic<float> m_soc{72.0f};
    void setSocValue(float value);
    std::atomic<float> m_ocv{0.0f};
    void setOcvValue(float value);
    std::atomic<float> m_cap{0.0f};
    void setCapValue(float value);
    std::atomic<float> m_esr{0.0f};
    void setEsrValue(float value);
    QString getChmodelname() const;
    void setModelValue(const QString &model);

    int m_modelindex{0};
    QElapsedTimer m_integralTimer;
    QSharedPointer<BatteryModel> m_activeModel;
    void updateSocValue(float current);
    float getcurrentOCV() const;
    float getcurrentESR() const;

    /* status */
    void setChtatus(const QString &status);

    /* version */
    void setChSWVersion(const QString &ver);
    void setChHWVersion(const QString &ver);

    /* share */
    QString m_curange{""};
    // 0 -0x01-mA; 1 -0x10-Auto; 2 -0x00-A
    std::atomic<quint8> m_channel{0};
    void setChannel(int ch);
    bool getChstatus() const;
    void setChannelState(bool state);
    int getChmode();
    void setChannelmode(int index);
};

namespace ChCardindex {
    constexpr int Digital      = 0x00;
    constexpr int Battery      = 0x01;
    constexpr int Status       = 0x02;
    constexpr int Version      = 0x03;
}
