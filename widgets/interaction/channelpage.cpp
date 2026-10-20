#include "../component/channelcard.h"
#include "auxiliary/battery_model.h"
#include "channel/uart_protocol.h"
#include "auxiliary/signal_bus.h"
#include "ui_channelpage.h"
#include "channelpage.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QtEndian>

/* slots function */
void channelpage::on_dig_pushButton_clicked()
{
    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChmode() == ChCardindex::Battery && card->getChstatus()){
            ChannelBus::instance().to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "",false);
        }

        card->setChannelmode(ChCardindex::Digital);
    }
}
void channelpage::on_batt_pushButton_clicked()
{
    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChmode() == ChCardindex::Digital && card->getChstatus()){
            ChannelBus::instance().to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "",false);
        }

        card->setChannelmode(ChCardindex::Battery);
    }
}
void channelpage::on_stat_pushButton_clicked()
{
    for (channelcard *card :  qAsConst(m_channels)) {
        card->setChannelmode(ChCardindex::Status);
    }
}
void channelpage::on_ver_pushButton_clicked()
{
    for (channelcard *card :  qAsConst(m_channels)) {
        card->setChannelmode(ChCardindex::Version);
    }
}

void channelpage::on_setDe_pushButton_clicked(){emit jump_devicesetpage();}
void channelpage::on_setCh_pushButton_clicked(){emit jump_chsetpage();}

void channelpage::on_pre_pushButton_clicked(){to_channelpage(ui->ch_stackedWidget->currentIndex() - 1);}
void channelpage::on_next_pushButton_clicked(){to_channelpage(ui->ch_stackedWidget->currentIndex() + 1);}
void channelpage::to_channelpage(int newindex)
{
    int total = ui->ch_stackedWidget->count();
    ui->ch_stackedWidget->setCurrentIndex(newindex);

    ui->progressBar->setValue(newindex + 1);
    ui->progressBar->setMaximum(total);

    ui->pre_pushButton->setEnabled(newindex > 0);
    ui->next_pushButton->setEnabled(newindex < total - 1);
}

channelpage::channelpage(QWidget *parent) :QWidget(parent),
    ui(new Ui::channelpage)
{
    ui->setupUi(this);
}
channelpage::~channelpage()
{
    delete ui;
}

/* create channel card */
void channelpage::update_SoftVer(int ch,const QString &ver)
{
   channelcard* card = create_chcard(ch);
   card->setChSWVersion(ver);
}
void channelpage::update_HardVer(int ch,const QString &ver)
{
    channelcard* card = create_chcard(ch);
    card->setChHWVersion(ver);
}
channelcard* channelpage::create_chcard(int ch)
{
    auto itcard = m_channels.find(ch);
    if (itcard == m_channels.end()) {
        QWidget* page = nullptr;
        QHBoxLayout* layout = nullptr;
        int neededPages = (m_channels.size() + ChPageCard::PageCard) / ChPageCard::PageCard;// Round up

        if (ui->ch_stackedWidget->count() < neededPages) {
            page = new QWidget(ui->ch_stackedWidget);
            ui->ch_stackedWidget->addWidget(page);
            ui->ch_stackedWidget->setCurrentIndex(0);

            layout = new QHBoxLayout(page);
            layout->setContentsMargins(ChPageCard::RLMargin,ChPageCard::TBMargin,ChPageCard::RLMargin,ChPageCard::TBMargin);
            layout->setSpacing(ChPageCard::CardSpace);
            page->setLayout(layout);

            ui->progressBar->setValue(1);
            ui->progressBar->setMaximum(neededPages);
            ui->pre_pushButton->setEnabled(false);
            ui->next_pushButton->setEnabled(0 < neededPages - 1);
        }else{
            page = ui->ch_stackedWidget->widget(neededPages - 1); // index
            layout = qobject_cast<QHBoxLayout*>(page->layout());
        }

         channelcard* card = new channelcard(page);
         layout->addWidget(card);
         card->setChannel(ch);

         connect(card, &channelcard::clicked, this, [card](bool switchs) {
             quint8 func = switchs ? JV6xxx::Output::SET_OUTPSTATUS_ON : JV6xxx::Output::SET_OUTPSTATUS_OFF;
             ChannelBus::instance().to_Channel(card->m_channel,JV6xxx::OUTPUT, func, "",false);
             });
         //connect(card, &channelcard::longPressed, this, [this]() {emit jump_chsetpage();});

         m_channels[ch] = card;
         return card;
     }

    return itcard.value();
}

/* update card style */
void channelpage::update_IsOutput(int ch,bool status)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setChannelState(status);
    }
}
void channelpage::update_Voltage(int ch,float voltage)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        if (itcard.value()->getChmode() == ChCardindex::Digital){
            itcard.value()->setVoltage(voltage);
        }else if (itcard.value()->getChmode() == ChCardindex::Battery && itcard.value()->getChstatus()){
            if (!itcard.value()->getcurrentIsOver()){
                QByteArray data(4,0);

                float ocvval = itcard.value()->getcurrentOCV();
                qToBigEndian(ocvval, reinterpret_cast<uint8_t*>(data.data()));
                ChannelBus::instance().to_Channel(ch,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURVOLT, data,false);

                float esrval = itcard.value()->getcurrentESR();
                qToBigEndian(esrval, reinterpret_cast<uint8_t*>(data.data()));
                ChannelBus::instance().to_Channel(ch,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURIMP, data,false);
                return;
            }

            ChannelBus::instance().to_Channel(itcard.value()->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "",false);
        }
    }
}
void channelpage::update_CurrentAndUnit(int ch,float current)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        if (itcard.value()->getChmode() == ChCardindex::Digital){
            QString mA;QString A;
            if (itcard.value()->m_curange == JV6xxx::Param::Auto){
                mA = "@m"; A = "@A";
            }else{
                mA = "mA"; A =  "A";
            }

            if ((qAbs(current) < 1e-4)){
                itcard.value()->setCurrent(current * 1000);
                itcard.value()->setCurrentUnit(mA);
            }else{
                itcard.value()->setCurrent(current);
                itcard.value()->setCurrentUnit(A);
            }
        }else if (itcard.value()->getChmode() == ChCardindex::Battery && itcard.value()->getChstatus()){
            itcard.value()->updateSocValue(current);
        }
    }
}
void channelpage::update_Range(int ch,int range)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->m_curange = range;
    }
}

void channelpage::update_Imp(int ch,float imp)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()){
        itcard.value()->setEsrValue(imp);
    }
}
void channelpage::update_Cv(int ch,float cv)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()){
        if (itcard.value()->getChmode() == ChCardindex::Digital){
            itcard.value()->setCvValue(cv);
        }else if(itcard.value()->getChmode() == ChCardindex::Battery){
            itcard.value()->setOcvValue(cv);
        }
    }
}
void channelpage::update_Cc(int ch,float cc)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setCcValue(cc);
    }
}
void channelpage::update_Ovp(int ch,float ovp)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setOvpValue(ovp);
    }
}
void channelpage::update_Status(int ch,quint16 status)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        QString binaryStr = QString("%1").arg(status, 16, 2, QLatin1Char('0'));
        itcard.value()->setChtatus(binaryStr);
    }
}
void channelpage::update_Soc(int ch,float soc)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setSocValue(soc);
    }
}
void channelpage::update_Cap(int ch,float cap)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setCapValue(cap);
    }
}
void channelpage::update_Model(int ch,const QString model)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setModelValue(model);
    }
}

/* web or main call function */
QJsonArray channelpage::getAllChannelsData()
{
    QJsonArray channels;

    for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
        QJsonObject channel;
        channel["channel"] = it.key();
        channel["isOutput"] = it.value()->getChstatus();
        channel["voltage"] = it.value()->m_voltage.load();
        channel["current"] = it.value()->m_current.load();
        channel["current_unit"] = it.value()->getChunit();
        channel["cvSetpoint"] = it.value()->m_cv.load();
        channel["cvstatus"] = it.value()->getChCVChecked();
        channel["ccSetpoint"] = it.value()->m_cc.load();
        channel["ccstatus"] = it.value()->getChCCChecked();
        channel["ovSetpoint"] = it.value()->m_ovp.load();
        channel["ovpstatus"] = it.value()->getChOVPChecked();
        channels.append(channel);
    }

    return channels;
}
bool channelpage::existBatteryOutPut()
{
     for (auto itcard = m_channels.begin(); itcard != m_channels.end(); ++itcard) {
         if (itcard.value()->getChmode() == ChCardindex::Battery && itcard.value()->getChstatus()){
             return true;
         }
     }

     return false;
}
