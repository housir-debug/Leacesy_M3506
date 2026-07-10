#include "digitalcard.h"
#include "ui_digitalcard.h"
#include <QMouseEvent>
#include <QTimer>
#include <QStyle>

digitalcard::digitalcard(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::digitalcard) // objectname
{
    ui->setupUi(this);

    m_longPressTimer = new QTimer(this);
    m_longPressTimer->setSingleShot(true);
    m_longPressTimer->setInterval(900);   // 900 ms

    connect(m_longPressTimer, &QTimer::timeout, this, [this]{
        if (m_isPressed && !m_longPressTriggered) {
            m_longPressTriggered = true;
            emit longPressed(m_channel);
        }
    });
}

digitalcard::~digitalcard()
{
    delete ui;
}

// mouse event procressing

void digitalcard::mousePressEvent(QMouseEvent *event)
{
    m_isPressed = true;
    m_pressPos = event->pos();

    m_longPressTriggered = false;
    m_longPressTimer->start();

    // event->accept();
    // Pass to the parent class
    QWidget::mousePressEvent(event);
}

void digitalcard::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPressed){
        m_longPressTimer->stop();

        if (!m_longPressTriggered) {
            QPoint delta = event->pos() - m_pressPos;
            if (delta.manhattanLength() < 18) {
                bool status = property("outputed").toBool();
                emit clicked(m_channel,!status);
            }
        }
    }

    m_isPressed = false;

    // event->accept();
    // Pass to the parent class
    QWidget::mouseReleaseEvent(event);
}

void digitalcard::mouseMoveEvent(QMouseEvent *event)
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

bool digitalcard::getChstatus() const
{
    return property("outputed").toBool();
}

quint8 digitalcard::getChRange() const
{
    return m_range;
}


void digitalcard::setChannel(quint8 ch)
{
    m_channel = ch;
}

void digitalcard::setChannelState(bool state)
{
    setProperty("outputed",state);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void digitalcard::setChannelName(const QString &name)
{
    ui->channellabel->setText(name);
}

void digitalcard::setVoltage(float value)
{
    QString text = QString::number(value, 'f', 4);
    ui->voltagelabel->setText(text);
}

void digitalcard::setCurrent(float value)
{
    QString text = QString::number(value, 'f', 4);
    ui->currentlabel->setText(text);
}

void digitalcard::setCurrentUnit(const QString &unit)
{
    bool state = unit.size()==2;
    ui->currentunitlabel->setProperty("mA",state);
    ui->currentunitlabel->setText(unit);

    ui->currentunitlabel->style()->unpolish(ui->currentunitlabel);
    ui->currentunitlabel->style()->polish(ui->currentunitlabel);
    ui->currentunitlabel->update();
}

void digitalcard::setChannelRange(quint8 range)
{
    // 0 -0x01-mA; 1 -0x10-Auto; 2 -0x00-A
    m_range = range;
}

void digitalcard::setCVChecked(bool checked)
{
    ui->cvcheckbox->setChecked(checked);
}

void digitalcard::setCvValue(float value)
{
    QString text = QString::number(value, 'f', 3) + " V";
    ui->cvvaluelabel->setText(text);
}

void digitalcard::setCCChecked(bool checked)
{
    ui->cccheckbox->setChecked(checked);
}

void digitalcard::setCcValue(float value)
{
    QString text = QString::number(value, 'f', 3) + " A";
    ui->ccvaluelabel->setText(text);
}

void digitalcard::setOVPChecked(bool checked)
{
    ui->ovpcheckbox->setChecked(checked);
}

void digitalcard::setOvpValue(float value)
{
    QString text = QString::number(value, 'f', 3) + " V";
    ui->ovpvaluelabel->setText(text);
}
