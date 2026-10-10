#include "auxiliary/uartProtocol.h"
#include "auxiliary/channelbus.h"
#include "ui_channelpage.h"
#include "channelpage.h"

/* slots function */

void channelpage::on_setDe_pushButton_clicked(){emit jump_devicesetpage();}

void channelpage::on_setCh_pushButton_clicked(){emit jump_chsetpage();}

void channelpage::on_dig_pushButton_clicked()
{
    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChstatus()){
            ChannelBus::instance()->to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");
        }

        card->setChannelmode(ChCardindex::Digital);
    }
}

void channelpage::on_batt_pushButton_clicked()
{
    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChstatus()){
            ChannelBus::instance()->to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");
        }

        card->setChannelmode(ChCardindex::Battery);
    }
}

void channelpage::on_stat_pushButton_clicked()
{
    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChstatus()){
            ChannelBus::instance()->to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");
        }

        card->setChannelmode(ChCardindex::Status);
    }
}

void channelpage::on_ver_pushButton_clicked()
{
    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChstatus()){
            ChannelBus::instance()->to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");
        }

        card->setChannelmode(ChCardindex::Version);
    }
}

void channelpage::on_pre_pushButton_clicked()
{
    int newindex = ui->ch_stackedWidget->currentIndex() - 1;
    to_channelpage(newindex);
}

void channelpage::on_next_pushButton_clicked()
{
    int newindex = ui->ch_stackedWidget->currentIndex() + 1;
    to_channelpage(newindex);
}

void channelpage::to_channelpage(int newindex)
{
    ui->ch_stackedWidget->setCurrentIndex(newindex);
    int total = ui->ch_stackedWidget->count();

    ui->progressBar->setValue(newindex + 1);
    ui->progressBar->setMaximum(total);

    ui->pre_pushButton->setEnabled(newindex > 0);
    ui->next_pushButton->setEnabled(newindex < total - 1);
}

channelpage::channelpage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::channelpage)
{
    ui->setupUi(this);
}

channelpage::~channelpage()
{
    delete ui;
}

/* create channel card */

void channelpage::update_cardtest()
{
    QList<int> channels = {3, 1, 4,2,8,9,24,15,32,12,18};
    //for (int i = 1; i <= 36; i++) {channels.append(i);}
    std::sort(channels.begin(), channels.end());

    for (const int ch : channels) {
        update_SoftVer(ch,"1.2.3.4");
        update_HardVer(ch,"4.3.2.1");
    }
}

void channelpage::update_SoftVer(int ch,const QString &ver)
{
   channelcard* card = create_chcard(ch);
   if (card){card->setChSWVersion(ver);}
}

void channelpage::update_HardVer(int ch,const QString &ver)
{
    channelcard* card = create_chcard(ch);
    if (card){card->setChHWVersion(ver);}
}

channelcard* channelpage::create_chcard(int ch)
{
    if (!m_channels.contains(ch)){
        QWidget* page = nullptr;
        QHBoxLayout* layout = nullptr;
        int neededPages = (m_channels.size() + ChPageCard::PageCard) / ChPageCard::PageCard;// Round up

        if (ui->ch_stackedWidget->count() < neededPages) {
            page = new QWidget();
            layout = new QHBoxLayout(page);

            layout->setContentsMargins(ChPageCard::RLMargin,ChPageCard::TBMargin,ChPageCard::RLMargin,ChPageCard::TBMargin);
            layout->setSpacing(ChPageCard::CardSpace);
            page->setLayout(layout);

            ui->ch_stackedWidget->addWidget(page);
            ui->ch_stackedWidget->setCurrentIndex(0);
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
             ChannelBus::instance()->ChannelBus::instance()->to_Channel(card->m_channel,JV6xxx::OUTPUT, func, "");
             });
         //connect(card, &channelcard::longPressed, this, [this]() {emit jump_chsetpage();});

         m_channels[ch] = card;
         return card;
     }

    return m_channels[ch];
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
        }else if (itcard.value()->getChstatus()){
            /* battery mode and outputed status */
            QByteArray data(4,0);
            float ocvval = itcard.value()->getcurrentOCV();
            float esrval = itcard.value()->getcurrentESR();

            if (ocvval == 0.0f || esrval == 0.0f){
                ChannelBus::instance()->to_Channel(itcard.value()->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");
                return;
            }else{
                qToBigEndian(ocvval, reinterpret_cast<uint8_t*>(data.data()));
                ChannelBus::instance()->to_Channel(ch,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURVOLT, data);
                qToBigEndian(esrval, reinterpret_cast<uint8_t*>(data.data()));
                ChannelBus::instance()->to_Channel(ch,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURIMP, data);
            }
        }
    }
}

void channelpage::update_CurrentAndUnit(int ch,float current)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        if (itcard.value()->getChmode() == ChCardindex::Digital){
            QString newUnit = (qAbs(current) < 1e-4) ? "mA" : "A";
            itcard.value()->setCurrent(current);
            itcard.value()->setCurrentUnit(newUnit);
        }else if (itcard.value()->getChstatus()){
            itcard.value()->updateSocValue(current);
        }
    }
}

void channelpage::update_Cv(int ch,float cv)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()){
        itcard.value()->setCvValue(cv);
        itcard.value()->setOcvValue(cv);
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

void channelpage::update_Imp(int ch,float imp)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()){
        itcard.value()->setEsrValue(imp);
    }
}

void channelpage::update_Range(int ch,int range)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->m_curange = m_range.value(range);
    }
}

void channelpage::update_Status(int ch,quint16 status)
{
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        QString binaryStr = QString("%1").arg(status, 16, 2, QLatin1Char('0'));

        if (itcard.value()->getChmode() == ChCardindex::Digital){
            bool cv  = binaryStr.at(14) == "1";
            bool cc  = binaryStr.at(13) == "1";
            bool ovp = binaryStr.at(11) == "1";

            itcard.value()->setCVChecked(cv);
            itcard.value()->setCCChecked(cc);
            itcard.value()->setOVPChecked(ovp);
        }else if(itcard.value()->getChmode() == ChCardindex::Status){
            itcard.value()->setChtatus(binaryStr);
        }
    }
}

/* web or main call function */

bool channelpage::load_BatteryModel()
{
    for (auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
        if (it.value()->getChstatus()){
            return false;
        }
    }

    if (m_modelManager && m_modelManager->loadAllModels()) {
        m_currentModelList = m_modelManager->getAvailableModels();
        QString batteryModel = m_currentModelList[0]; // load = true -> exist at least 1

        for (auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
            it.value()->m_modelindex = 0;
            it.value()->setModelValue(batteryModel);
            it.value()->m_activeModel = m_modelManager->getModel(batteryModel);
        }

        return true;
    }

    return false;
}

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
