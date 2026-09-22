#include "channelpage.h"
#include "ui_channelpage.h"
#include "auxiliary/uartProtocol.h"

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


void channelpage::update_SoftVer(int ch,const QString &ver){
   channelcard* card = create_chcard(ch);
   if (card){card->setChSWVersion(ver);}
}

void channelpage::update_HardVer(int ch,const QString &ver){
    channelcard* card = create_chcard(ch);
    if (card){card->setChHWVersion(ver);}
}

channelcard* channelpage::create_chcard(int ch){
    if (!m_channels.contains(ch)){
         channelcard* card = new channelcard(this);
         connect(card, &channelcard::clicked, this, [this,card](bool switchs) {
             quint8 func = switchs ? JV6xxx::Output::SET_OUTPSTATUS_ON : JV6xxx::Output::SET_OUTPSTATUS_OFF;
             to_Channel(card->m_channel,JV6xxx::OUTPUT, func, "");
             });
         connect(card, &channelcard::longPressed, this, [this]() {emit jump_chsetpage();});

         card->setChannel(ch);
         m_channels[ch] = card; // on top of underneath
         int neededPages = (m_channels.size() + 5) / 6;  // Round up

         QHBoxLayout* layout = nullptr;
         if (ui->ch_stackedWidget->count() < neededPages) {
             QWidget* page = new QWidget();
             QFrame* frame = new QFrame(page);
             layout = new QHBoxLayout(frame);

             frame->setGeometry(0, 0, 1138, 320);
             // 1138 220*5=1100 1138-1100=38 38/6=6-2
             layout->setContentsMargins(7, 2, 7, 2);
             layout->setSpacing(6);

             frame->setLayout(layout);
             ui->ch_stackedWidget->addWidget(page);

             ui->progressBar->setValue(1);
             ui->progressBar->setMaximum(neededPages);
             ui->ch_stackedWidget->setCurrentIndex(0);

             ui->pre_pushButton->setEnabled(false);
             ui->next_pushButton->setEnabled(0 < neededPages - 1);
         }else{
             QWidget* page = ui->ch_stackedWidget->widget(neededPages - 1); // index
             QFrame* frame = page->findChild<QFrame*>();
             layout = qobject_cast<QHBoxLayout*>(frame->layout());
         }

         layout->addWidget(card);
         return card;
     }

    return m_channels[ch];
}

void channelpage::update_IsOutput(int ch,bool status){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setChannelState(status);
    }
}

void channelpage::update_Voltage(int ch,float voltage){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        if (itcard.value()->getChmode() == Chindex::Digital){
            itcard.value()->setVoltage(voltage);
        }else if (itcard.value()->getChstatus()){
            /* battery mode and outputed status */
            QByteArray data(4,0);
            float ocvval = itcard.value()->getcurrentOCV();
            float esrval = itcard.value()->getcurrentESR();

            if (ocvval == 0.0f || esrval == 0.0f){
                to_Channel(itcard.value()->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");
                return;
            }else{
                qToBigEndian(ocvval, reinterpret_cast<uint8_t*>(data.data()));
                to_Channel(ch,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURVOLT, data);
                qToBigEndian(esrval, reinterpret_cast<uint8_t*>(data.data()));
                to_Channel(ch,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURIMP, data);
            }
        }
    }
}

void channelpage::update_CurrentAndUnit(int ch,float current){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        if (itcard.value()->getChmode() == Chindex::Digital){
            QString newUnit = (qAbs(current) < 1e-4) ? "mA" : "A";
            itcard.value()->setCurrent(current);
            itcard.value()->setCurrentUnit(newUnit);
        }else if (itcard.value()->getChstatus()){
            itcard.value()->updateSocValue(current);
        }
    }
}

void channelpage::update_Cv(int ch,float cv){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()){
        itcard.value()->setCvValue(cv);
        itcard.value()->setOcvValue(cv);
    }
}

void channelpage::update_Cc(int ch,float cc){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setCcValue(cc);
    }
}

void channelpage::update_Ovp(int ch,float ovp){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setOvpValue(ovp);
    }
}

void channelpage::update_Imp(int ch,float imp){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()){
        itcard.value()->setEsrValue(imp);
    }
}

void channelpage::update_Range(int ch,int range){ // 按照最新设计这个应该用不上了
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->m_curange = m_range.value(range);
    }
}

void channelpage::update_Status(int ch,quint16 status){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        QString binaryStr = QString("%1").arg(status, 16, 2, QLatin1Char('0'));
        bool cv  = binaryStr.at(14) == "1";
        bool cc  = binaryStr.at(13) == "1";
        bool ovp = binaryStr.at(11) == "1";

        itcard.value()->setCVChecked(cv);
        itcard.value()->setCCChecked(cc);
        itcard.value()->setOVPChecked(ovp);
    }
}

// web or main call function

bool channelpage::load_BatteryModel(){
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

void channelpage::to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param){
    // All channel send
    if (channel == 0) {
        #define CHANNEL(n) emit to_UartChannel##n(cmd, func, param,false);
        CHANNEL_COUNT
        #undef CHANNEL
        return;
    }

    // Single channel send
    switch(channel) {
        #define CHANNEL(n) case n: return emit to_UartChannel##n(cmd, func, param,false);
        CHANNEL_COUNT
        #undef CHANNEL
        default:return;
    }
}

/* slots function */
void channelpage::on_set_pushButton_clicked(){emit jump_devicesetpage();}

void channelpage::on_ver_pushButton_clicked(){
    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChstatus()){to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");}
        card->setChannelmode(Chindex::Version);
    }
}

void channelpage::on_batt_pushButton_clicked(){
    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChstatus()){to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");}
        card->setChannelmode(Chindex::Battery);
    }
}

void channelpage::on_dig_pushButton_clicked(){
    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChstatus()){to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");}
        card->setChannelmode(Chindex::Digital);
    }
}

void channelpage::on_pre_pushButton_clicked(){
    int newindex = ui->ch_stackedWidget->currentIndex() - 1;
    int total = ui->ch_stackedWidget->count();

    ui->ch_stackedWidget->setCurrentIndex(newindex);
    ui->progressBar->setValue(newindex + 1);
    ui->progressBar->setMaximum(total);

    ui->next_pushButton->setEnabled(newindex < total - 1);
    ui->pre_pushButton->setEnabled(newindex > 0);
}

void channelpage::on_next_pushButton_clicked(){
    int newindex = ui->ch_stackedWidget->currentIndex() + 1;
    int total = ui->ch_stackedWidget->count();

    ui->ch_stackedWidget->setCurrentIndex(newindex);
    ui->progressBar->setValue(newindex + 1);
    ui->progressBar->setMaximum(total);

    ui->next_pushButton->setEnabled(newindex < total - 1);
    ui->pre_pushButton->setEnabled(newindex > 0);
}
