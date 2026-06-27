// DigitalCardWidget.h
#ifndef DIGITALCARDWIDGET_H
#define DIGITALCARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class DigitalCardWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale) // for animation

public:
    explicit DigitalCardWidget(QWidget *parent = nullptr);
    ~DigitalCardWidget();

    void setScale(qreal scale) ;
    qreal scale() const { return m_scale; }

    void setEnabled(bool enabled);
    void setChannelOutput(bool output);
    void setChannelName(const QString &name);
    void setVoltage(double voltage, const QString &unit = "V");
    void setCurrent(double current, const QString &unit = "A");
    void setCVSetpoint(double value);
    void setCVMode(bool enabled);
    void setCCSetpoint(double value);
    void setCCMode(bool enabled);
    void setOVPSetpoint(double value);
    void setOVPMode(bool enabled);

signals:
    void clicked();
    void pressAndHold();

protected:
    void paintEvent(QPaintEvent *event) override;
    //void resizeEvent(QResizeEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void setupUI();
    void updateStyles();
    QHBoxLayout* createSetpointRow(const QString &label, const QString &color,
                                   QLabel **valueLabel, QWidget **indicator);

    bool m_channelOutput{false};
    QWidget *m_cardWidget;

    QLabel *m_channelLabel;
    QLabel *m_dotLabel;
    QLabel *m_voltageLabel;
    QLabel *m_currentLabel;
    QLabel *m_currentUnitLabel;

    QLabel *m_cvLabel;
    QLabel *m_cvValueLabel;
    QWidget *m_cvIndicator;
    QLabel *m_ccLabel;
    QLabel *m_ccValueLabel;
    QWidget *m_ccIndicator;
    QLabel *m_ovpLabel;
    QLabel *m_ovpValueLabel;
    QWidget *m_ovpIndicator;

    QPropertyAnimation *m_releaseAnimation;
    QPropertyAnimation *m_pressAnimation;
    QTimer *m_longPressTimer;
    QPoint m_pressPos;

    qreal m_scale{1.0};
    bool m_pressed{false};
    bool m_longPressed{false};
};

#endif // DIGITALCARDWIDGET_H
