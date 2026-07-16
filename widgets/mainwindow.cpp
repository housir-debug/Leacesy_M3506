#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QtCore>

Q_LOGGING_CATEGORY(widget, "WIDGET:")

QJsonArray Mainwindow::getAllChannelsData() {
    QJsonArray channels;
    #define CHANNEL(n) \
        do { \
            if (m_digitalcards.contains(n)) { \
                QJsonObject channel; \
                channel["channel"] = n; \
                channel["isOutput"] = m_digitalcards[n]->getChstatus(); \
                channel["voltage"] = m_digitalcards[n]->getChVoltage(); \
                channel["current"] = m_digitalcards[n]->getChCurrent(); \
                channel["current_unit"] = m_digitalcards[n]->getChunit(); \
                channel["cvSetpoint"] = m_digitalcards[n]->getChCvValue(); \
                channel["cvstatus"] = m_digitalcards[n]->getChCVChecked(); \
                channel["ccSetpoint"] = m_digitalcards[n]->getChCcValue(); \
                channel["ccstatus"] = m_digitalcards[n]->getChCCChecked(); \
                channel["ovSetpoint"] = m_digitalcards[n]->getChOvpValue(); \
                channel["ovpstatus"] = m_digitalcards[n]->getChOVPChecked(); \
                channels.append(channel); \
        }} while(0);

    CHANNEL_COUNT
    #undef CHANNEL
    return channels;
}

void Mainwindow::load_BatteryModel(){
    if (m_modelManager) {
        QTimer::singleShot(0, this, [this]() {
            if (m_modelManager->loadAllModels()) {
                m_currentModelList = m_modelManager->getAvailableModels();

                if (!m_currentModelList.isEmpty()) {
                    QString batteryModel = m_currentModelList[0];

                    for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
                        if (!it.value()->getChstatus()){
                            it.value()->setModelValue(batteryModel);
                            it.value()->setModel(0,m_modelManager->getModel(batteryModel));
                        }
                    }
                }
            }
        });
    }
}


Mainwindow::Mainwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Mainwindow)
{
    ui->setupUi(this);

    QRect verrect = ui->functionversionframe->geometry();
    m_vercard = new versionview(ui->functionversionframe->parentWidget());
    m_vercard->setGeometry(verrect);
    delete ui->functionversionframe;

    QRect chstatusrect = ui->functionchstatusframe->geometry();
    m_chstatuscard = new chstatusview(ui->functionchstatusframe->parentWidget());
    m_chstatuscard->setGeometry(chstatusrect);
    delete ui->functionchstatusframe;

    QRect numberrect = ui->functionkeyframe->geometry();
    m_funcnmbkeycard = new numberkeypad(ui->functionkeyframe->parentWidget());
    m_funcnmbkeycard->setGeometry(numberrect);
    delete ui->functionkeyframe;
    QObject::connect(m_funcnmbkeycard, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){
            float val = value.toFloat();

            if (m_initalpage == 0){
                // model: 0 - CV ; 1 - CC ; 3 - OVP;
                QByteArray buffer(reinterpret_cast<const char*>(&val), sizeof(float));
                std::reverse(buffer.begin(), buffer.end()); // big-endian order

                return to_Channel(m_functioncCh,0x02, m_functioncsetmode, buffer);
            }else{
                if (m_functioncsetmode == 0){
                    ui->socvaluelabel->setText(value);
                    m_funcbatterycard->setSocValue(val);
                    m_batterycards[m_functioncCh]->setSocValue(val);
                }else{
                    ui->capvaluelabel->setText(value);
                    m_funcbatterycard->setCapValue(val);
                    m_batterycards[m_functioncCh]->setCapValue(val);
                }
            }
        }
    });

    QRect digitalcardrect = ui->functiondigitalframe->geometry();
    m_funcdigitalcard = new digitalcard(ui->functiondigitalframe->parentWidget());
    m_funcdigitalcard->setGeometry(digitalcardrect);
    delete ui->functiondigitalframe;
    QObject::connect(m_funcdigitalcard, &digitalcard::clicked, this, [this](quint8 ch,bool switchs) {
        quint8 func = switchs ? 0x01 : 0x00;
        to_Channel(ch, 0x01, func, "");
    });

    QRect batterycardrect = ui->functionbatteryframe->geometry();
    m_funcbatterycard = new batterycard(ui->functionbatteryframe->parentWidget());
    m_funcbatterycard->setGeometry(batterycardrect);
    delete ui->functionbatteryframe;
    QObject::connect(m_funcbatterycard,&batterycard::clicked, this, [this](quint8 ch,bool switchs) {
        quint8 func = switchs ? 0x01 : 0x00;
        to_Channel(ch, 0x01, func, "");
    });

    QRect setnumberrect = ui->settingkeyframe->geometry();
    m_setnmbkeycard = new numberkeypad(ui->settingkeyframe->parentWidget());
    m_setnmbkeycard->setGeometry(setnumberrect);
    delete ui->settingkeyframe;
    QObject::connect(m_setnmbkeycard, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){
            switch(m_setmode){
                case 0:{// IP
                    ConfigManager::setinterfaces(true,value,ConfigManager::s_SM,ConfigManager::s_Gateway);
                    ui->iplabel->setText(ConfigManager::s_IP);
                    ui->ipvaluelabel->setText(value);
                    return;
                }
                case 1:{// SM
                    ConfigManager::setinterfaces(true,ConfigManager::s_IP,value,ConfigManager::s_Gateway);
                    ui->masklabel->setText(ConfigManager::s_SM);
                    ui->maskvaluelabel->setText(value);
                    return;
                }
                case 2:{// gateway
                    ConfigManager::setinterfaces(true,ConfigManager::s_IP,ConfigManager::s_SM,value);
                    ui->gatelabel->setText(ConfigManager::s_Gateway);
                    ui->gatevaluelabel->setText(value);
                    return;
                }
                case 3:{// canid
                    ConfigManager::setConfigValue("Device/CANID",value);
                    ui->canidvaluelabel->setText(value);
                    ConfigManager::s_CANid = value;
                    ui->canidlabel->setText(value);
                    emit to_CANid(value);
                    return;
                }
                default:return;
            }
        }
    });

    m_remoteOverlay = new remoteoverlay(this);
    connect(m_remoteOverlay, &remoteoverlay::exitRemote, this, []() {
        ConfigManager::s_remoteSt.store(0);
    });
}

Mainwindow::~Mainwindow()
{
    for (auto it = m_digitalcards.cbegin(); it != m_digitalcards.cend(); ++it) {
        it.value()->deleteLater();
    }

    for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
        it.value()->deleteLater();
    }

    m_digitalcards.clear();
    m_batterycards.clear();

    m_vercard->deleteLater();
    m_chstatuscard->deleteLater();
    m_funcnmbkeycard->deleteLater();
    m_funcdigitalcard->deleteLater();
    m_funcbatterycard->deleteLater();
    m_setnmbkeycard->deleteLater();
    m_remoteOverlay->deleteLater();
    delete ui;
}


// auto add exist channel card

bool Mainwindow::update_cardtest()
{
    //QList<quint8> channels = {3, 1, 4,2,8,9,24,15,32,12,18};
    QList<quint8> channels;
    for (int i = 1; i <= 36; i++) {
        channels.append(static_cast<quint8>(i));
    }
    //QList<quint8> channels = {1,2,3,4,5};
    std::sort(channels.begin(), channels.end());

    int totalChannels = channels.size();
    int neededPages = (totalChannels + 5) / 6;  // Round up

    add_digitalcard(neededPages,channels);
    add_batterycard(neededPages,channels);

    return true;
}

bool Mainwindow::update_showcard()
{
    if (m_ChsoftVer.size() != 0 && m_ChsoftVer.size() == m_ChhardVer.size()){
        QList<quint8> channels = m_ChsoftVer.keys();
        std::sort(channels.begin(), channels.end());

        int totalChannels = channels.size();
        int neededPages = (totalChannels + 5) / 6;  // Round up

        add_digitalcard(neededPages,channels);
        add_batterycard(neededPages,channels);

        return true;
    }

    qCDebug(widget)<<"[update_showcard]channel count = zero or channel ver error";
    return false;
}

void Mainwindow::add_digitalcard(int neededPages,const QList<quint8>& channels)
{
    while (ui->digitalstackedWidget->count() < neededPages) {
        QWidget* page = new QWidget();
        QFrame* frame = new QFrame(page);
        frame->setGeometry(0, 0, 1280, 320);

        QHBoxLayout* layout = new QHBoxLayout(frame);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(2);
        frame->setLayout(layout);

        ui->digitalstackedWidget->addWidget(page);
    }

    for (int idx = 0; idx < channels.size(); idx++) {
        int ch = channels[idx];
        int pageIndex = idx / 6;

        QWidget* page = ui->digitalstackedWidget->widget(pageIndex);
        QFrame* frame = page->findChild<QFrame*>();
        QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(frame->layout());

        digitalcard* card = new digitalcard(frame);
        card->setChannel(ch);

        connect(card, &digitalcard::clicked, this, [this](quint8 ch, bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        connect(card, &digitalcard::longPressed, this, [this](quint8 ch) {
            m_initalpage = 0;
            m_functioncCh = ch;

            m_vercard->setChSWVersion(m_ChsoftVer.value(ch));
            m_vercard->setChHWVersion(m_ChhardVer.value(ch));
            QString chname = QString("CH-%1").arg(ch, 2, 10, QChar('0'));
            m_vercard->setChannelName(chname);

            m_funcdigitalcard->setChannel(ch);
            m_funcdigitalcard->setChannelState(m_digitalcards[ch]->getChstatus());
            m_funcdigitalcard->setCvValue(m_digitalcards[ch]->getChCvValue());
            m_funcdigitalcard->setCcValue(m_digitalcards[ch]->getChCcValue());
            m_funcdigitalcard->setOvpValue(m_digitalcards[ch]->getChOvpValue());

            ui->cvvaluelabel->setText("");
            ui->ccvaluelabel->setText("");
            ui->ovpvaluelabel->setText("");

            ui->cvradioButton->setChecked(true);
            ui->topstackedwidget->setCurrentIndex(3);
            ui->functionstackedWidget->setCurrentIndex(0);
        });

        m_digitalcards[ch] = card;
        layout->addWidget(card);
    }

    ui->digitalstackedWidget->setCurrentIndex(0);

    ui->digitalrowsbackpushButton->setEnabled(false);
    ui->digitalrowsnextpushButton->setEnabled(0 < neededPages - 1);
    ui->digitalrowslabelpushButton->setText(QString("%1 / %2").arg(1).arg(neededPages));
}

void Mainwindow::add_batterycard(int neededPages,const QList<quint8>& channels)
{
    while (ui->batterystackedWidget->count() < neededPages) {
        QWidget* page = new QWidget();
        QFrame* frame = new QFrame(page);
        frame->setGeometry(0, 0, 1280, 320);

        QHBoxLayout* layout = new QHBoxLayout(frame);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(2);
        frame->setLayout(layout);

        ui->batterystackedWidget->addWidget(page);
    }

    for (int idx = 0; idx < channels.size(); idx++) {
        int ch = channels[idx];
        int pageIndex = idx / 6;

        QWidget* page = ui->batterystackedWidget->widget(pageIndex);
        QFrame* frame = page->findChild<QFrame*>();
        QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(frame->layout());

        batterycard* card = new batterycard(frame);
        card->setChannel(ch);

        connect(card, &batterycard::clicked, this, [this](quint8 ch, bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        connect(card, &batterycard::longPressed, this, [this](quint8 ch) {
            m_initalpage = 1;
            m_functioncCh = ch;

            m_vercard->setChSWVersion(m_ChsoftVer.value(ch));
            m_vercard->setChHWVersion(m_ChhardVer.value(ch));
            QString chname = QString("CH-%1").arg(ch, 2, 10, QChar('0'));
            m_vercard->setChannelName(chname);

            m_funcbatterycard->setChannel(ch);
            m_funcbatterycard->setChannelState(m_batterycards[ch]->getChstatus());
            m_funcbatterycard->setSocValue(m_batterycards[ch]->getChSOCvalue());
            m_funcbatterycard->setOcvValue(m_batterycards[ch]->getChOcvvalue());
            m_funcbatterycard->setEsrValue(m_batterycards[ch]->getChEsrvalue());
            m_funcbatterycard->setCapValue(m_batterycards[ch]->getChCapvalue());
            m_funcbatterycard->setModelValue(m_batterycards[ch]->getChmodelname());

            ui->socvaluelabel->setText("");
            ui->capvaluelabel->setText("");

            ui->socradioButton->setChecked(true);
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
            ui->functionstackedWidget->setCurrentIndex(1); // function - battery
        });

        m_batterycards[ch] = card;
        layout->addWidget(card);
    }

    ui->batterystackedWidget->setCurrentIndex(0);

    ui->batteryrowsbackpushButton->setEnabled(false);
    ui->batteryrowsnextpushButton->setEnabled(0 < neededPages - 1);
    ui->batteryrowslabelpushButton->setText(QString("%1 / %2").arg(1).arg(neededPages));
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

// update digital card information

void Mainwindow::update_SoftVer(int ch,const QString &ver){
    if (!m_ChsoftVer.contains(ch)){
        m_ChsoftVer[ch] = ver;
    }
}

void Mainwindow::update_HardVer(int ch,const QString &ver){
    if (!m_ChhardVer.contains(ch)){
        m_ChhardVer[ch] = ver;
    }
}

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
    if (m_digitalcards.contains(ch) && m_initalpage == 0) {
        m_digitalcards[ch]->setVoltage(voltage);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
            m_funcdigitalcard->setVoltage(voltage);
        }
    }

    if (m_batterycards.contains(ch) && m_batterycards[ch]->getChstatus()){
        float newocv = m_batterycards[ch]->getcurrentOCV();
        QByteArray ocvbuffer(reinterpret_cast<const char*>(&newocv), sizeof(float));
        std::reverse(ocvbuffer.begin(), ocvbuffer.end());
        to_Channel(ch,0x02, 0x00, ocvbuffer);

        float newesr = m_batterycards[ch]->getcurrentESR();
        QByteArray esrbuffer(reinterpret_cast<const char*>(&newesr), sizeof(float));
        std::reverse(esrbuffer.begin(), esrbuffer.end());
        to_Channel(ch,0x02, 0x02, esrbuffer);
    }
}

void Mainwindow::update_CurrentAndUnit(int ch,float current){
    if (m_digitalcards.contains(ch) && m_initalpage == 0) {
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
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setCvValue(cv);
    }

    if (m_batterycards.contains(ch)){
        m_batterycards[ch]->setOcvValue(cv);
    }

    if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
        if (m_initalpage == 0){
            m_funcdigitalcard->setCvValue(cv);

            QString text = QString::number(cv, 'f', 3);
            ui->cvvaluelabel->setText(text);
        }else{
            m_funcbatterycard->setOcvValue(cv);
        }
    }
}


void Mainwindow::update_Range(int ch,quint8 range)
{
    if (m_digitalcards.contains(ch) && m_initalpage == 0) {
        quint8 unitCode;QString unit;
        switch (range) {
            case 0x01:  unitCode = 0;unit = "mA";   break;
            case 0x10:  unitCode = 1;unit = "Auto"; break;
            case 0x00:  unitCode = 2;unit = "A";    break;
            default:    unitCode = 0;unit = "A";    break;
        }
        m_digitalcards[ch]->setChannelRange(unitCode);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh){
            m_funcdigitalcard->setChannelRange(unitCode);
            ui->functionunitpushButton->setText(unit);
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

void Mainwindow::update_Cc(int ch,float cc){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setCcValue(cc);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh && m_initalpage == 0){
            m_funcdigitalcard->setCcValue(cc);

            QString text = QString::number(cc, 'f', 3);
            ui->ccvaluelabel->setText(text);
        }
    }
}

void Mainwindow::update_Ovp(int ch,float ovp){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setOvpValue(ovp);

        if (ui->topstackedwidget->currentIndex() == 3 && ch == m_functioncCh && m_initalpage == 0){
            m_funcdigitalcard->setOvpValue(ovp);

            QString text = QString::number(ovp, 'f', 3);
            ui->ovpvaluelabel->setText(text);
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

void Mainwindow::update_remotemodel(quint8 reface){
    if (ConfigManager::s_remoteSt.load() != reface){
        ConfigManager::s_remoteSt.store(reface);

        if (reface == 0){
            m_remoteOverlay->hide();
        }else{
            m_remoteOverlay->show();
        }
    }
}

// to setting page and return or digital - battery

void Mainwindow::on_digitalsettingspushButton_clicked()
{
    refresh_settingpage();
    m_initalpage = 0;
}

void Mainwindow::on_batterysettingspushButton_clicked()
{
    refresh_settingpage();
    m_initalpage = 1;
}

void Mainwindow::on_functionsettingspushButton_clicked()
{
    refresh_settingpage();
}

void Mainwindow::refresh_settingpage()
{
    ui->topstackedwidget->setCurrentIndex(2);  // settingspage

    if (ConfigManager::s_isDHCP){
        ui->dhcpradioButton->setChecked(true);

        ui->ipradioButton->setEnabled(false);
        ui->maskradioButton->setEnabled(false);
        ui->gateradioButton->setEnabled(false);

        ui->canidradioButton->setChecked(true);
        m_setmode = 3;
    }else{
        ui->setstaticradioButton->setChecked(true);

        ui->ipradioButton->setEnabled(true);
        ui->maskradioButton->setEnabled(true);
        ui->gateradioButton->setEnabled(true);

        ui->ipradioButton->setChecked(true);
        m_setmode = 0;
    }

    ui->iplabel->setText(ConfigManager::s_IP);
    ui->masklabel->setText(ConfigManager::s_SM);
    ui->gatelabel->setText(ConfigManager::s_Gateway);
    ui->maclabel->setText(ConfigManager::s_MAC);
    ui->ipvaluelabel->setText("");
    ui->maskvaluelabel->setText("");
    ui->gatevaluelabel->setText("");

    ui->canidlabel->setText(ConfigManager::s_CANid);
    ui->gpibidlabel->setText(ConfigManager::s_GPIBid);
    ui->snlabel->setText(ConfigManager::s_serialNumber);
    ui->canidvaluelabel->setText("");
    ui->gpibvaluelabel->setText("");
}


void Mainwindow::on_functionrowsbackpushButton_clicked()
{
    ui->topstackedwidget->setCurrentIndex(m_initalpage);
}

void Mainwindow::on_settingrowsbackpushButton_clicked()
{
    ui->topstackedwidget->setCurrentIndex(m_initalpage);
}


void Mainwindow::on_digitalmodepushButton_clicked()
{
    ui->topstackedwidget->setCurrentIndex(1);  // batterypage
    m_initalpage = 1;
}

void Mainwindow::on_batterymodepushButton_clicked()
{
    ui->topstackedwidget->setCurrentIndex(0);  // digitalpage
    m_initalpage = 0;
}

// digital / battery to switchs cards or other

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


void Mainwindow::on_digitalallONpushButton_clicked()
{
    static bool allOn{false};

    allOn = !allOn;
    ui->digitalallONpushButton->setText(allOn? "All - OFF" : "All - ON");

    quint8 func = allOn ? 0x01 : 0x00;
    to_Channel(0, 0x01, func, "");
}

void Mainwindow::on_digitalallunitpushButton_clicked()
{
    quint8 unitCode; QString unit;

    static quint8 step = 0;
    switch (step) {
    case 0:  unitCode = 0x01;unit = "mA";   break;
    case 1:  unitCode = 0x10;unit = "Auto"; break;
    case 2:  unitCode = 0x00;unit = "A";    break;
    default: unitCode = 0x00;unit = "A";    break;
    }
    step = (step + 1) % 3;

    QString newunit = "All - " + unit;
    ui->digitalallunitpushButton->setText(newunit);

    QByteArray Unit_buffer;
    Unit_buffer.append(char(unitCode));
    to_Channel(0,0x04, 0x0E, Unit_buffer);
}

void Mainwindow::on_batteryallONpushButton_clicked()
{
    if (!m_currentModelList.isEmpty()) {
        on_digitalallONpushButton_clicked();
    }
}

void Mainwindow::on_batteryallmodelpushButton_clicked()
{
    static quint8 currentIndex = 0;

    if (!m_currentModelList.isEmpty()) {
        currentIndex = (currentIndex + 1) % m_currentModelList.size();
        QString batteryModel = m_currentModelList[currentIndex];

        QString modelname = "All - " + batteryModel;
        ui->batteryallmodelpushButton->setText(modelname);

        for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
            it.value()->setModelValue(batteryModel);
            it.value()->setModel(currentIndex,m_modelManager->getModel(batteryModel));
        }
    }
}

// function page switchs

void Mainwindow::on_functionallapplypushButton_clicked()
{
    if (m_initalpage == 0){
        // model: 0 - CV ; 1 - CC ; 3 - OVP;
        if (ui->cvvaluelabel->text() != ""){
            float val = ui->cvvaluelabel->text().toFloat();
            QByteArray buffer(reinterpret_cast<const char*>(&val), sizeof(float));
            std::reverse(buffer.begin(), buffer.end()); // big-endian order
            to_Channel(0,0x02, 0, buffer);
        }
        if (ui->ccvaluelabel->text() != ""){
            float val = ui->ccvaluelabel->text().toFloat();
            QByteArray buffer(reinterpret_cast<const char*>(&val), sizeof(float));
            std::reverse(buffer.begin(), buffer.end()); // big-endian order
            to_Channel(0,0x02, 1, buffer);
        }
        if (ui->ovpvaluelabel->text() != ""){
            float val = ui->ovpvaluelabel->text().toFloat();
            QByteArray buffer(reinterpret_cast<const char*>(&val), sizeof(float));
            std::reverse(buffer.begin(), buffer.end()); // big-endian order
            to_Channel(0,0x02, 3, buffer);
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


void Mainwindow::on_cvradioButton_clicked()
{
    m_functioncsetmode = 0;
}

void Mainwindow::on_ccradioButton_clicked()
{
    m_functioncsetmode = 1;
}

void Mainwindow::on_ovpradioButton_clicked()
{
    m_functioncsetmode = 3;
}

void Mainwindow::on_functionunitpushButton_clicked()
{
    quint8 unitCode;
    quint8 step = m_digitalcards[m_functioncCh]->getChRange();

    step = (step + 1) % 3;
    switch (step) {
        case 0:  unitCode = 0x01;break;
        case 1:  unitCode = 0x10;break;
        case 2:  unitCode = 0x00;break;
        default: unitCode = 0x00;break;
    }

    QByteArray Unit_buffer;
    Unit_buffer.append(char(unitCode));
    to_Channel(m_functioncCh,0x04, 0x0E, Unit_buffer);
}


void Mainwindow::on_socradioButton_clicked()
{
    m_functioncsetmode = 0;
}

void Mainwindow::on_capradioButton_clicked()
{
    m_functioncsetmode = 1;
}

void Mainwindow::on_functionmodelpushButton_clicked()
{
    if (m_batterycards.contains(m_functioncCh)){
        quint8 currentIndex = m_batterycards[m_functioncCh]->getChmodelindex();

        if (!m_currentModelList.isEmpty()) {
            currentIndex = (currentIndex + 1) % m_currentModelList.size();
            QString batteryModel = m_currentModelList[currentIndex];

            ui->batteryallmodelpushButton->setText(batteryModel);
            m_batterycards[m_functioncCh]->setModelValue(batteryModel);
            m_batterycards[m_functioncCh]->setModel(currentIndex,m_modelManager->getModel(batteryModel));
        }
    }
}

// setting page switchs

void Mainwindow::on_dhcpradioButton_clicked()
{
    ui->ipradioButton->setEnabled(false);
    ui->maskradioButton->setEnabled(false);
    ui->gateradioButton->setEnabled(false);

    ui->canidradioButton->setChecked(true);
    m_setmode = 3;

    ConfigManager::setinterfaces(false,"","","");
    ui->iplabel->setText(ConfigManager::s_IP);
    ui->masklabel->setText(ConfigManager::s_SM);
    ui->gatelabel->setText(ConfigManager::s_Gateway);
}

void Mainwindow::on_setstaticradioButton_clicked()
{
    ui->ipradioButton->setEnabled(true);
    ui->maskradioButton->setEnabled(true);
    ui->gateradioButton->setEnabled(true);

    ui->ipradioButton->setChecked(true);
    m_setmode = 0;
}


void Mainwindow::on_ipradioButton_clicked()
{
    m_setmode = 0;
    ui->ipradioButton->setChecked(true);
    m_setnmbkeycard->setValue(ConfigManager::s_IP);
}

void Mainwindow::on_maskradioButton_clicked()
{
    m_setmode = 1;
    ui->maskradioButton->setChecked(true);
    m_setnmbkeycard->setValue(ConfigManager::s_SM);
}

void Mainwindow::on_gateradioButton_clicked()
{
    m_setmode = 2;
    ui->gateradioButton->setChecked(true);
    m_setnmbkeycard->setValue(ConfigManager::s_Gateway);
}

void Mainwindow::on_canidradioButton_clicked()
{
    m_setmode = 3;
    ui->canidradioButton->setChecked(true);
}

void Mainwindow::on_gpibidradioButton_clicked()
{
    m_setmode = 4;
    ui->gpibidradioButton->setChecked(true);
}

