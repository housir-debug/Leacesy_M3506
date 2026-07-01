#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "auxiliary/config_manager.h"
#include <QScroller>

Mainwindow::Mainwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Mainwindow)
{
    #define CHANNEL(n) m_channel[n] = "0.0.0.0"; \
    CHANNEL_COUNT
    #undef CHANNEL

    ui->setupUi(this);

    m_model = new QStandardItemModel(this);
    ui->digitallistview->setModel(m_model);



    QList<QString> testChannels;
    for (int i = 1; i <= 16; ++i) {
       testChannels.append(QString("CH-%1").arg(i, 2, 10, QChar('0')));
    }

    addCardsFromList(testChannels);
}

Mainwindow::~Mainwindow()
{
    if (!m_cards.isEmpty()){
        for (auto *card : qAsConst(m_cards)) {
            card->deleteLater();
        }
    }

    m_cards.clear();
    delete ui;
}

void Mainwindow::addCardsFromList(const QList<QString> &channelNames)
{
    for (const QString &name : channelNames) {
        digitalcard *card = new digitalcard(this);
        card->setChannelName(name);
        m_cards.append(card);

        QStandardItem *item = new QStandardItem();
        item->setSizeHint(QSize(200, 310));
        m_model->appendRow(item);

        QModelIndex index = m_model->indexFromItem(item);
        ui->digitallistview->setIndexWidget(index, card);
    }

    // listview width = screen width
    QScroller::grabGesture(ui->digitallistview->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(ui->digitallistview->viewport());

    QScrollerProperties properties;
    // Maximum speed,default 0.5,more small [stop]distance more near
    properties.setScrollMetric(QScrollerProperties::MaximumVelocity, 0.18);
    // Sliding start distance,default 0.1,more small [react]more faster
    properties.setScrollMetric(QScrollerProperties::DragStartDistance, 0.0036);
    // Deceleration rate,default 0.15,more small [stop]more slowly
    properties.setScrollMetric(QScrollerProperties::DecelerationFactor, 0.18);
    // Speed following,default 0.6,more small [react]more faster
    properties.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 0.18);

    scroller->setScrollerProperties(properties);
}

digitalcard* Mainwindow::findCardByChannelName(const QString &name) const
{
    for (auto *card : qAsConst(m_cards)) {
        if (card->getChannelName() == name) {
            return card;
        }
    }

    return nullptr;
}

// Serial port call

void Mainwindow::update_SoftVer(int ch,const QString &ver){
    switch(ch) {
        #define CHANNEL(n) \
        case n: \
            m_channel[n] = "0.0.0.0"; \
            return;/*qCDebug(uart_bridge) << "Channel" << n << "SV updated to:" << ver;*/

        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}

void Mainwindow::update_HardVer(int ch,const QString &ver){
    switch(ch) {
        #define CHANNEL(n) \
        case n: \
            mCH##n##_hv = ver; \
            emit CH##n##_hvChanged(); \
            return;/*qCDebug(uart_bridge) << "Channel" << n << "HV updated to:" << ver;*/

        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}

void Mainwindow::update_Voltage(int ch,float voltage){
    switch(ch) {
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
                return;/*qCDebug(uart_bridge) << "Channel" << n << "voltage updated to:" << voltage;*/
        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}

void Mainwindow::update_CurrentAndUnit(int ch,float current){
    QString newUnit = (qAbs(current) < 1e-4) ? "mA" : "A"; // true: mA   false: A

    switch(ch) {
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
            }/*qCDebug(uart_bridge) << "Channel" << n << "current updated to:" << current;*/
        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}

void GuiBridge::update_Status(int ch,quint16 status){
    QString binaryStr = QString("%1").arg(status, 16, 2, QLatin1Char('0'));

    switch(ch) {
        #define CHANNEL(n) \
            case n: \
                if (mCH##n##_Status != binaryStr) { \
                    mCH##n##_Status = binaryStr; \
                    emit CH##n##_StatusChanged(); \
                }return;

        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}

void GuiBridge::update_Cv(int ch,float cv){
    switch(ch) {
        #define CHANNEL(n) \
            case n: \
                mCH##n##_cv.store(cv); \
                emit CH##n##_cvChanged(); \
                return;

        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}

void GuiBridge::update_Cc(int ch,float cc){
    switch(ch) {
        #define CHANNEL(n) \
            case n: \
                mCH##n##_cc.store(cc); \
                emit CH##n##_ccChanged(); \
                return;

        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}

void GuiBridge::update_Ovp(int ch,float ovp){
    switch(ch) {
        #define CHANNEL(n) \
            case n: \
                mCH##n##_ovp.store(ovp); \
                emit CH##n##_ovpChanged(); \
                return;

        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}

void GuiBridge::update_IsOutput(int ch,bool status){
    switch(ch) {
        #define CHANNEL(n) \
            case n: \
                mCH##n##_isOutput.store(status); \
                emit CH##n##_isOutputChanged(); \
                return;

        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}

void GuiBridge::update_Imp(int ch,float imp){
    switch(ch) {
        #define CHANNEL(n) \
        case n: \
            mCH##n##_imp.store(imp); \
            emit CH##n##_impChanged(); \
            return;

        CHANNEL_COUNT
        #undef CHANNEL
        default: return;
    }
}
