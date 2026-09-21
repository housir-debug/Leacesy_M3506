#include "batterycard.h"
#include "ui_batterycard.h"
#include <QMouseEvent>
#include <QTimer>
#include <QStyle>
#include <cmath>

batterycard::batterycard(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::batterycard)
{
    ui->setupUi(this);

    m_longPressTimer = new QTimer(this);
    m_longPressTimer->setSingleShot(true);
    m_longPressTimer->setInterval(900); // 900 ms

    connect(m_longPressTimer, &QTimer::timeout, this, [this]{
        if (m_isPressed && !m_longPressTriggered) {
            m_longPressTriggered = true;
            emit longPressed(m_channel);
        }
    });
}

batterycard::~batterycard()
{
    delete ui;
}

// mouse event procressing

void batterycard::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed && (event->pos() - m_pressPos).manhattanLength() > 18) {
        m_longPressTimer->stop();
        m_isPressed = false;
    }
}

void batterycard::mousePressEvent(QMouseEvent *event)
{
    m_longPressTriggered = false;
    m_longPressTimer->start();

    m_pressPos = event->pos();
    m_isPressed = true;
    event->accept();
}

void batterycard::mouseReleaseEvent(QMouseEvent *event)
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

QString batterycard::getChmodelname() const{return m_model;}

quint8 batterycard::getChmodelindex() const{return m_modelindex;}

bool batterycard::getcurrentisOver() const{
    if (!m_activeModel.isNull()){
        return m_activeModel->isOver(m_soc);
    }else{
        return false;
    }
}

float batterycard::getcurrentOCV() const{
    if (!m_activeModel.isNull()){
        return m_activeModel->getOCV(m_soc);
    }else{
        return 0.0f;
    }
}

float batterycard::getcurrentESR() const{
    if (!m_activeModel.isNull()){
        return m_activeModel->getESR(m_soc);
    }else{
        return 0.0f;
    }
}

float batterycard::getChSOCvalue() const{return m_soc;}

float batterycard::getChOcvvalue() const{return m_ocv;}

float batterycard::getChEsrvalue() const{return m_esr;}

float batterycard::getChCapvalue() const{return m_cap;}

bool batterycard::getChstatus() const{return property("outputed").toBool();}

// auto SOC update API

void batterycard::startupdateValue(){m_integralTimer.restart();}

void batterycard::updateSocValue(float current)
{
    float deltaTimeHours = m_integralTimer.elapsed() / 3600000.0f; // ms -> hours
    float deltaSOC = (current * deltaTimeHours * 100) / m_cap;

    if (deltaSOC < 0 && qAbs(deltaSOC) >= m_soc){
        setSocValue(0.0f);
    }else if(deltaSOC > 0 && qAbs(deltaSOC) >= (100.0f - m_soc)){
        setSocValue(100.0f);
    }else{
        m_soc += deltaSOC;
        setSocValue(m_soc);
    }

    m_integralTimer.restart();
}

// property setting API

void batterycard::setChannel(int ch)
{
    QString name = QString("CH-%1").arg(ch, 2, 10, QChar('0'));
    ui->channellabel->setText(name);
    m_channel = ch;
}

void batterycard::setSocValue(float value)
{
    m_soc = value;
    int soc =  qRound(value);
    ui->socprogressBar->setValue(soc);
}

void batterycard::setOcvValue(float value)
{
    QString text = QString::number(value, 'f', 4) + " V";
    ui->ocvvaluelabel->setText(text);
    m_ocv = value;
}

void batterycard::setCapValue(float value)
{
    QString text = QString::number(value, 'f', 2) + " Ah";
    ui->capvaluelabel->setText(text);
    m_cap = value;
}

void batterycard::setEsrValue(float value)
{
    QString text = QString::number(value, 'f', 4) + " Ω";
    ui->esrvaluelabel->setText(text);
    m_esr = value;
}

void batterycard::setChannelState(bool state)
{
    setProperty("outputed",state);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void batterycard::setModelValue(const QString &model)
{
    ui->modelnamelabel->setText(model);
    m_model = model;
}

void batterycard::setModel(quint8 index,const QSharedPointer<BatteryModel> &model)
{
    m_modelindex = index;
    m_activeModel = model;
}

