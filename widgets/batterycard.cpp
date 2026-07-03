#include "batterycard.h"
#include "ui_batterycard.h"
#include <QMouseEvent>
#include <QTimer>
#include <QStyle>
#include <QtDebug>

batterycard::batterycard(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::batterycard)
{
    ui->setupUi(this);

    m_longPressTimer = new QTimer(this);
    m_longPressTimer->setSingleShot(true);
    m_longPressTimer->setInterval(900);   // 900 ms

    connect(m_longPressTimer, &QTimer::timeout, this, [this]{
        if (m_isPressed && !m_longPressTriggered) {
            m_longPressTriggered = true;
            emit longPressed();
        }
    });
}

batterycard::~batterycard()
{
    delete ui;
}

// mouse event procressing

void batterycard::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    m_pressPos = event->pos();

    m_longPressTriggered = false;
    m_longPressTimer->start();

    // event->accept();
    // Pass to the parent class
    QWidget::mousePressEvent(event);
}

void batterycard::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPressed){
        m_longPressTimer->stop();

        if (!m_longPressTriggered) {
            QPoint delta = event->pos() - m_pressPos;
            if (delta.manhattanLength() < 18) {
                qDebug()<<"触发打印";
                bool status = property("outputed").toBool();
                emit clicked(!status);
            }
        }
    }

    m_isPressed = false;

    // event->accept();
    // Pass to the parent class
    QWidget::mouseReleaseEvent(event);
}

void batterycard::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed) {
        QPoint delta = event->pos() - m_pressPos;
        if (delta.manhattanLength() > 18) {
            m_longPressTimer->stop();
            m_isPressed = false;
        }
    }

    // Pass to the parent class
    QWidget::mouseMoveEvent(event);
}

// property setting API

void batterycard::setChannelState(bool state)
{
    setProperty("outputed",state);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void batterycard::setChannelName(const QString &name)
{
    ui->channellabel->setText(name);
}

void batterycard::setSocValue(quint8 value)
{
    ui->socprogressBar->setValue(value);
}

void batterycard::setVocValue(float value)
{
    QString text = QString::number(value, 'f', 4) + " V";
    ui->ocvvaluelabel->setText(text);
}

void batterycard::setCapValue(float value)
{
    QString text = QString::number(value, 'f', 4) + " Ah";
    ui->capvaluelabel->setText(text);
}

void batterycard::setEsrValue(float value)
{
    QString text = QString::number(value, 'f', 4) + " Ω";
    ui->capvaluelabel->setText(text);
}

void batterycard::setModelValue(const QString &model)
{
    ui->modelnamelabel->setText(model);
}
