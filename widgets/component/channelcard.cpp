#include "auxiliary/battery_model.h"
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
        if (currentIndex() == ChCardindex::Battery || currentIndex() == ChCardindex::Digital){
            emit clicked(!property("Outputed").toBool());
        }
    }

    m_longPressTimer->stop();
    m_isPressed = false;
    event->accept();
}

channelcard::channelcard(QWidget *parent) : QStackedWidget(parent),
    ui(new Ui::channelcard)
{
    ui->setupUi(this);

    QStringList models = BatteryModelManager::instance().getAvailableModels();
    if (!models.isEmpty()){setModelValue(models[0]);}

    m_longPressTimer = new QTimer(this);
    m_longPressTimer->setSingleShot(true);
    m_longPressTimer->setInterval(900);   // 900 ms

    connect(m_longPressTimer, &QTimer::timeout, this, [this]{
        if (m_isPressed && !m_longPressTriggered) {
            m_longPressTriggered = true;
            // emit longPressed();
            setCurrentIndex(ChCardindex::Selected);
        }
    }, Qt::DirectConnection);
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

void channelcard::setCvValue(float value)
{
    ui->cvvalue_label->setText(QString::number(value, 'f', 3) + " V");
    m_cv.store(value);
}
void channelcard::setCcValue(float value)
{
    ui->ccvalue_label->setText(QString::number(value, 'f', 3) + " A");
    m_cc.store(value);
}
void channelcard::setOvpValue(float value)
{
    ui->ovpvalue_label->setText(QString::number(value, 'f', 3) + " V");
    m_ovp.store(value);
}

bool channelcard::getChCVChecked() const{return ui->cv_frame->property("Status").toBool();}
bool channelcard::getChCCChecked() const{return ui->cc_frame->property("Status").toBool();}
bool channelcard::getChOVPChecked() const{return ui->ovp_frame->property("Status").toBool();}

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
    return BatteryModelManager::instance().getOCV(getChmodelname(),m_soc.load());
}
float channelcard::getcurrentESR() const
{
    return BatteryModelManager::instance().getESR(getChmodelname(),m_soc.load());
}
bool channelcard::getcurrentIsOver() const
{
    return BatteryModelManager::instance().isOver(getChmodelname(),m_soc.load());
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
    ui->statusch_label->setText(channel);
    ui->versionch_label->setText(channel);
}
void channelcard::setChtatus(const QString &status)
{
    if (status.size() == 16){
        ui->cv_frame->setProperty("Status",status.at(14) == "1");
        ui->cv_frame->style()->unpolish(ui->cv_frame);
        ui->cv_frame->style()->polish(ui->cv_frame);
        ui->cc_frame->setProperty("Status",status.at(13) == "1");
        ui->cc_frame->style()->unpolish(ui->cc_frame);
        ui->cc_frame->style()->polish(ui->cc_frame);
        ui->ovp_frame->setProperty("Status",status.at(11) == "1");
        ui->ovp_frame->style()->unpolish(ui->ovp_frame);
        ui->ovp_frame->style()->polish(ui->ovp_frame);

        for (int i = 0; i < 16; ++i) {
            QString btnName = QString("pushButton%1").arg(i+1);
            QPushButton *btn = ui->statuschframe->findChild<QPushButton*>(btnName);

            btn->setProperty("status", status.at(i) == "1");
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
        }
    }
}

bool channelcard::getChstatus() const{return property("Outputed").toBool();}
void channelcard::setChannelState(bool state)
{
    m_integralTimer.restart();
    setProperty("Outputed",state);
    ui->digital->style()->unpolish(ui->digital);
    ui->digital->style()->polish(ui->digital);
    ui->battery->style()->unpolish(ui->battery);
    ui->battery->style()->polish(ui->battery);
}

int channelcard::getChmode(){return currentIndex();}
void channelcard::setChannelmode(int index){setCurrentIndex(index);}

/* slots */
void channelcard::on_digt_pushButton_clicked()
{
    setCurrentIndex(ChCardindex::Digital);

    if (currentIndex() == ChCardindex::Battery && getChstatus()){
        emit clicked(false);
    }
}
void channelcard::on_batt_pushButton_clicked()
{
    setCurrentIndex(ChCardindex::Battery);

    if (currentIndex() == ChCardindex::Digital && getChstatus()){
        emit clicked(false);
    }
}
void channelcard::on_stat_pushButton_clicked(){setCurrentIndex(ChCardindex::Status);}
void channelcard::on_vers_pushButton_clicked(){setCurrentIndex(ChCardindex::Version);}
