#pragma once
#include <QFrame>

namespace Ui {class digitalcard;}

class digitalcard : public QFrame
{
    Q_OBJECT

private:
    Ui::digitalcard *ui;

    bool m_isPressed;
    QPoint m_pressPos;
    QTimer *m_longPressTimer;
    bool m_longPressTriggered;

    std::atomic<int> m_range{0};
    std::atomic<int> m_channel{0};
    std::atomic<float> m_voltage{0.0f};
    std::atomic<float> m_current{0.0f};
    std::atomic<float> m_cv{0.0f};
    std::atomic<float> m_cc{1.0f};
    std::atomic<float> m_ovp{8.0f};

public:
    explicit digitalcard(QWidget *parent = nullptr);
    ~digitalcard();

    int getChRange() const;
    bool getChstatus() const;
    float getChVoltage() const;
    float getChCurrent() const;
    QString getChunit() const;
    float getChCvValue() const;
    bool getChCVChecked() const;
    float getChCcValue() const;
    bool getChCCChecked() const;
    float getChOvpValue() const;
    bool getChOVPChecked() const;

    void setChannelRange(int range);
    void setChannel(int ch);
    void setChannelState(bool state);
    void setVoltage(float value);
    void setCurrent(float value);
    void setCurrentUnit(const QString &unit);
    void setCVChecked(bool checked);
    void setCvValue(float value);
    void setCCChecked(bool checked);
    void setCcValue(float value);
    void setOVPChecked(bool checked);
    void setOvpValue(float value);

signals:
    void longPressed(quint8 ch);
    void clicked(quint8 ch,bool status);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

