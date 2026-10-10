#include "ui_channelcard.h"
#include "channelcard.h"
#include <QMouseEvent>
#include <QTimer>
#include <QStyle>

// mouse event procressing

void channelcard::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed && (event->pos() - m_pressPos).manhattanLength() > 18) {
        m_longPressTimer->stop();
        m_isPressed = false;
    }
}

void channelcard::mousePressEvent(QMouseEvent *event)
{
    m_longPressTriggered = false;
    m_longPressTimer->start();

    m_pressPos = event->pos();
    m_isPressed = true;
    event->accept();
}

void channelcard::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPressed && !m_longPressTriggered){
        bool status = property("outputed").toBool();
        emit clicked(m_channel,!status);
    }

    m_longPressTimer->stop();
    m_isPressed = false;
    event->accept();
}

channelcard::channelcard(QWidget *parent) : QStackedWidget(parent),
    ui(new Ui::channelcard) // objectname
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

channelcard::~channelcard()
{
    delete ui;
}

/* digital */

void channelcard::setVoltage(float value)
{
    ui->volt_label->setText(QString::number(value, 'f', 4));
    m_voltage.store(value);
}

void channelcard::setCurrent(float value)
{
    ui->curr_label->setText(QString::number(value, 'f', 4));
    m_current.store(value);
}

QString channelcard::getChunit() const{return ui->currunit_label->text();}

void channelcard::setCurrentUnit(const QString &unit){ui->currunit_label->setText(unit);}

bool channelcard::getChCVChecked() const{return ui->cv_frame->property("Status").toBool();}

void channelcard::setCVChecked(bool checked)
{
    ui->cv_frame->setProperty("Status",checked);
    ui->cv_frame->style()->unpolish(this);
    ui->cv_frame->style()->polish(this);
}

void channelcard::setCvValue(float value)
{
    ui->cvvalue_label->setText(QString::number(value, 'f', 3) + " V");
    m_cv.store(value);
}

bool channelcard::getChCCChecked() const{return ui->cc_frame->property("Status").toBool();}

void channelcard::setCCChecked(bool checked)
{
    ui->cc_frame->setProperty("Status",checked);
    ui->cc_frame->style()->unpolish(this);
    ui->cc_frame->style()->polish(this);
}

void channelcard::setCcValue(float value)
{
    ui->ccvalue_label->setText(QString::number(value, 'f', 3) + " A");
    m_cc.store(value);
}

bool channelcard::getChOVPChecked() const{return ui->ovp_frame->property("Status").toBool();}

void channelcard::setOVPChecked(bool checked)
{
    ui->ovp_frame->setProperty("Status",checked);
    ui->ovp_frame->style()->unpolish(this);
    ui->ovp_frame->style()->polish(this);
}

void channelcard::setOvpValue(float value)
{
    ui->ovpvalue_label->setText(QString::number(value, 'f', 3) + " V");
    m_ovp.store(value);
}

/* battery */

void channelcard::setSocValue(float value)
{
    ui->socprogressBar->setValue(qRound(value));
    m_soc = value;

    if (60 < value && value <= 100){
        ui->socprogressBar->setProperty("Power","high");
    }else if (20 < value && value <=60) {
        ui->socprogressBar->setProperty("Power","mid");
    }else {
        ui->socprogressBar->setProperty("Power","low");
    }

    ui->socprogressBar->style()->unpolish(this);
    ui->socprogressBar->style()->polish(this);
    // update();
}

void channelcard::setOcvValue(float value)
{
    ui->ocvvaluelabel->setText(QString::number(value, 'f', 4) + " V");
    m_ocv = value;
}

void channelcard::setCapValue(float value)
{
    ui->capvaluelabel->setText(QString::number(value, 'f', 2) + " Ah");
    m_cap = value;
}

void channelcard::setEsrValue(float value)
{
    ui->esrvaluelabel->setText(QString::number(value, 'f', 4) + " Ω");
    m_esr = value;
}

QString channelcard::getChmodelname() const{return ui->modelnamelabel->text();}

void channelcard::setModelValue(const QString &model){ui->modelnamelabel->setText(model);}

float channelcard::getcurrentOCV() const
{
    if (!m_activeModel.isNull() && !m_activeModel->isOver(m_soc.load())){
        return m_activeModel->getOCV(m_soc.load());
    }else{
        return 0.0f;
    }
}

float channelcard::getcurrentESR() const
{
    if (!m_activeModel.isNull() && !m_activeModel->isOver(m_soc.load())){
        return m_activeModel->getESR(m_soc.load());
    }else{
        return 0.0f;
    }
}

void channelcard::updateSocValue(float current)
{
    float deltaTimeHours = m_integralTimer.elapsed() / 3600000.0f; // ms -> hours
    float deltaSOC = (current * deltaTimeHours * 100) / m_cap.load();

    if (deltaSOC < 0 && qAbs(deltaSOC) >= m_soc.load()){
        setSocValue(0.0f);
    }else if(deltaSOC > 0 && qAbs(deltaSOC) >= (100.0f - m_soc.load())){
        setSocValue(100.0f);
    }else{
        setSocValue(m_soc + deltaSOC);
    }

    m_integralTimer.restart();
}

/* status */

void channelcard::setChtatus(const QString &status)
{
    if (status.size() == 16){
        for (int i = 0; i < 16; ++i) {
            QString btnName = QString("pushButton%1").arg(i+1);
            QPushButton *btn = ui->statuschframe->findChild<QPushButton*>(btnName);

            bool statuschar  = status.at(i) == "1";
            btn->setProperty("status", statuschar);

            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
            //btn->update();
        }
    }
}

/* version */

void channelcard::setChSWVersion(const QString &ver){ui->chswvaluelabel->setText(ver);}

void channelcard::setChHWVersion(const QString &ver){ui->chhwvaluelabel->setText(ver);}

/* share */

void channelcard::setChannel(int ch)
{
    QString channel = QString("CH-%1").arg(ch, 2, 10, QChar('0'));
    m_channel.store(ch);

    ui->digitalch_label->setText(channel);
    ui->batterych_label->setText(channel);
    ui->versionch_label->setText(channel);
}

bool channelcard::getChstatus() const{return property("Outputed").toBool();}

void channelcard::setChannelState(bool state)
{
    m_integralTimer.restart();

    setProperty("Outputed",state);
    style()->unpolish(this);
    style()->polish(this);
    // update();
}

int channelcard::getChmode(){return currentIndex();}

void channelcard::setChannelmode(int index){setCurrentIndex(index);}
