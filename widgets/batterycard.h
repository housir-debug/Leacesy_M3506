#pragma once

#include <QFrame>

namespace Ui {
    class batterycard; // = UI_digitalcard instead
}

class batterycard : public QFrame
{
    Q_OBJECT

public:
    explicit batterycard(QWidget *parent = nullptr);
    ~batterycard();

    void setChannelState(bool state);
    void setChannelName(const QString &name);

    void setSocValue(quint8 value);
    void setVocValue(float value);
    void setCapValue(float value);
    void setEsrValue(float value);
    void setModelValue(const QString &model);

private:
    Ui::batterycard *ui;

    bool m_isPressed;
    QPoint m_pressPos;

    QTimer *m_longPressTimer;
    bool m_longPressTriggered;

signals:
    void clicked(bool status);
    void longPressed();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    // void enterEvent(QEvent *event) override;
    // void leaveEvent(QEvent *event) override;
};
