#pragma once
#include <QElapsedTimer>
#include <QStackedWidget>
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
    // void longPressed();
    void clicked(bool status);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public:
    explicit channelcard(QWidget *parent = nullptr);
    ~channelcard();

    /* digital */
    void setVoltage(float value);
    void setCurrent(float value);
    std::atomic<float> m_voltage{0.0f};
    std::atomic<float> m_current{0.0f};

    QString m_curange{""};
    QString getChunit() const;
    void setCurrentUnit(const QString &unit);

    void setCvValue(float value);
    void setCcValue(float value);
    void setOvpValue(float value);
    std::atomic<float> m_cv{0.0f};
    std::atomic<float> m_cc{1.0f};
    std::atomic<float> m_ovp{8.0f};

    bool getChCVChecked() const;
    bool getChCCChecked() const;
    bool getChOVPChecked() const;

    /* battery */
    void setSocValue(float value);
    void setOcvValue(float value);
    void setCapValue(float value);
    void setEsrValue(float value);
    std::atomic<float> m_soc{72.0f};
    std::atomic<float> m_ocv{0.0f};
    std::atomic<float> m_cap{0.0f};
    std::atomic<float> m_esr{0.0f};
    QString getChmodelname() const;
    void setModelValue(const QString &model);

    QElapsedTimer m_integralTimer;
    float getcurrentOCV() const;
    float getcurrentESR() const;
    bool getcurrentIsOver() const;
    void updateSocValue(float current);

    /* version */
    void setChSWVersion(const QString &ver);
    void setChHWVersion(const QString &ver);

    /* share */
    void setChannel(int ch);
    void setChtatus(const QString &status);
    bool getChstatus() const;
    void setChannelState(bool state);
    int getChmode();
    void setChannelmode(int index);
    std::atomic<quint8> m_channel{0};

private slots:
    void on_stat_pushButton_clicked();
};

namespace ChCardindex {
    constexpr int Digital      = 0x00;
    constexpr int Battery      = 0x01;
    constexpr int Status       = 0x02;
    constexpr int Version      = 0x03;
}
