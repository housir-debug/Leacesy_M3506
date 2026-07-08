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
        // digitalcard
        digitalcard *digitcard = new digitalcard(this);
        digitcard->setChannelName(QString("CH-%1").arg(i, 2, 10, QChar('0')));
        m_digitalcards[i] = digitcard;

        QStandardItem *digititem = new QStandardItem();
        digititem->setSizeHint(QSize(CARD_WIDTH, CARD_HEIGHT));
        m_digitalModel->appendRow(digititem);

        QModelIndex digitindex = m_digitalModel->indexFromItem(digititem);
        ui->digitallistview->setIndexWidget(digitindex, digitcard);

        QObject::connect(digitcard, &digitalcard::clicked, this, [this, i](bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(i, 0x01, func, "");
        });
        QObject::connect(digitcard,&digitalcard::longPressed,this,[this,i]{
            m_functioncCh = i;
            m_initalpage = 0;
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
        });

        // batterycard
        batterycard *battercard = new batterycard(this);
        battercard->setChannelName(QString("CH-%1").arg(i, 2, 10, QChar('0')));
        m_batterycards[i] = battercard;

        QStandardItem *batteritem = new QStandardItem();
        batteritem->setSizeHint(QSize(CARD_WIDTH, CARD_HEIGHT));
        m_batteryModel->appendRow(batteritem);

        QModelIndex batterindex = m_batteryModel->indexFromItem(batteritem);
        ui->batterylistview->setIndexWidget(batterindex, battercard);

        QObject::connect(battercard, &batterycard::clicked, this, [this, i](bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(i, 0x01, func, "");
        });
        QObject::connect(battercard,&batterycard::longPressed,this,[this,i]{
            m_functioncCh = i;
            m_initalpage = 1;
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
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
                        it.value()->setModel(m_modelManager->getModel(batteryModel));
                    }
                }
            }
        });
    }
}

// auto add exist channel card

void Mainwindow::update_SoftVer(int ch,const QString &ver){
    Q_UNUSED(ver);
    if (!m_digitalcards.contains(ch)) {
        digitalcard *card = new digitalcard(this);
        card->setChannelName(QString("CH-%1").arg(ch, 2, 10, QChar('0')));
        m_digitalcards[ch] = card;

        QStandardItem *item = new QStandardItem();
        item->setSizeHint(QSize(CARD_WIDTH, CARD_HEIGHT));
        m_digitalModel->appendRow(item);

        QModelIndex index = m_digitalModel->indexFromItem(item);
        ui->digitallistview->setIndexWidget(index, card);

        QObject::connect(card, &digitalcard::clicked, this, [this, ch](bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        QObject::connect(card,&digitalcard::longPressed,this,[this,ch]{
            m_functioncCh = ch;
            m_initalpage = 0;
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
        });
    }
}

void Mainwindow::update_HardVer(int ch,const QString &ver){
    Q_UNUSED(ver);
    if (!m_batterycards.contains(ch)) {
        batterycard *card = new batterycard(this);
        card->setChannelName(QString("CH-%1").arg(ch, 2, 10, QChar('0')));
        m_batterycards[ch] = card;

        QStandardItem *item = new QStandardItem();
        item->setSizeHint(QSize(CARD_WIDTH, CARD_HEIGHT));
        m_batteryModel->appendRow(item);

        QModelIndex index = m_batteryModel->indexFromItem(item);
        ui->batterylistview->setIndexWidget(index, card);

        QObject::connect(card, &batterycard::clicked, this, [this, ch](bool switchs) {
            quint8 func = switchs ? 0x01 : 0x00;
            to_Channel(ch, 0x01, func, "");
        });
        QObject::connect(card,&batterycard::longPressed,this,[this,ch]{
            m_functioncCh = ch;
            m_initalpage = 1;
            ui->topstackedwidget->setCurrentIndex(3);  // functionpage
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

void Mainwindow::update_Voltage(int ch,float voltage){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setVoltage(voltage);
    }

    if (m_batterycards.contains(ch) && m_initalpage == 1){
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
    if (m_digitalcards.contains(ch)) {
        QString newUnit = (qAbs(current) < 1e-4) ? "mA" : "A";
        m_digitalcards[ch]->setCurrentUnit(newUnit);
        m_digitalcards[ch]->setCurrent(current);
    }

    if (m_batterycards.contains(ch) && m_initalpage == 1){
        m_batterycards[ch]->updateSocValue(current);
    }
}

void Mainwindow::update_Status(int ch,quint16 status){
    if (m_digitalcards.contains(ch)) {
        QString binaryStr = QString("%1").arg(status, 16, 2, QLatin1Char('0'));

        bool cv  = binaryStr.at(14) == "1";
        m_digitalcards[ch]->setCVChecked(cv);
        bool cc  = binaryStr.at(13) == "1";
        m_digitalcards[ch]->setCCChecked(cc);
        bool ovp = binaryStr.at(11) == "1";
        m_digitalcards[ch]->setOVPChecked(ovp);
    }
}

void Mainwindow::update_Cv(int ch,float cv){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setCvValue(cv);
    }

    if (m_batterycards.contains(ch)){
        m_batterycards[ch]->setOcvValue(cv);
    }
}

void Mainwindow::update_Cc(int ch,float cc){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setCcValue(cc);
    }
}

void Mainwindow::update_Ovp(int ch,float ovp){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setOvpValue(ovp);
    }
}

void Mainwindow::update_IsOutput(int ch,bool status){
    if (m_digitalcards.contains(ch)) {
        m_digitalcards[ch]->setChannelState(status);
    }

    if (m_batterycards.contains(ch)){
        m_batterycards[ch]->setChannelState(status);
        m_batterycards[ch]->startupdateValue();
    }
}

void Mainwindow::update_Imp(int ch,float imp){
    if (m_batterycards.contains(ch)){
        m_batterycards[ch]->setEsrValue(imp);
    }
}

// ************************************other

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

void Mainwindow::setChannel_Setstatus(int channel,int model,const QString& val){
    // model: 0 - CV ; 1 - CC ; 3 - OVP;
    float value = val.toFloat();
    QByteArray buffer(reinterpret_cast<const char*>(&value), sizeof(float));
    std::reverse(buffer.begin(), buffer.end()); // big-endian order

    return to_Channel(channel,0x02, model, buffer);
}

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

QString Mainwindow::set_unit(int channel)
{
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

QString Mainwindow::set_Model(int channel)
{
    static int currentIndex = 0;

    if (!m_currentModelList.isEmpty()) {
        currentIndex = (currentIndex + 1) % m_currentModelList.size();
        QString batteryModel = m_currentModelList[currentIndex];

        if (channel == 0) {
            for (auto it = m_batterycards.cbegin(); it != m_batterycards.cend(); ++it) {
                it.value()->setModelValue(batteryModel);
                it.value()->setModel(m_modelManager->getModel(batteryModel));
            }
        }else{
            if (m_batterycards.contains(channel)){
                m_batterycards[channel]->setModelValue(batteryModel);
                m_batterycards[channel]->setModel(m_modelManager->getModel(batteryModel));
            }
        }

        return batteryModel;
    }

    return "empty";
}

// channel card switchs

void Mainwindow::on_digitalsettingspushButton_clicked()
{
    ui->topstackedwidget->setCurrentIndex(2);  // settingspage
}

void Mainwindow::on_batterysettingspushButton_clicked()
{
    ui->topstackedwidget->setCurrentIndex(2);  // settingspage
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
    QString newunit = "All - " + set_unit(0);
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
    QString modelname = "All - " + set_Model(0);
    ui->batteryallmodelpushButton->setText(modelname);
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
    // Apply all 逻辑：遍历所有通道应用设置
    /*for (auto it = m_numbercards.cbegin(); it != m_numbercards.cend(); ++it) {
        int ch = it.key();
        // 发送命令到每个通道
        to_Channel(ch, 0x04, 0x0E, QByteArray(1, 0x10));  // 切换到 Auto 单位
    }*/
}
