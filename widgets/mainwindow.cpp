#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "auxiliary/uartProtocol.h"
#include <QTimer>
#include <QtCore>

Q_LOGGING_CATEGORY(widget, "WIDGET:")

// auto add exist channel card

void Mainwindow::update_cardtest()
{
    QList<int> channels = {3, 1, 4,2,8,9,24,15,32,12,18};
    //for (int i = 1; i <= 36; i++) {channels.append(i);}
    std::sort(channels.begin(), channels.end());

    for (const int ch : channels) {
        update_SoftVer(ch,"1.2.3.4");
        update_HardVer(ch,"4.3.2.1");
    }
}

// update digital card information

void Mainwindow::update_SoftVer(int ch,const QString &ver){
   channelcard* card = create_chcard(ch);
   if (card){card->setChSWVersion(ver);}
}

void Mainwindow::update_HardVer(int ch,const QString &ver){
    channelcard* card = create_chcard(ch);
    if (card){card->setChHWVersion(ver);}
}

channelcard* Mainwindow::create_chcard(int ch){
    if (!m_channels.contains(ch)){
         channelcard* card = new channelcard(this);
         card->setChannel(ch);

         connect(card, &channelcard::clicked, this, [this,card](bool switchs) {
             quint8 func = switchs ? JV6xxx::Output::SET_OUTPSTATUS_ON : JV6xxx::Output::SET_OUTPSTATUS_OFF;
             to_Channel(card->m_channel,JV6xxx::OUTPUT, func, "");
             });
         connect(card, &channelcard::longPressed, this, [this,card]() {
             m_chstatuscard->setChtatus("0000000000000000");

             m_funcchcard->setChannel(card->m_channel);
             m_funcchcard->m_curange = card->m_curange;
             m_funcchcard->setVoltage(card->m_voltage.load());
             m_funcchcard->setCurrent(card->m_current.load());
             m_funcchcard->setCurrentUnit(card->getChunit());
             m_funcchcard->setCvValue(card->m_cv.load());
             m_funcchcard->setCcValue(card->m_cc.load());
             m_funcchcard->setOvpValue(card->m_ovp.load());
             m_funcchcard->setChannelState(card->getChstatus());
             m_funcchcard->setCVChecked(card->getChCVChecked());
             m_funcchcard->setCCChecked(card->getChCCChecked());
             m_funcchcard->setOVPChecked(card->getChOVPChecked());

             m_funcchcard->setSocValue(card->m_soc.load());
             m_funcchcard->setOcvValue(card->m_ocv.load());
             m_funcchcard->setEsrValue(card->m_esr.load());
             m_funcchcard->setCapValue(card->m_cap.load());
             QString modelname = card->getChmodelname();
             m_funcchcard->setModelValue(modelname);

             ui->cvradioButton->setChecked(true);
             ui->cvvaluelabel->setText("");
             ui->ccvaluelabel->setText("");
             ui->ovpvaluelabel->setText("");
             ui->functionunitpushButton->setText(card->m_curange);

             ui->socradioButton->setChecked(true);
             ui->socvaluelabel->setText("");
             ui->capvaluelabel->setText("");
             ui->functionmodelpushButton->setText(modelname);

             m_funcchcard->setChannelmode(card->getChmode());
             ui->functionstackedWidget->setCurrentIndex(card->getChmode());
             ui->topstackedwidget->setCurrentIndex(WidgetIndex::Function);
             });

         QWidget* page = nullptr;
         QFrame* frame = nullptr;
         QHBoxLayout* layout = nullptr;

         m_channels[ch] = card; // on top of underneath
         int neededPages = (m_channels.size() + 5) / 6;  // Round up

         if (ui->digitalstackedWidget->count() < neededPages) {
             page = new QWidget();
             frame = new QFrame(page);
             frame->setGeometry(0, 0, 1280, 320);

             layout = new QHBoxLayout(frame);
             layout->setContentsMargins(2, 2, 2, 2);
             layout->setSpacing(2);

             frame->setLayout(layout);
             ui->digitalstackedWidget->addWidget(page);

             ui->digitalstackedWidget->setCurrentIndex(0);
             ui->digitalrowsbackpushButton->setEnabled(false);
             ui->digitalrowsnextpushButton->setEnabled(0 < neededPages - 1);
             ui->digitalrowslabelpushButton->setText(QString("%1 / %2").arg(1).arg(neededPages));
         }else{
             page = ui->digitalstackedWidget->widget(neededPages - 1); // index
             frame = page->findChild<QFrame*>();
             layout = qobject_cast<QHBoxLayout*>(frame->layout());
         }

         layout->addWidget(card);
         return card;
     }

    return m_channels[ch];
}


void Mainwindow::update_IsOutput(int ch,bool status){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setChannelState(status);
    }

    if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
        m_funcchcard->setChannelState(status);
    }
}

void Mainwindow::update_Voltage(int ch,float voltage){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        if (itcard.value()->getChmode() == Chindex::Digital){
            itcard.value()->setVoltage(voltage);

            if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
                m_funcchcard->setVoltage(voltage);
            }
        }else if (itcard.value()->getChstatus()){
            /* battery mode and outputed status */
            QByteArray data(4,0);float value;
            value = itcard.value()->getcurrentOCV();
            if (value == 0.0f){
                to_Channel(itcard.value()->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");
            }else{
                qToBigEndian(value, reinterpret_cast<uint8_t*>(data.data()));
                to_Channel(ch,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURVOLT, data);
            }

            value = itcard.value()->getcurrentESR();
            if (value == 0.0f){
                to_Channel(itcard.value()->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");
            }else{
                qToBigEndian(value, reinterpret_cast<uint8_t*>(data.data()));
                to_Channel(ch,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURIMP, data);
            }
        }
    }
}

void Mainwindow::update_CurrentAndUnit(int ch,float current){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        if (itcard.value()->getChmode() == Chindex::Digital){
            QString newUnit = (qAbs(current) < 1e-4) ? "mA" : "A";
            itcard.value()->setCurrent(current);
            itcard.value()->setCurrentUnit(newUnit);

            if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
                m_funcchcard->setCurrent(current);
                m_funcchcard->setCurrentUnit(newUnit);
            }
        }else if (itcard.value()->getChstatus()){
            itcard.value()->updateSocValue(current);

            if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
                m_funcchcard->updateSocValue(current);
            }
        }
    }

}

void Mainwindow::update_Cv(int ch,float cv){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()){
        itcard.value()->setCvValue(cv);
        itcard.value()->setOcvValue(cv);

        if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
            ui->cvvaluelabel->setText(QString::number(cv, 'f', 3));
            m_funcchcard->setCvValue(cv);
            m_funcchcard->setOcvValue(cv);
        }
    }
}


void Mainwindow::update_Cc(int ch,float cc){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setCcValue(cc);

        if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
            ui->ccvaluelabel->setText(QString::number(cc, 'f', 3));
            m_funcchcard->setCcValue(cc);
        }
    }
}

void Mainwindow::update_Ovp(int ch,float ovp){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->setOvpValue(ovp);

        if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
            ui->ovpvaluelabel->setText(QString::number(ovp, 'f', 3));
            m_funcchcard->setOvpValue(ovp);
        }
    }
}

void Mainwindow::update_Range(int ch,int range){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        itcard.value()->m_curange = m_range.value(range);

        if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
            QString unit = m_range.contains(range) ? m_range[range] : "";
            ui->functionunitpushButton->setText(unit);
            m_funcchcard->m_curange = m_range.value(range);
        }
    }
}

void Mainwindow::update_Status(int ch,quint16 status){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()) {
        QString binaryStr = QString("%1").arg(status, 16, 2, QLatin1Char('0'));
        bool cv  = binaryStr.at(14) == "1";
        bool cc  = binaryStr.at(13) == "1";
        bool ovp = binaryStr.at(11) == "1";

        itcard.value()->setCVChecked(cv);
        itcard.value()->setCCChecked(cc);
        itcard.value()->setOVPChecked(ovp);

        if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
            m_chstatuscard->setChtatus(binaryStr);
            m_funcchcard->setCVChecked(cv);
            m_funcchcard->setCCChecked(cc);
            m_funcchcard->setOVPChecked(ovp);
        }
    }
}


void Mainwindow::update_Imp(int ch,float imp){
    auto itcard = m_channels.find(ch);
    if (itcard != m_channels.end()){
        itcard.value()->setEsrValue(imp);

        if (ui->topstackedwidget->currentIndex() == WidgetIndex::Function && ch == m_funcchcard->m_channel){
            m_funcchcard->setEsrValue(imp);
        }
    }
}

void Mainwindow::update_remotemodel(int reface){
    if (ConfigManager::s_remoteSt.load() != reface){
        if (reface == InterSt::LOCAL){m_remoteOverlay->hide();} else{m_remoteOverlay->show();}
        ConfigManager::s_remoteSt.store(reface);
    }
}

// web or main call function

bool Mainwindow::load_BatteryModel(){
    for (auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
        if (it.value()->getChstatus()){
            qCWarning(widget)<<"Loading is prohibited during work!";
            return false;
        }
    }

    if (m_modelManager && m_modelManager->loadAllModels()) {
        m_currentModelList = m_modelManager->getAvailableModels();
        QString batteryModel = m_currentModelList[0]; // load = true -> exist at least 1
        QString modelname  =  "All - " + batteryModel;
        ui->digitalallunitpushButton->setText(modelname);

        for (auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
            it.value()->m_modelindex = 0;
            it.value()->setModelValue(batteryModel);
            it.value()->m_activeModel = m_modelManager->getModel(batteryModel);
        }

        return true;
    }

    return false;
}

QJsonArray Mainwindow::getAllChannelsData()
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


void Mainwindow::update_setting(){m_devicesetcard->responseUpdate();}

Mainwindow::Mainwindow(QWidget *parent) : QWidget(parent), ui(new Ui::Mainwindow)
{
    ui->setupUi(this);

    m_range = {
        {0x00, "A"},
        {0x01, "mA"},
        {0x10, "Auto"},
    };
    for (auto it = m_range.constBegin(); it != m_range.constEnd(); ++it) {
        m_rangeReverse[it.value()] = it.key();
    }

    // function page component

    m_vercard = new versionview(ui->functionversionframe->parentWidget());
    m_vercard->setGeometry(ui->functionversionframe->geometry());
    delete ui->functionversionframe;

    m_chstatuscard = new chstatusview(ui->functionchstatusframe->parentWidget());
    m_chstatuscard->setGeometry(ui->functionchstatusframe->geometry());
    delete ui->functionchstatusframe;

    m_funcchcard = new channelcard(ui->functionchframe->parentWidget());
    m_funcchcard->setGeometry(ui->functionchframe->geometry());
    QObject::connect(m_funcchcard, &channelcard::clicked, this, [this](quint8 ch,bool switchs) {
        quint8 func = switchs ? JV6xxx::Output::SET_OUTPSTATUS_ON : JV6xxx::Output::SET_OUTPSTATUS_OFF;
        to_Channel(ch,JV6xxx::OUTPUT, func, "");
    });
    delete ui->functionchframe;

    m_funcnmbkeycard = new numberkeypad(ui->functionkeyframe->parentWidget());
    m_funcnmbkeycard->setGeometry(ui->functionkeyframe->geometry());
    QObject::connect(m_funcnmbkeycard, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){
            float val = value.toFloat();

            if (m_funcchcard->getChmode() == Chindex::Digital){
                // model: 0 - CV ; 1 - CC ; 3 - OVP;
                QByteArray data(4,0);
                qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));
                return to_Channel(m_funcchcard->m_channel,JV6xxx::SETTING, m_functioncsetmode, data);
            }else if (m_functioncsetmode == 0){
                ui->socvaluelabel->setText(value);
                m_funcchcard->setSocValue(val);
                m_channels[m_funcchcard->m_channel]->setSocValue(val);
            }else{
                ui->capvaluelabel->setText(value);
                m_funcchcard->setCapValue(val);
                m_channels[m_funcchcard->m_channel]->setCapValue(val);
            }
        }
    });
    delete ui->functionkeyframe;

    // setting page component

    m_devicesetcard = new devicesetting(ui->settingframe->parentWidget());
    m_devicesetcard->setGeometry(ui->settingframe->geometry());
    QObject::connect(m_devicesetcard,&devicesetting::set_network,this,&Mainwindow::set_network);
    QObject::connect(m_devicesetcard,&devicesetting::set_canbaud,this,&Mainwindow::set_canbaud);
    QObject::connect(m_devicesetcard,&devicesetting::set_RS232Baud,this,&Mainwindow::set_RS232Baud);
    delete ui->settingframe;

    m_setnmbkeycard = new numberkeypad(ui->settingkeyframe->parentWidget());
    m_setnmbkeycard->setGeometry(ui->settingkeyframe->geometry());
    QObject::connect(m_setnmbkeycard, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){m_devicesetcard->setting(value);}
    });
    delete ui->settingkeyframe;

    // remote over page

    m_remoteOverlay = new remoteoverlay(this);
    connect(m_remoteOverlay, &remoteoverlay::exitRemote, this, []() {ConfigManager::s_remoteSt.store(InterSt::LOCAL);});

    update_cardtest();
}

Mainwindow::~Mainwindow()
{
    delete ui;
}


void Mainwindow::to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param){
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

// digital / battery to switchs cards or other

void Mainwindow::on_digitalmodepushButton_clicked(){
    m_channelbattery = !m_channelbattery;
    ui->digitalallunitpushButton->setText("Refresh");
    ui->digitalmodepushButton->setIcon(m_channelbattery ? QIcon(":/web/icon/promotion.png") : QIcon(":/web/icon/battery.png"));

    for (channelcard *card :  qAsConst(m_channels)) {
        if (card->getChstatus()){to_Channel(card->m_channel,JV6xxx::OUTPUT, JV6xxx::Output::SET_OUTPSTATUS_OFF, "");}
        card->setChannelmode(m_channelbattery ? Chindex::Battery : Chindex::Digital);
    }
}

void Mainwindow::on_digitalsettingspushButton_clicked(){m_devicesetcard->reenterUpdate();ui->topstackedwidget->setCurrentIndex(WidgetIndex::Settings);}

void Mainwindow::on_digitalrowsbackpushButton_clicked()
{
    int total = ui->digitalstackedWidget->count();
    int newindex = ui->digitalstackedWidget->currentIndex() - 1;

    ui->digitalstackedWidget->setCurrentIndex(newindex);
    ui->digitalrowsbackpushButton->setEnabled(newindex > 0);
    ui->digitalrowsnextpushButton->setEnabled(newindex < total - 1);
    ui->digitalrowslabelpushButton->setText(QString("%1 / %2").arg(newindex + 1).arg(total));
}

void Mainwindow::on_digitalrowsnextpushButton_clicked()
{
    int total = ui->digitalstackedWidget->count();
    int newindex = ui->digitalstackedWidget->currentIndex() + 1;

    ui->digitalstackedWidget->setCurrentIndex(newindex);
    ui->digitalrowsbackpushButton->setEnabled(newindex > 0);
    ui->digitalrowsnextpushButton->setEnabled(newindex < total - 1);
    ui->digitalrowslabelpushButton->setText(QString("%1 / %2").arg(newindex + 1).arg(total));
}


void Mainwindow::on_digitalallONpushButton_clicked(){
    static bool allOn{false};
    allOn = !allOn;

    quint8 func = allOn?JV6xxx::Output::SET_OUTPSTATUS_ON : JV6xxx::Output::SET_OUTPSTATUS_OFF;
    ui->digitalallONpushButton->setText(allOn? "All - OFF" : "All - ON");
    to_Channel(0,JV6xxx::OUTPUT, func, "");
}

void Mainwindow::on_digitalallunitpushButton_clicked()
{
    if (m_channelbattery){
        static quint8 currentIndex = 0;

        if (!m_currentModelList.isEmpty()) {
            currentIndex = (currentIndex + 1) % m_currentModelList.size();
            QString batteryModel = m_currentModelList[currentIndex];
            ui->digitalallunitpushButton->setText("All - " + batteryModel);

            for (auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
                m_funcchcard->m_modelindex = currentIndex;
                m_channels[m_funcchcard->m_channel]->setModelValue(batteryModel);
                m_channels[m_funcchcard->m_channel]->m_activeModel = m_modelManager->getModel(batteryModel);
            }
        }
    }else{
        static int range = 0;
        auto it = m_range.upperBound(range);
        if (it == m_range.end()) {it = m_range.begin();}

        range = it.key();
        ui->digitalallunitpushButton->setText("All - " + it.value());
        to_Channel(0,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSCURR_RANGE, QByteArray(1, char(it.key())));
    }
}

// setting page switchs

void Mainwindow::on_settingrowsbackpushButton_clicked(){ui->topstackedwidget->setCurrentIndex(WidgetIndex::Channel);}

// function page switchs or other

void Mainwindow::on_functionrowsbackpushButton_clicked(){ui->topstackedwidget->setCurrentIndex(WidgetIndex::Channel);}

void Mainwindow::on_functionsettingspushButton_clicked(){m_devicesetcard->reenterUpdate();ui->topstackedwidget->setCurrentIndex(2);}

void Mainwindow::on_functionallapplypushButton_clicked()
{
    if (m_funcchcard->getChmode() == Chindex::Digital){
        QByteArray data(4,0);
        // model: 0 - CV ; 1 - CC ; 3 - OVP;
        if (ui->cvvaluelabel->text() != ""){
            float val = ui->cvvaluelabel->text().toFloat();
            qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));
            to_Channel(0,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURVOLT, data);
        }
        if (ui->ccvaluelabel->text() != ""){
            float val = ui->ccvaluelabel->text().toFloat();
            qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));
            to_Channel(0,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURCURR, data);
        }
        if (ui->ovpvaluelabel->text() != ""){
            float val = ui->ovpvaluelabel->text().toFloat();
            qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));
            to_Channel(0,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURVOLT_PROT, data);
        }
    }else{
        if (ui->socvaluelabel->text() != ""){
            float val = ui->socvaluelabel->text().toFloat();
            for (auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
                it.value()->setSocValue(val);
            }
        }
        if (ui->capvaluelabel->text() != ""){
            float val = ui->capvaluelabel->text().toFloat();
            for (auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
                it.value()->setCapValue(val);
            }
        }
    }
}


void Mainwindow::on_cvradioButton_clicked(){m_functioncsetmode = 0;}

void Mainwindow::on_ccradioButton_clicked(){m_functioncsetmode = 1;}

void Mainwindow::on_ovpradioButton_clicked(){m_functioncsetmode = 3;}

void Mainwindow::on_functionunitpushButton_clicked()
{
    auto it = m_rangeReverse.upperBound(m_funcchcard->m_curange);
    if (it == m_rangeReverse.end()) {it = m_rangeReverse.begin();}
    to_Channel(m_funcchcard->m_channel,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_SENSCURR_RANGE, QByteArray(1, char(it.value())));
}


void Mainwindow::on_socradioButton_clicked(){m_functioncsetmode = 0;}

void Mainwindow::on_capradioButton_clicked(){m_functioncsetmode = 1;}

void Mainwindow::on_functionmodelpushButton_clicked()
{
   if (!m_currentModelList.isEmpty()) {
        m_funcchcard->m_modelindex = (m_funcchcard->m_modelindex + 1) % m_currentModelList.size();
        QString batteryModel = m_currentModelList[m_funcchcard->m_modelindex];

        m_channels[m_funcchcard->m_channel]->setModelValue(batteryModel);
        m_channels[m_funcchcard->m_channel]->m_activeModel = m_modelManager->getModel(batteryModel);
    }
}
