#include "digitalcard.h"
#include "ui_digitalcard.h"
#include <QMouseEvent>
#include <QTimer>


digitalcard::digitalcard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::digitalCard)
{
    ui->setupUi(this);

    m_scaleAnimation = new QPropertyAnimation(this, "geometry");
    m_scaleAnimation->setDuration(110);
    m_scaleAnimation->setEasingCurve(QEasingCurve::OutQuad);

    QRect currentGeo = geometry();
    m_scaleAnimation->setStartValue(currentGeo);

    int shrink = 3;
    QRect targetGeo = currentGeo;
    targetGeo.setX(currentGeo.x() + shrink);
    targetGeo.setY(currentGeo.y() + shrink);
    targetGeo.setWidth(currentGeo.width() - shrink * 2);
    targetGeo.setHeight(currentGeo.height() - shrink * 2);
    m_scaleAnimation->setEndValue(targetGeo);

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
    m_scaleAnimation->start();

    m_longPressTriggered = false;
    m_longPressTimer->start();

    // Marked as processed
    event->accept();

    // Pass to the parent class
    QWidget::mousePressEvent(event);
}

void digitalcard::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPressed){
        m_isPressed = false;
        m_longPressTimer->stop();

        if (!m_longPressTriggered) {
            QPoint delta = event->pos() - m_pressPos;
            if (delta.manhattanLength() < 18) {
                emit clicked();
            }
        }
    }

    // Marked as processed
    event->accept();

    // Pass to the parent class
    QWidget::mouseReleaseEvent(event);
}

void digitalcard::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed) {
        QPoint delta = event->pos() - m_pressPos;
        if (delta.manhattanLength() > 18) {
            m_longPressTimer->stop();
        }
    }

    // Pass to the parent class
    QWidget::mouseMoveEvent(event);
}

// property setting API

void digitalcard::setChannelState(bool state)
{
    setProperty("outputed",state);
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
    QFont font = ui->currentunitlabel->font();
    font.setPointSize(unit.size()==1 ? 27:18);
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
