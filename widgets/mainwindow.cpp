#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QtCore>

Q_LOGGING_CATEGORY(widget, "WIDGET:")

// auto add exist channel card

void Mainwindow::update_cardtest()
{
    QList<int> channels;
    //channels = {1,2,3,4,5};
    channels = {3, 1, 4,2,8,9,24,15,32,12,18};
    //for (int i = 1; i <= 36; i++) {channels.append(i);}
    std::sort(channels.begin(), channels.end());

    int totalChannels = channels.size();
    int neededPages = (totalChannels + 5) / 6;  // Round up

    add_digitalcard(neededPages,channels);
    add_batterycard(neededPages,channels);
}

void Mainwindow::update_showcard()
{
    if (m_ChsoftVer.size() != 0 && m_ChsoftVer.size() == m_ChhardVer.size()){
        QList<int> channels = m_ChsoftVer.keys();
        std::sort(channels.begin(), channels.end());

        int totalChannels = channels.size();
        int neededPages = (totalChannels + 5) / 6;  // Round up

        add_digitalcard(neededPages,channels);
        add_batterycard(neededPages,channels);
        return;
    }

    qCDebug(widget)<<"[update_showcard]:sfverSize: "<<m_ChsoftVer.size()<<"hfversize: "<<m_ChhardVer.size();
}

void Mainwindow::add_digitalcard(int neededPages,const QList<int>& channels)
{
    while (ui->digitalstackedWidget->count() < neededPages) {
        QWidget* page = new QWidget();
        QFrame* frame = new QFrame(page);

        QHBoxLayout* layout = new QHBoxLayout(frame);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(2);

        frame->setGeometry(0, 0, 1280, 320);
        frame->setLayout(layout);

        ui->digitalstackedWidget->addWidget(page);
    }

    for (int idx = 0; idx < channels.size(); idx++) {
        int ch = channels[idx]; // 1-36
        int pageIndex = idx / 6;

        QWidget* page = ui->digitalstackedWidget->widget(pageIndex);
        QFrame* frame = page->findChild<QFrame*>();
        digitalcard* card = new digitalcard(frame);
        card->setChannel(ch);

        connect(card, &digitalcard::clicked, this, [this](quint8 ch, bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        connect(card, &digitalcard::longPressed, this, [this](quint8 ch) {
            m_initalpage = 0;
            m_functioncCh = ch;

            m_vercard->setChannelName(ch);
            m_vercard->setChSWVersion(m_ChsoftVer.value(ch));
            m_vercard->setChHWVersion(m_ChhardVer.value(ch));

            m_chstatuscard->setChtatus("0000000000000000");
            int range = m_digitalcards[ch]->getChRange();

            m_funcdigitalcard->setChannel(ch);
            m_funcdigitalcard->setChannelRange(range);
            m_funcdigitalcard->setVoltage(m_digitalcards[ch]->getChVoltage());
            m_funcdigitalcard->setCurrent(m_digitalcards[ch]->getChCurrent());
            m_funcdigitalcard->setCurrentUnit(m_digitalcards[ch]->getChunit());
            m_funcdigitalcard->setCvValue(m_digitalcards[ch]->getChCvValue());
            m_funcdigitalcard->setCcValue(m_digitalcards[ch]->getChCcValue());
            m_funcdigitalcard->setOvpValue(m_digitalcards[ch]->getChOvpValue());
            m_funcdigitalcard->setChannelState(m_digitalcards[ch]->getChstatus());
            m_funcdigitalcard->setCVChecked(m_digitalcards[ch]->getChCVChecked());
            m_funcdigitalcard->setCCChecked(m_digitalcards[ch]->getChCCChecked());
            m_funcdigitalcard->setOVPChecked(m_digitalcards[ch]->getChOVPChecked());

            m_functioncsetmode = 0;
            ui->cvradioButton->setChecked(true);
            ui->cvvaluelabel->setText("");
            ui->ccvaluelabel->setText("");
            ui->ovpvaluelabel->setText("");
            ui->functionunitpushButton->setText(m_range.value(range));

            ui->functionstackedWidget->setCurrentIndex(0); // = m_initalpage
            ui->topstackedwidget->setCurrentIndex(3);
        });

        QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(frame->layout());
        layout->addWidget(card);

        m_digitalcards[ch] = card;
    }

    ui->digitalstackedWidget->setCurrentIndex(0);
    ui->digitalrowsbackpushButton->setEnabled(false);
    ui->digitalrowsnextpushButton->setEnabled(0 < neededPages - 1);
    ui->digitalrowslabelpushButton->setText(QString("%1 / %2").arg(1).arg(neededPages));
}

void Mainwindow::add_batterycard(int neededPages,const QList<int>& channels)
{
    while (ui->batterystackedWidget->count() < neededPages) {
        QWidget* page = new QWidget();
        QFrame* frame = new QFrame(page);

        QHBoxLayout* layout = new QHBoxLayout(frame);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(2);

        frame->setGeometry(0, 0, 1280, 320);
        frame->setLayout(layout);

        ui->batterystackedWidget->addWidget(page);
    }

    for (int idx = 0; idx < channels.size(); idx++) {
        int ch = channels[idx];
        int pageIndex = idx / 6;

        QWidget* page = ui->batterystackedWidget->widget(pageIndex);
        QFrame* frame = page->findChild<QFrame*>();
        batterycard* card = new batterycard(frame);
        card->setChannel(ch);

        connect(card, &batterycard::clicked, this, [this](quint8 ch, bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        connect(card, &batterycard::longPressed, this, [this](quint8 ch) {
            m_initalpage = 1;
            m_functioncCh = ch;

            m_vercard->setChannelName(ch);
            m_vercard->setChSWVersion(m_ChsoftVer.value(ch));
            m_vercard->setChHWVersion(m_ChhardVer.value(ch));

            m_chstatuscard->setChtatus("0000000000000000");
            QString modelname = m_batterycards[ch]->getChmodelname();

            m_funcbatterycard->setChannel(ch);
            m_funcbatterycard->setModelValue(modelname);
            m_funcbatterycard->setSocValue(m_batterycards[ch]->getChSOCvalue());
            m_funcbatterycard->setOcvValue(m_batterycards[ch]->getChOcvvalue());
            m_funcbatterycard->setEsrValue(m_batterycards[ch]->getChEsrvalue());
            m_funcbatterycard->setCapValue(m_batterycards[ch]->getChCapvalue());
            m_funcbatterycard->setChannelState(m_batterycards[ch]->getChstatus());

            m_functioncsetmode = 0;
            ui->socradioButton->setChecked(true);
            ui->socvaluelabel->setText("");
            ui->capvaluelabel->setText("");
            ui->functionmodelpushButton->setText(modelname);

            ui->functionstackedWidget->setCurrentIndex(1); // = m_initalpage
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
        });

        QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(frame->layout());
        layout->addWidget(card);

        m_batterycards[ch] = card;
    }

    ui->batterystackedWidget->setCurrentIndex(0);
    ui->batteryrowsbackpushButton->setEnabled(false);
    ui->batteryrowsnextpushButton->setEnabled(0 < neededPages - 1);
    ui->batteryrowslabelpushButton->setText(QString("%1 / %2").arg(1).arg(neededPages));
}

// update digital card information

void Mainwindow::update_SoftVer(int ch,const QString &ver){m_ChsoftVer[ch] = ver;}

void Mainwindow::update_HardVer(int ch,const QString &ver){m_ChhardVer[ch] = ver;}

void Mainwindow::update_IsOutput(int ch,bool status){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setChannelState(status && m_initalpage == 0);
    }

    if (m_batterycards.contains(ch)){
        m_batterycards[ch]->setChannelState(status && m_initalpage == 1);
        m_batterycards[ch]->startupdateValue();
    }

    if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
        m_funcdigitalcard->setChannelState(status && m_initalpage == 0);
        m_funcbatterycard->setChannelState(status && m_initalpage == 1);
        m_funcbatterycard->startupdateValue();
    }
}

void Mainwindow::update_Voltage(int ch,float voltage){
    if (m_initalpage == 0 && m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setVoltage(voltage);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
            m_funcdigitalcard->setVoltage(voltage);
        }
    }

    if (m_batterycards.contains(ch) && m_batterycards[ch]->getChstatus()){
        QByteArray data(4,0);float value;
        value = m_batterycards[ch]->getcurrentOCV();
        qToBigEndian(value, reinterpret_cast<uint8_t*>(data.data()));
        to_Channel(ch,0x02, 0x00, data);

        value = m_batterycards[ch]->getcurrentESR();
        qToBigEndian(value, reinterpret_cast<uint8_t*>(data.data()));
        to_Channel(ch,0x02, 0x02, data);
    }
}

void Mainwindow::update_CurrentAndUnit(int ch,float current){
    if (m_initalpage == 0 && m_digitalcards.contains(ch)) {
        QString newUnit = (qAbs(current) < 1e-4) ? "mA" : "A";
        m_digitalcards[ch]->setCurrent(current);
        m_digitalcards[ch]->setCurrentUnit(newUnit);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
            m_funcdigitalcard->setCurrent(current);
            m_funcdigitalcard->setCurrentUnit(newUnit);
        }
    }

    if (m_batterycards.contains(ch) && m_batterycards[ch]->getChstatus()){
        m_batterycards[ch]->updateSocValue(current);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
            m_funcbatterycard->updateSocValue(current);
        }
    }
}

void Mainwindow::update_Cv(int ch,float cv){
    if (m_digitalcards.contains(ch)){m_digitalcards[ch]->setCvValue(cv);}
    if (m_batterycards.contains(ch)){m_batterycards[ch]->setOcvValue(cv);}

    if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
        if (m_initalpage == 0){
            ui->cvvaluelabel->setText(QString::number(cv, 'f', 3));
            m_funcdigitalcard->setCvValue(cv);
        }else{
            m_funcbatterycard->setOcvValue(cv);
        }
    }
}


void Mainwindow::update_Cc(int ch,float cc){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setCcValue(cc);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh && m_initalpage == 0){
            ui->ccvaluelabel->setText(QString::number(cc, 'f', 3));
            m_funcdigitalcard->setCcValue(cc);
        }
    }
}

void Mainwindow::update_Ovp(int ch,float ovp){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setOvpValue(ovp);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh && m_initalpage == 0){
            ui->ovpvaluelabel->setText(QString::number(ovp, 'f', 3));
            m_funcdigitalcard->setOvpValue(ovp);
        }
    }
}

void Mainwindow::update_Range(int ch,int range)
{
    if (m_initalpage == 0 && m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setChannelRange(range);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
            QString unit = m_range.contains(range) ? m_range[range] : "";
            ui->functionunitpushButton->setText(unit);
            m_funcdigitalcard->setChannelRange(range);
        }
    }
}

void Mainwindow::update_Status(int ch,quint16 status){
    if (m_digitalcards.contains(ch)) {
        QString binaryStr = QString("%1").arg(status, 16, 2, QLatin1Char('0'));

        bool cv  = binaryStr.at(14) == "1";
        bool cc  = binaryStr.at(13) == "1";
        bool ovp = binaryStr.at(11) == "1";

        if (m_initalpage == 0){
            m_digitalcards[ch]->setCVChecked(cv);
            m_digitalcards[ch]->setCCChecked(cc);
            m_digitalcards[ch]->setOVPChecked(ovp);
        }

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
            m_chstatuscard->setChtatus(binaryStr);

            if (m_initalpage == 0){
                m_funcdigitalcard->setCVChecked(cv);
                m_funcdigitalcard->setCCChecked(cc);
                m_funcdigitalcard->setOVPChecked(ovp);
            }
        }
    }
}


void Mainwindow::update_Imp(int ch,float imp){
    if (m_batterycards.contains(ch)){
        m_batterycards[ch]->setEsrValue(imp);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh && m_initalpage == 1){
            m_funcbatterycard->setEsrValue(imp);
        }
    }
}

void Mainwindow::update_remotemodel(int reface){
    if (ConfigManager::s_remoteSt.load() != reface){
        if (reface == 0){m_remoteOverlay->hide();} else{m_remoteOverlay->show();}
        ConfigManager::s_remoteSt.store(reface);
    }
}

// web or main call function

bool Mainwindow::load_BatteryModel(){
    for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
        if (it.value()->getChstatus()){
            qCWarning(widget)<<"Loading is prohibited during work!";
            return false;
        }
    }

    if (m_modelManager && m_modelManager->loadAllModels()) {
        m_currentModelList = m_modelManager->getAvailableModels();
        QString batteryModel = m_currentModelList[0]; // load = true -> exist at least 1
        QString modelname  =  "All - " + batteryModel;
        ui->batteryallmodelpushButton->setText(modelname);

        for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
            it.value()->setModel(0,m_modelManager->getModel(batteryModel));
            it.value()->setModelValue(batteryModel);
        }

        return true;
    }

    return false;
}

QJsonArray Mainwindow::getAllChannelsData()
{
    QJsonArray channels;

    for (auto it = m_digitalcards.begin(); it != m_digitalcards.end(); ++it) {
        QJsonObject channel;
        channel["channel"] = it.key();
        channel["isOutput"] = it.value()->getChstatus();
        channel["voltage"] = it.value()->getChVoltage();
        channel["current"] = it.value()->getChCurrent();
        channel["current_unit"] = it.value()->getChunit();
        channel["cvSetpoint"] = it.value()->getChCvValue();
        channel["cvstatus"] = it.value()->getChCVChecked();
        channel["ccSetpoint"] = it.value()->getChCcValue();
        channel["ccstatus"] = it.value()->getChCCChecked();
        channel["ovSetpoint"] = it.value()->getChOvpValue();
        channel["ovpstatus"] = it.value()->getChOVPChecked();
        channels.append(channel);
    }

    return channels;
}


void Mainwindow::update_setting(){m_devicesetcard->responseUpdate();}

Mainwindow::Mainwindow(QWidget *parent) : QWidget(parent), ui(new Ui::Mainwindow)
{
    ui->setupUi(this);

    // function page component

    m_vercard = new versionview(ui->functionversionframe->parentWidget());
    m_vercard->setGeometry(ui->functionversionframe->geometry());
    delete ui->functionversionframe;

    m_chstatuscard = new chstatusview(ui->functionchstatusframe->parentWidget());
    m_chstatuscard->setGeometry(ui->functionchstatusframe->geometry());
    delete ui->functionchstatusframe;

    m_funcdigitalcard = new digitalcard(ui->functiondigitalframe->parentWidget());
    m_funcdigitalcard->setGeometry(ui->functiondigitalframe->geometry());
    QObject::connect(m_funcdigitalcard, &digitalcard::clicked, this, [this](quint8 ch,bool switchs) {
        quint8 func = switchs ? 0x01 : 0x00;
        to_Channel(ch, 0x01, func, "");
    });
    delete ui->functiondigitalframe;

    m_funcbatterycard = new batterycard(ui->functionbatteryframe->parentWidget());
    m_funcbatterycard->setGeometry(ui->functionbatteryframe->geometry());
    QObject::connect(m_funcbatterycard,&batterycard::clicked, this, [this](quint8 ch,bool switchs) {
        quint8 func = switchs ? 0x01 : 0x00;
        to_Channel(ch, 0x01, func, "");
    });
    delete ui->functionbatteryframe;

    m_funcnmbkeycard = new numberkeypad(ui->functionkeyframe->parentWidget());
    m_funcnmbkeycard->setGeometry(ui->functionkeyframe->geometry());
    QObject::connect(m_funcnmbkeycard, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){
            float val = value.toFloat();

            if (m_initalpage == 0){
                // model: 0 - CV ; 1 - CC ; 3 - OVP;
                QByteArray data(4,0);
                qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));
                return to_Channel(m_functioncCh,0x02, m_functioncsetmode, data);
            }else if (m_functioncsetmode == 0){
                ui->socvaluelabel->setText(value);
                m_funcbatterycard->setSocValue(val);
                m_batterycards[m_functioncCh]->setSocValue(val);
            }else{
                ui->capvaluelabel->setText(value);
                m_funcbatterycard->setCapValue(val);
                m_batterycards[m_functioncCh]->setCapValue(val);
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
    connect(m_remoteOverlay, &remoteoverlay::exitRemote, this, []() {ConfigManager::s_remoteSt.store(0);});
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

void Mainwindow::allONrefresh()
{
    static bool allOn{false};

    allOn = !allOn;
    quint8 func = allOn?0x01:0x00;
    to_Channel( 0, 0x01, func, "");
    ui->digitalallONpushButton->setText(allOn? "All - OFF" : "All - ON");
    ui->batteryallONpushButton->setText(allOn? "All - OFF" : "All - ON");
}

// digital / battery to switchs cards or other

void Mainwindow::on_digitalmodepushButton_clicked(){ui->topstackedwidget->setCurrentIndex(1);m_initalpage = 1;}

void Mainwindow::on_batterymodepushButton_clicked(){ui->topstackedwidget->setCurrentIndex(0);m_initalpage = 0;}

void Mainwindow::on_digitalsettingspushButton_clicked(){ui->topstackedwidget->setCurrentIndex(2);m_initalpage = 0;}

void Mainwindow::on_batterysettingspushButton_clicked(){ui->topstackedwidget->setCurrentIndex(2);m_initalpage = 1;}


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

void Mainwindow::on_batteryrowsbackpushButton_clicked()
{
    int total = ui->batterystackedWidget->count();
    int newindex = ui->batterystackedWidget->currentIndex() - 1;

    ui->batterystackedWidget->setCurrentIndex(newindex);
    ui->batteryrowsbackpushButton->setEnabled(newindex > 0);
    ui->batteryrowsnextpushButton->setEnabled(newindex < total- 1);
    ui->batteryrowslabelpushButton->setText(QString("%1/%2").arg(newindex + 1).arg(total));
}

void Mainwindow::on_batteryrowsnextpushButton_clicked()
{
    int total = ui->batterystackedWidget->count();
    int newindex = ui->batterystackedWidget->currentIndex() + 1;

    ui->batterystackedWidget->setCurrentIndex(newindex);
    ui->batteryrowsbackpushButton->setEnabled(newindex > 0);
    ui->batteryrowsnextpushButton->setEnabled(newindex < total- 1);
    ui->batteryrowslabelpushButton->setText(QString("%1/%2").arg(newindex + 1).arg(total));
}


void Mainwindow::on_digitalallONpushButton_clicked(){allONrefresh();}

void Mainwindow::on_digitalallunitpushButton_clicked()
{
    static int range = 0;
    auto it = m_range.upperBound(range);
    if (it == m_range.end()) {it = m_range.begin();}

    range = it.key();
    QString newunit = "All - " + it.value();
    ui->digitalallunitpushButton->setText(newunit);
    to_Channel(0,0x04, 0x0E, QByteArray(1, char(it.key())));
}

void Mainwindow::on_batteryallONpushButton_clicked(){allONrefresh();}

void Mainwindow::on_batteryallmodelpushButton_clicked()
{
    static quint8 currentIndex = 0;

    if (!m_currentModelList.isEmpty()) {
        currentIndex = (currentIndex + 1) % m_currentModelList.size();
        QString batteryModel = m_currentModelList[currentIndex];

        QString modelname = "All - " + batteryModel;
        ui->batteryallmodelpushButton->setText(modelname);

        for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
            it.value()->setModel(currentIndex,m_modelManager->getModel(batteryModel));
            it.value()->setModelValue(batteryModel);
        }
    }
}

// setting page switchs

void Mainwindow::on_settingrowsbackpushButton_clicked(){ui->topstackedwidget->setCurrentIndex(m_initalpage);}

// function page switchs or other

void Mainwindow::on_functionrowsbackpushButton_clicked(){ui->topstackedwidget->setCurrentIndex(m_initalpage);}

void Mainwindow::on_functionsettingspushButton_clicked(){ui->topstackedwidget->setCurrentIndex(2);}

void Mainwindow::on_functionallapplypushButton_clicked()
{
    if (m_initalpage == 0){
        QByteArray data(4,0);
        // model: 0 - CV ; 1 - CC ; 3 - OVP;
        if (ui->cvvaluelabel->text() != ""){
            float val = ui->cvvaluelabel->text().toFloat();
            qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));
            to_Channel(0,0x02, 0, data);
        }
        if (ui->ccvaluelabel->text() != ""){
            float val = ui->ccvaluelabel->text().toFloat();
            qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));
            to_Channel(0,0x02, 1, data);
        }
        if (ui->ovpvaluelabel->text() != ""){
            float val = ui->ovpvaluelabel->text().toFloat();
            qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));
            to_Channel(0,0x02, 3, data);
        }
    }else{
        if (ui->socvaluelabel->text() != ""){
            float val = ui->socvaluelabel->text().toFloat();
            for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
                it.value()->setSocValue(val);
            }
        }
        if (ui->capvaluelabel->text() != ""){
            float val = ui->capvaluelabel->text().toFloat();
            for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
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
    int range = m_funcdigitalcard->getChRange();

    auto it = m_range.upperBound(range);
    if (it == m_range.end()) {it = m_range.begin();}
    to_Channel(m_functioncCh,0x04, 0x0E, QByteArray(1, char(it.key())));
}


void Mainwindow::on_socradioButton_clicked(){m_functioncsetmode = 0;}

void Mainwindow::on_capradioButton_clicked(){m_functioncsetmode = 1;}

void Mainwindow::on_functionmodelpushButton_clicked()
{
    quint8 currentIndex = m_funcbatterycard->getChmodelindex();

    if (!m_currentModelList.isEmpty()) {
        currentIndex = (currentIndex + 1) % m_currentModelList.size();
        QString batteryModel = m_currentModelList[currentIndex];

        ui->batteryallmodelpushButton->setText(batteryModel);
        m_batterycards[m_functioncCh]->setModelValue(batteryModel);
        m_batterycards[m_functioncCh]->setModel(currentIndex,m_modelManager->getModel(batteryModel));
    }
}

