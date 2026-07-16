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
    float getChVoltage() const;
    float getChCurrent() const;
    QString getChunit() const;
    float getChCvValue() const;
    bool getChCVChecked() const;
    float getChCcValue() const;
    bool getChCCChecked() const;
    float getChOvpValue() const;
    bool getChOVPChecked() const;

    void setChannel(quint8 ch);
    void setChannelState(bool state);
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

    bool m_isPressed;
    QPoint m_pressPos;

    QTimer *m_longPressTimer;
    bool m_longPressTriggered;

    std::atomic<quint8> m_channel{0};
    std::atomic<quint8> m_range{0};
    std::atomic<float> m_voltage{0.0f};
    std::atomic<float> m_current{0.0f};
    std::atomic<float> m_cv{0.0f};
    std::atomic<float> m_cc{1.0f};
    std::atomic<float> m_ovp{8.0f};

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

