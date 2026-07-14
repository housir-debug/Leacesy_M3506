#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QScroller>
#include <QTimer>

Q_LOGGING_CATEGORY(widget, "WIDGET:")

/*m_IPaddress = ConfigManager::s_IP;
m_SM = ConfigManager::s_SM;
m_GPIBid = ConfigManager::s_GPIBid;
m_CANid = ConfigManager::s_CANid;*/

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
                    ui->ipvaluelabel->setText(value);

                    ConfigManager::getNetworkConfig();
                    ui->iplabel->setText(ConfigManager::s_IP);
                    return;
                }
                case 1:{// SM
                    ConfigManager::setinterfaces(true,ConfigManager::s_IP,value,ConfigManager::s_Gateway);
                    ui->maskvaluelabel->setText(value);

                    ConfigManager::getNetworkConfig();
                    ui->masklabel->setText(ConfigManager::s_SM);
                    return;
                }
                case 2:{// gateway
                    ConfigManager::setinterfaces(true,ConfigManager::s_IP,ConfigManager::s_SM,value);
                    ui->gatevaluelabel->setText(value);

                    ConfigManager::getNetworkConfig();
                    ui->gatelabel->setText(ConfigManager::s_Gateway);
                    return;
                }
                case 3:{// canid
                    emit to_CANid(value);
                    ConfigManager::setConfigValue("Device/CANID",value);
                    ui->canidvaluelabel->setText(value);
                    ConfigManager::s_CANid = value;
                    ui->canidlabel->setText(value);
                    return;
                }
            }
        }
    });

    m_remoteOverlay = new remoteoverlay(this);
    connect(m_remoteOverlay, &remoteoverlay::exitRemote, this, []() {
       // m_remoteOverlay->hide();
        ConfigManager::s_remoteSt.store(0);
    });

    m_digitalModel = new QStandardItemModel(this);
    ui->digitallistview->setModel(m_digitalModel);
    m_batteryModel = new QStandardItemModel(this);
    ui->batterylistview->setModel(m_batteryModel);

    // listview width = screen width
    /*
    QScroller::grabGesture(ui->digitallistview->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(ui->digitallistview->viewport());

    QScrollerProperties properties;
    // Maximum speed,default 0.5,more small [stop]distance more near
    properties.setScrollMetric(QScrollerProperties::MaximumVelocity, 0.81);
    // Sliding start distance,default 0.1,more small [react]more faster
    properties.setScrollMetric(QScrollerProperties::DragStartDistance, 0.0036);
    // Deceleration rate,default 0.15,more small [stop]more slowly
    properties.setScrollMetric(QScrollerProperties::DecelerationFactor, 0.81);
    // Speed following,default 0.6,more small [react]more faster
    properties.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 0.81);

    scroller->setScrollerProperties(properties);
    */

    // test digital card view
    for (int i = 1; i <= 36; ++i) {
        QString chname = QString("CH-%1").arg(i, 2, 10, QChar('0'));

        // digitalcard
        digitalcard *digitcard = new digitalcard(this);
        digitcard->setChannelName(chname);
        digitcard->setChannel(i);

        QStandardItem *digititem = new QStandardItem();
        digititem->setSizeHint(QSize(CARD_WIDTH, CARD_HEIGHT));
        m_digitalModel->appendRow(digititem);
        m_digitalcards[i] = digitcard;

        QModelIndex digitindex = m_digitalModel->indexFromItem(digititem);
        ui->digitallistview->setIndexWidget(digitindex, digitcard);

        QObject::connect(digitcard, &digitalcard::clicked, this, [this](quint8 ch,bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        QObject::connect(digitcard,&digitalcard::longPressed,this,[this,chname](quint8 ch){
            m_initalpage = 0;
            m_functioncCh = ch;

            m_vercard->setChannelName(chname);
            m_vercard->setChSWVersion(m_ChsoftVer[ch]);
            m_vercard->setChHWVersion(m_ChhardVer[ch]);

            bool chstatus = m_digitalcards[ch]->getChstatus();
            m_funcdigitalcard->setChannelState(chstatus);
            m_funcdigitalcard->setChannelName(chname);
            m_funcdigitalcard->setChannel(ch);

            ui->cvvaluelabel->setText("");
            ui->ccvaluelabel->setText("");
            ui->ovpvaluelabel->setText("");

            ui->cvradioButton->setChecked(true);
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
            ui->functionstackedWidget->setCurrentIndex(0); // function - digital
        });

        // batterycard
        batterycard *battercard = new batterycard(this);
        battercard->setChannelName(chname);
        battercard->setChannel(i);

        QStandardItem *batteritem = new QStandardItem();
        batteritem->setSizeHint(QSize(CARD_WIDTH, CARD_HEIGHT));
        m_batteryModel->appendRow(batteritem);
        m_batterycards[i] = battercard;

        QModelIndex batterindex = m_batteryModel->indexFromItem(batteritem);
        ui->batterylistview->setIndexWidget(batterindex, battercard);

        QObject::connect(battercard, &batterycard::clicked, this,  [this](quint8 ch,bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        QObject::connect(battercard,&batterycard::longPressed,this,[this,chname](quint8 ch){
            m_initalpage = 1;
            m_functioncCh = ch;

            m_vercard->setChannelName(chname);
            m_vercard->setChSWVersion(m_ChsoftVer[ch]);
            m_vercard->setChHWVersion(m_ChhardVer[ch]);

            bool chstatus = m_batterycards[ch]->getChstatus();
            m_funcbatterycard->setChannelState(chstatus);
            m_funcbatterycard->setChannelName(chname);
            m_funcbatterycard->setChannel(ch);

            ui->socvaluelabel->setText("");
            ui->capvaluelabel->setText("");

            ui->socradioButton->setChecked(true);
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
            ui->functionstackedWidget->setCurrentIndex(1); // function - battery
        });
    }
}

// auto add exist channel card

void Mainwindow::update_SoftVer(int ch,const QString &ver){
    m_ChsoftVer[ch] = ver;

    if (!m_digitalcards.contains(ch)) {
        QString chname = QString("CH-%1").arg(ch, 2, 10, QChar('0'));

        digitalcard *card = new digitalcard(this);
        card->setChannelName(chname);
        card->setChannel(ch);

        QStandardItem *item = new QStandardItem();
        item->setSizeHint(QSize(CARD_WIDTH, CARD_HEIGHT));
        m_digitalModel->appendRow(item);
        m_digitalcards[ch] = card;

        QModelIndex index = m_digitalModel->indexFromItem(item);
        ui->digitallistview->setIndexWidget(index, card);

        QObject::connect(card, &digitalcard::clicked, this, [this](quint8 ch,bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        QObject::connect(card,&digitalcard::longPressed,this,[this,chname](quint8 ch){
            m_initalpage = 0;
            m_functioncCh = ch;

            m_vercard->setChannelName(chname);
            m_vercard->setChSWVersion(m_ChsoftVer[ch]);
            m_vercard->setChHWVersion(m_ChhardVer[ch]);

            bool chstatus = m_digitalcards[ch]->getChstatus();
            m_funcdigitalcard->setChannelState(chstatus);
            m_funcdigitalcard->setChannelName(chname);
            m_funcdigitalcard->setChannel(ch);

            ui->cvvaluelabel->setText("");
            ui->ccvaluelabel->setText("");
            ui->ovpvaluelabel->setText("");

            ui->cvradioButton->setChecked(true);
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
            ui->functionstackedWidget->setCurrentIndex(0); // function - digital
        });
    }
}

void Mainwindow::update_HardVer(int ch,const QString &ver){
    m_ChhardVer[ch] = ver;

    if (!m_batterycards.contains(ch)) {
        QString chname = QString("CH-%1").arg(ch, 2, 10, QChar('0'));

        batterycard *card = new batterycard(this);
        card->setChannelName(chname);
        card->setChannel(ch);

        QStandardItem *item = new QStandardItem();
        item->setSizeHint(QSize(CARD_WIDTH, CARD_HEIGHT));
        m_batteryModel->appendRow(item);
        m_batterycards[ch] = card;

        QModelIndex index = m_batteryModel->indexFromItem(item);
        ui->batterylistview->setIndexWidget(index, card);

        QObject::connect(card, &batterycard::clicked, this, [this](quint8 ch,bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        QObject::connect(card,&batterycard::longPressed,this,[this,chname](quint8 ch){
            m_initalpage = 1;
            m_functioncCh = ch;

            m_vercard->setChannelName(chname);
            m_vercard->setChSWVersion(m_ChsoftVer[ch]);
            m_vercard->setChHWVersion(m_ChhardVer[ch]);

            bool chstatus = m_batterycards[ch]->getChstatus();
            m_funcbatterycard->setChannelState(chstatus);
            m_funcbatterycard->setChannelName(chname);
            m_funcbatterycard->setChannel(ch);

            ui->socvaluelabel->setText("");
            ui->capvaluelabel->setText("");

            ui->socradioButton->setChecked(true);
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
            ui->functionstackedWidget->setCurrentIndex(1); // function - battery
        });
    }
}

void Mainwindow::load_BatteryModel(){
    if (m_modelManager) {
        QTimer::singleShot(0, this, [this]() {
            if (m_modelManager->loadAllModels()) {
                m_currentModelList = m_modelManager->getAvailableModels();

                if (!m_currentModelList.isEmpty()) {
                    QString batteryModel = m_currentModelList[0];

                    for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
                        it.value()->setModelValue(batteryModel);
                        it.value()->setModel(0,m_modelManager->getModel(batteryModel));
                    }
                }
            }
        });
    }
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
    delete ui;
}

// update digital card information

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

// ************************************other

/*QJsonArray Mainwindow::getAllChannelsData() {
    QJsonArray channels;
    #define CHANNEL(n) \
        do { \
            if (mCH##n##_sv != "0.0.0.0") { \
                QJsonObject channel; \
                channel["channel"] = n; \
                channel["isOutput"] = mCH##n##_isOutput.load(); \
                channel["voltage"] = mCH##n##_Voltage.load(); \
                channel["current"] = mCH##n##_Current.load(); \
                channel["current_unit"] = mCH##n##_CurrentUnit; \
                channel["cvSetpoint"] = mCH##n##_cv.load(); \
                channel["ccSetpoint"] = mCH##n##_cc.load(); \
                channel["ovSetpoint"] = mCH##n##_ovp.load(); \
                channel["status"] = mCH##n##_Status; \
                channels.append(channel); \
        }} while(0);

    CHANNEL_COUNT
    #undef CHANNEL
    return channels;
}*/

// *********************************************

// Auxiliary function

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
        default:
            return;
    }
}

// channel card switchs

void Mainwindow::on_digitalsettingspushButton_clicked()
{
    m_initalpage = 0;
    ui->topstackedwidget->setCurrentIndex(2);  // settingspage

    ConfigManager::getNetworkConfig();
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

void Mainwindow::on_batterysettingspushButton_clicked()
{
    m_initalpage = 1;
    ui->topstackedwidget->setCurrentIndex(2);  // settingspage

    ConfigManager::getNetworkConfig();
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


void Mainwindow::on_digitalrowsbackpushButton_clicked()
{
    int itemHeight = CARD_HEIGHT + ui->digitallistview->spacing();

    QScrollBar *vbar = ui->digitallistview->verticalScrollBar();
    int totalRows = (vbar->maximum() / itemHeight) + 1;
    int currentRow = vbar->value() / itemHeight;

    int newRow = qMax(0, currentRow - 1);   // 1 page 1 rows
    QModelIndex targetIndex = m_digitalModel->index(newRow * 6, 0); // 1 rows / 6 cards
    ui->digitallistview->scrollTo(targetIndex, QAbstractItemView::PositionAtTop);

    ui->digitalrowslabelpushButton->setText(QString("%1 / %2").arg(newRow + 1).arg(totalRows));
    ui->digitalrowsnextpushButton->setEnabled(newRow < totalRows - 1);
    ui->digitalrowsbackpushButton->setEnabled(newRow > 0);
}

void Mainwindow::on_digitalrowsnextpushButton_clicked()
{
    int itemHeight = CARD_HEIGHT + ui->digitallistview->spacing();

    QScrollBar *vbar = ui->digitallistview->verticalScrollBar();
    int totalRows = (vbar->maximum() / itemHeight) + 1;
    int currentRow = vbar->value() / itemHeight;

    int newRow = qMin(totalRows - 1, currentRow + 1);   // 1 page 1 rows
    QModelIndex targetIndex = m_digitalModel->index(newRow * 6, 0); // 1 rows / 6 cards
    ui->digitallistview->scrollTo(targetIndex, QAbstractItemView::PositionAtTop);

    ui->digitalrowslabelpushButton->setText(QString("%1 / %2").arg(newRow + 1).arg(totalRows));
    ui->digitalrowsnextpushButton->setEnabled(newRow < totalRows - 1);
    ui->digitalrowsbackpushButton->setEnabled(newRow > 0);
}

void Mainwindow::on_batteryrowsbackpushButton_clicked()
{
    int itemHeight = CARD_HEIGHT + ui->batterylistview->spacing();

    QScrollBar *vbar = ui->batterylistview->verticalScrollBar();
    int totalRows = (vbar->maximum() / itemHeight) + 1;
    int currentRow = vbar->value() / itemHeight;

    int newRow = qMax(0, currentRow - 1);   // 1 page 1 rows
    QModelIndex targetIndex = m_batteryModel->index(newRow * 6, 0); // 1 rows / 6 cards
    ui->batterylistview->scrollTo(targetIndex, QAbstractItemView::PositionAtTop);

    ui->batteryrowslabelpushButton->setText(QString("%1/%2").arg(newRow + 1).arg(totalRows));
    ui->batteryrowsnextpushButton->setEnabled(newRow < totalRows - 1);
    ui->batteryrowsbackpushButton->setEnabled(newRow > 0);
}

void Mainwindow::on_batteryrowsnextpushButton_clicked()
{
    int itemHeight = CARD_HEIGHT + ui->batterylistview->spacing();

    QScrollBar *vbar = ui->batterylistview->verticalScrollBar();
    int totalRows = (vbar->maximum() / itemHeight) + 1;
    int currentRow = vbar->value() / itemHeight;

    int newRow = qMin(totalRows - 1, currentRow + 1);   // 1 page 1 rows
    QModelIndex targetIndex = m_batteryModel->index(newRow * 6, 0); // 1 rows / 6 cards
    ui->batterylistview->scrollTo(targetIndex, QAbstractItemView::PositionAtTop);

    ui->batteryrowslabelpushButton->setText(QString("%1/%2").arg(newRow + 1).arg(totalRows));
    ui->batteryrowsnextpushButton->setEnabled(newRow < totalRows - 1);
    ui->batteryrowsbackpushButton->setEnabled(newRow > 0);
}


void Mainwindow::on_digitalmodepushButton_clicked()
{
    m_initalpage = 1;
    load_BatteryModel();
    ui->topstackedwidget->setCurrentIndex(1);  // batterypage
}

void Mainwindow::on_batterymodepushButton_clicked()
{
    m_initalpage = 0;
    ui->topstackedwidget->setCurrentIndex(0);  // digitalpage
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

    QByteArray Unit_buffer;
    Unit_buffer.append(char(unitCode));
    to_Channel(0,0x04, 0x0E, Unit_buffer);

    QString newunit = "All - " + unit;
    ui->digitalallunitpushButton->setText(newunit);
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

        for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
            it.value()->setModelValue(batteryModel);
            it.value()->setModel(currentIndex,m_modelManager->getModel(batteryModel));
        }

        QString modelname = "All - " + batteryModel;
        ui->batteryallmodelpushButton->setText(modelname);
    }
}

// function page switchs

void Mainwindow::on_functionsettingspushButton_clicked()
{
    ui->topstackedwidget->setCurrentIndex(2);  // settingspage
}

void Mainwindow::on_functionrowsbackpushButton_clicked()
{
    ui->topstackedwidget->setCurrentIndex(m_initalpage);
}

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
    quint8 step = m_digitalcards[m_functioncCh]->getChRange();
    step = (step + 1) % 3;
    quint8 unitCode;

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
        quint8 currentIndex = m_batterycards[m_functioncCh]->getChmodel();

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
    ConfigManager::setinterfaces(false,"","","");

    ui->ipradioButton->setEnabled(false);
    ui->maskradioButton->setEnabled(false);
    ui->gateradioButton->setEnabled(false);

    ui->canidradioButton->setChecked(true);
    m_setmode = 3;

    ConfigManager::getNetworkConfig();
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

void Mainwindow::on_settingrowsbackpushButton_clicked()
{
    ui->topstackedwidget->setCurrentIndex(m_initalpage);
}
