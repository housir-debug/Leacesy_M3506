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

void digitalcard::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed && (event->pos() - m_pressPos).manhattanLength() > 18) {
        m_longPressTimer->stop();
        m_isPressed = false;
    }
}

void digitalcard::mousePressEvent(QMouseEvent *event)
{
    m_longPressTriggered = false;
    m_longPressTimer->start();

    m_pressPos = event->pos();
    m_isPressed = true;
    event->accept();
}

void digitalcard::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPressed && !m_longPressTriggered){
        bool status = property("outputed").toBool();
        emit clicked(m_channel,!status);
    }

    m_longPressTimer->stop();
    m_isPressed = false;
    event->accept();
}

// property get API

int digitalcard::getChRange() const{return m_range.load();}

bool digitalcard::getChstatus() const{return property("outputed").toBool();}

float digitalcard::getChVoltage() const{return m_voltage.load();}

float digitalcard::getChCurrent() const{return m_current.load();}

QString digitalcard::getChunit() const{return ui->currentunitlabel->text();}

float digitalcard::getChCvValue() const{return m_cv.load();}

bool digitalcard::getChCVChecked() const{return ui->cvcheckbox->isChecked();}

float digitalcard::getChCcValue() const{return m_cc.load();}

bool digitalcard::getChCCChecked() const{return ui->cccheckbox->isChecked();}

float digitalcard::getChOvpValue() const{return m_ovp.load();}

bool digitalcard::getChOVPChecked() const{return ui->ovpcheckbox->isChecked();}

// property setting API

void digitalcard::setChannelRange(int range){m_range.store(range);}// 0 -0x01-mA; 1 -0x10-Auto; 2 -0x00-A

void digitalcard::setChannel(int ch)
{
    QString name = QString("CH-%1").arg(ch, 2, 10, QChar('0'));
    ui->channellabel->setText(name);
    m_channel.store(ch);
}

void digitalcard::setChannelState(bool state)
{
    setProperty("outputed",state);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void digitalcard::setVoltage(float value)
{
    QString text = QString::number(value, 'f', 4);
    ui->voltagelabel->setText(text);
    m_voltage.store(value);
}

void digitalcard::setCurrent(float value)
{
    QString text = QString::number(value, 'f', 4);
    ui->currentlabel->setText(text);
    m_current.store(value);
}

void digitalcard::setCurrentUnit(const QString &unit)
{
    ui->currentunitlabel->setText(unit);
    ui->currentunitlabel->setProperty("mA",unit.size()==2);

    ui->currentunitlabel->style()->unpolish(ui->currentunitlabel);
    ui->currentunitlabel->style()->polish(ui->currentunitlabel);
    ui->currentunitlabel->update();
}

void digitalcard::setCVChecked(bool checked){ui->cvcheckbox->setChecked(checked);}

void digitalcard::setCvValue(float value)
{
    QString text = QString::number(value, 'f', 3) + " V";
    ui->cvvaluelabel->setText(text);
    m_cv.store(value);
}

void digitalcard::setCCChecked(bool checked){ui->cccheckbox->setChecked(checked);}

void digitalcard::setCcValue(float value)
{
    QString text = QString::number(value, 'f', 3) + " A";
    ui->ccvaluelabel->setText(text);
    m_cc.store(value);
}

void digitalcard::setOVPChecked(bool checked){ui->ovpcheckbox->setChecked(checked);}

void digitalcard::setOvpValue(float value)
{
    QString text = QString::number(value, 'f', 3) + " V";
    ui->ovpvaluelabel->setText(text);
    m_ovp.store(value);
}
