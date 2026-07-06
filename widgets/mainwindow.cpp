#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QScroller>
#include <QTimer>

Q_LOGGING_CATEGORY(widget, "WIDGET:")

/*m_SoftVer = ConfigManager::s_firmwareVersion;
m_HardVer = ConfigManager::s_hardwareVersion;

m_IPaddress = ConfigManager::s_IP;
m_SM = ConfigManager::s_SM;
m_GPIBid = ConfigManager::s_GPIBid;
m_CANid = ConfigManager::s_CANid;*/

Mainwindow::Mainwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Mainwindow)
{
    ui->setupUi(this);

    m_model = new QStandardItemModel(this);
    ui->digitallistview->setModel(m_model);

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
        digitalcard *card = new digitalcard(this);
        card->setChannelName(QString("CH-%1").arg(i, 2, 10, QChar('0')));

        QStandardItem *item = new QStandardItem();
        item->setSizeHint(QSize(206, 310));
        m_model->appendRow(item);

        QModelIndex index = m_model->indexFromItem(item);
        ui->digitallistview->setIndexWidget(index, card);
    }
}

void Mainwindow::scrollToRow(int row)
{
    if (row >= 0 && row <= m_totalRows) {
        m_currentRow = row;
        int itemHeight = CARD_HEIGHT + ui->digitallistview->spacing();

        int targetY = row * itemHeight;
        ui->digitallistview->verticalScrollBar()->setValue(targetY);

        ui->RowIndicator->setText(QString("%1/%2").arg(currentPage + 1).arg(totalPages));
        ui->prevRowBtn->setEnabled(row > 0);
        ui->nextRowBtn->setEnabled(row < m_totalRows - 1);
    }
}

Mainwindow::~Mainwindow()
{
    for (auto it = m_numbercards.cbegin(); it != m_numbercards.cend(); ++it) {
        it.value()->deleteLater();
    }

    m_numbercards.clear();
    delete ui;
}

void Mainwindow::update_Configuration(int model,const QString& val){
    switch(model){
        case 0:{
            // IP
            /*if(ConfigManager::refresh_interfaces(val,m_SM)){
                m_IPaddress = val;
                emit ipAdress_Changed();
                ConfigManager::s_IP = val;
            }
            return;
        }
        case 1:{
            // SM
            if(ConfigManager::refresh_interfaces(m_IPaddress,val)){
                m_SM = val;
                emit sm_Changed();
                ConfigManager::s_SM = val;
            }*/
            return;
        }
        case 2:{
            // GPIB
            //m_GPIBid = val;
            //emit gpibId_Changed();
            //ConfigManager::setConfigValue("Device/GPIBID",val);
            //emit to_GPIBid(val);
            return;
        }
        case 3:{
            // can
            emit to_CANid(val);
            ConfigManager::s_CANid = val;
            ConfigManager::setConfigValue("Device/CANID",val);
            return;
        }
    }
}

void Mainwindow::update_remotemodel(quint8 reface){
    Q_UNUSED(reface);
    /*for (auto it = m_numbercards.cbegin(); it != m_numbercards.cend(); ++it) {
        if (it.key() == ch){
            it.value()->setChannelState(status);
        }
    }*/
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
        default:
            return;
    }
}

// auto add exist channel card

void Mainwindow::update_SoftVer(int ch,const QString &ver){
    Q_UNUSED(ver);
    if (!m_numbercards.contains(ch)) {
        QString chName = "CH-" + QString::number(ch);

        digitalcard *card = new digitalcard(this);
        card->setChannelName(chName);
        m_numbercards[ch] = card;

        QStandardItem *item = new QStandardItem();
        item->setSizeHint(QSize(CARD_WIDTH, CARD_HEIGHT));
        m_model->appendRow(item);

        QModelIndex index = m_model->indexFromItem(item);
        ui->digitallistview->setIndexWidget(index, card);

        QObject::connect(card, &digitalcard::clicked, this, [this, ch](bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        //QObject::connect(card,&digitalcard::longPressed,this,[this,channel]{});
    }
}

void Mainwindow::update_HardVer(int ch,const QString &ver){
    Q_UNUSED(ch);Q_UNUSED(ver);
}

// update digital card information

void Mainwindow::update_Voltage(int ch,float voltage){
    if (m_numbercards.contains(ch)) {
        m_numbercards[ch]->setVoltage(voltage);
    }

    /*switch(ch) {
        #define CHANNEL(n) \
            case n: \
                mCH##n##_Voltage.store(voltage); \
                emit CH##n##_VoltageChanged(); \
                \
                if(mCH##n##_enablebattery.load()){\
                    float newocv = mCH##n##_activeModel->getOCV(mCH##n##_currentSOC); \
                    QByteArray ocvbuffer(reinterpret_cast<const char*>(&newocv), sizeof(float)); \
                    std::reverse(ocvbuffer.begin(), ocvbuffer.end()); \
                    to_Channel(ch,0x02, 0x00, ocvbuffer); \
                    \
                    float newesr = mCH##n##_activeModel->getESR(mCH##n##_currentSOC); \
                    QByteArray esrbuffer(reinterpret_cast<const char*>(&newesr), sizeof(float)); \
                    std::reverse(esrbuffer.begin(), esrbuffer.end()); \
                    to_Channel(ch,0x02, 0x02, esrbuffer); \
                    \
                    if (mCH##n##_activeModel->isOver(mCH##n##_currentSOC)){ \
                        mCH##n##_enablebattery.store(false); \
                    } \
                } \
                return;
        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }*/
}

void Mainwindow::update_CurrentAndUnit(int ch,float current){
    if (m_numbercards.contains(ch)) {
        QString newUnit = (qAbs(current) < 1e-4) ? "mA" : "A";
        m_numbercards[ch]->setCurrentUnit(newUnit);
        m_numbercards[ch]->setCurrent(current);
    }

    /*switch(ch) {
        #define CHANNEL(n) \
            case n: { \
                if (mCH##n##_CurrentUnit != newUnit) { \
                    mCH##n##_CurrentUnit = newUnit; \
                    emit CH##n##_CurrentUnitChanged(); \
                } \
                mCH##n##_Current.store((qAbs(current) < 1e-4) ? current * 1000.0f : current); \
                emit CH##n##_CurrentChanged(); \
                \
                if(mCH##n##_enablebattery.load()){\
                    qint64 elapsedMs = mCH##n##_integralTimer.elapsed();\
                    float deltaTimeHours = elapsedMs / 3600000.0f; \
                    float capacityAH = mCH##n##_capacityAH.load(); \
                    float deltaSOC = (current * deltaTimeHours * 100) / capacityAH; \
                    float currentsoc = mCH##n##_currentSOC.load(); \
                    if (deltaSOC < 0 && qAbs(deltaSOC)>=currentsoc){ \
                        mCH##n##_currentSOC.store(0.0f); \
                        qCDebug(uart_bridge) << "Battery depleted!"; \
                    } \
                    else if(deltaSOC > 0 && (currentsoc+deltaSOC)>=100.0f){ \
                        mCH##n##_currentSOC.store(100.0f); \
                        qDebug() << "Battery fully charged!"; \
                    } \
                    else{ \
                        mCH##n##_currentSOC.store(currentsoc+deltaSOC); \
                        mCH##n##_integralTimer.restart(); \
                    } \
                    \
                    emit CH##n##_CurrentSOCChanged(); \
                }\
                return; \
            }
        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }*/
}

void Mainwindow::update_Status(int ch,quint16 status){
    if (m_numbercards.contains(ch)) {
        QString binaryStr = QString("%1").arg(status, 16, 2, QLatin1Char('0'));

        bool cv  = binaryStr.at(14) == "1";
        m_numbercards[ch]->setCVChecked(cv);
        bool cc  = binaryStr.at(13) == "1";
        m_numbercards[ch]->setCCChecked(cc);
        bool ovp = binaryStr.at(11) == "1";
        m_numbercards[ch]->setOVPChecked(ovp);
    }
}

void Mainwindow::update_Cv(int ch,float cv){
    if (m_numbercards.contains(ch)) {
        m_numbercards[ch]->setCvValue(cv);
    }
}

void Mainwindow::update_Cc(int ch,float cc){
    if (m_numbercards.contains(ch)) {
        m_numbercards[ch]->setCcValue(cc);
    }
}

void Mainwindow::update_Ovp(int ch,float ovp){
    if (m_numbercards.contains(ch)) {
        m_numbercards[ch]->setOvpValue(ovp);
    }
}

void Mainwindow::update_IsOutput(int ch,bool status){
    if (m_numbercards.contains(ch)) {
        m_numbercards[ch]->setChannelState(status);
    }
}

// update battery card information

void Mainwindow::update_Imp(int ch,float imp){
    Q_UNUSED(ch);Q_UNUSED(imp)
    /*for (auto it = m_numbercards.cbegin(); it != m_numbercards.cend(); ++it) {
        if (it.key() == ch){
            it.value()->setChannelState(status);
        }
    }*/
}

// other


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

void Mainwindow::load_BatteryModel(){
    if (m_modelManager) {
        QTimer::singleShot(0, this, [this]() {
            if (m_modelManager->loadAllModels()) {
                m_currentModelList = m_modelManager->getAvailableModels();

                if (!m_currentModelList.isEmpty()) {
                    /*#define CHANNEL(n) \
                    mCH##n##_batteryModel = m_currentModelList[0]; \
                    mCH##n##_activeModel = m_modelManager->getModel(mCH##n##_batteryModel);
                    CHANNEL_COUNT
                    #undef CHANNEL*/
                }
            }
        });
    }
}




QString Mainwindow::setChannel_CurrentUnit(int channel){
    quint8 unitCode; QString unit;
    static int step = 0;

    switch (step) {
    case 0:  unitCode = 0x01;unit = "mA";   break;
    case 1:  unitCode = 0x10;unit = "Auto"; break;
    case 2:  unitCode = 0x00;unit = "A";    break;
    default: unitCode = 0x00;unit = "A";    break;
    }
    step = (step + 1) % 3;

    QByteArray Unit_buffer;
    Unit_buffer.append(char(unitCode));
    to_Channel(channel,0x04, 0x0E, Unit_buffer);

    return unit;
}

void Mainwindow::setChannel_Setstatus(int channel,int model,const QString& val){
    // model: 0 - CV ; 1 - CC ; 3 - OVP;
    float value = val.toFloat();
    QByteArray buffer(reinterpret_cast<const char*>(&value), sizeof(float));
    std::reverse(buffer.begin(), buffer.end()); // big-endian order

    return to_Channel(channel,0x02, model, buffer);
}

void Mainwindow::setChannel_BatteryOutput(int channel,bool switchs){
    Q_UNUSED(channel);Q_UNUSED(switchs);
    if (m_currentModelList.isEmpty()) {return;}
    //if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {}

    /*switch(channel) {
        #define CHANNEL(n) \
            case n:{ \
                if (mCH##n##_batterystaticmode){ \
                    float newocv = mCH##n##_activeModel->getOCV(mCH##n##_currentSOC); \
                    QByteArray ocvbuffer(reinterpret_cast<const char*>(&newocv), sizeof(float)); \
                    std::reverse(ocvbuffer.begin(), ocvbuffer.end()); \
                    to_Channel(channel,0x02, 0x00, ocvbuffer); \
                    \
                    float newesr = mCH##n##_activeModel->getESR(mCH##n##_currentSOC); \
                    QByteArray esrbuffer(reinterpret_cast<const char*>(&newesr), sizeof(float)); \
                    std::reverse(esrbuffer.begin(), esrbuffer.end()); \
                    to_Channel(channel,0x02, 0x02, esrbuffer); \
                    \
                } else{ \
                    mCH##n##_enablebattery.store(switchs); \
                } \
                setChannel_Output(channel,switchs); \
                mCH##n##_integralTimer.restart(); \
                return; \
            }
        CHANNEL_COUNT
        #undef CHANNEL
        default:
            qCWarning(uart_bridge) << "Invalid channel:" << channel;
            return;
        }*/
}

void Mainwindow::setChannel_InitSOC(int channel,const QString& val){
    Q_UNUSED(channel);Q_UNUSED(val);


}

void Mainwindow::setChannel_Capacity(int channel,const QString& val){
     Q_UNUSED(channel);Q_UNUSED(val);
}

void Mainwindow::setChannel_Batterymode(int channel,bool staticmode){
    Q_UNUSED(channel);Q_UNUSED(staticmode);
}

QString Mainwindow::setChannel_BatteryModel(int channel){
    Q_UNUSED(channel);
    if (m_currentModelList.isEmpty()) {return "";}

    static int currentIndex = 0;
    currentIndex = (currentIndex + 1) % m_currentModelList.size();
    return "";

    /*if (channel == 0) {
        #define CHANNEL(n)  \
        mCH##n##_batteryModel =  m_currentModelList[currentIndex]; \
        mCH##n##_activeModel = m_modelManager->getModel(mCH##n##_batteryModel); \
        emit CH##n##_BatteryModelChanged();
        CHANNEL_COUNT
        #undef CHANNEL
        return mCH1_batteryModel;
    }

    // Single channel send
    switch(channel) {
        #define CHANNEL(n) \
            case n:{ \
                mCH##n##_batteryModel =  m_currentModelList[currentIndex]; \
                mCH##n##_activeModel = m_modelManager->getModel(mCH##n##_batteryModel); \
                emit CH##n##_BatteryModelChanged(); \
                return mCH##n##_batteryModel; \
            }
        CHANNEL_COUNT
        #undef CHANNEL
        default:
            qCWarning(uart_bridge) << "Invalid channel:" << channel;
            return "";
    }*/
}


