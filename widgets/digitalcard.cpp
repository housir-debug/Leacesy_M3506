#include "digitalcard.h"
#include "ui_digitalcard.h"
#include <QMouseEvent>
#include <QTimer>
#include <QStyle>
#include <QtDebug>


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
            emit longPressed();
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
                qDebug()<<"触发打印";
                emit clicked();
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
    m_ChannelName = name;
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
    QFont font = ui->currentunitlabel->font(); // get source all property
    font.setPixelSize(unit.size()==1 ? 36 : 18);
    ui->currentunitlabel->setFont(font);
    ui->currentunitlabel->setText(unit);
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
