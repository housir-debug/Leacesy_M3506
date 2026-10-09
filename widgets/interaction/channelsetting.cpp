#include "channelsetting.h"
#include "ui_channelsetting.h"
#include "auxiliary/uartProtocol.h"

channelsetting::channelsetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::channelsetting)
{
    ui->setupUi(this);
    //ui->model_comboBox->addItems(m_modelManager->getAvailableModels());

    connect(ui->onoff_comboBox, &QComboBox::currentTextChanged,this,[this](const QString &text){
        if (text == "OFF"){ // 1-off outputed==true
            to_Channel(0,JV6xxx::OUTPUT,JV6xxx::Output::SET_OUTPSTATUS_OFF,"");
            ui->onoff_comboBox->setProperty("Outputed",false);
        }else{
            to_Channel(0,JV6xxx::OUTPUT,JV6xxx::Output::SET_OUTPSTATUS_ON,"");
            ui->onoff_comboBox->setProperty("Outputed",true);
        }

        ui->onoff_comboBox->style()->unpolish(ui->onoff_comboBox);
        ui->onoff_comboBox->style()->polish(ui->onoff_comboBox);
        });
    connect(ui->unit_comboBox, &QComboBox::currentTextChanged,this, [this](const QString &text){
        if(text == "A"){
            to_Channel(0,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSCURR_RANGE,QByteArray(1, 0x00));
        }else if(text == "mA"){
            to_Channel(0,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSCURR_RANGE,QByteArray(1, 0x01));
        }else{
            to_Channel(0,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSCURR_RANGE,QByteArray(1, 0x10));
        }});
    connect(ui->model_comboBox, &QComboBox::currentTextChanged,this,[this](const QString &text){
        for (int i = 1; i <= 36; ++i) {
            auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
            if (btn && btn->isChecked()){
                emit to_chModel(i,text);
            }
        }
    });

    m_setnmbkeycard = new numberkeypad(ui->keyframe->parentWidget());
    m_setnmbkeycard->setGeometry(ui->keyframe->geometry());
    QObject::connect(m_setnmbkeycard, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){
            float val = value.toFloat();

            switch (m_setmode) {
                case channelSet_mode::cv:return sendModecmd(val,JV6xxx::Setting::SET_SOURVOLT);
                case channelSet_mode::cc:return sendModecmd(val,JV6xxx::Setting::SET_SOURCURR);
                case channelSet_mode::ovp:return sendModecmd(val,JV6xxx::Setting::SET_SOURVOLT_PROT);
                case channelSet_mode::soc:
                    for (int i = 1; i <= 36; ++i) {
                        auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                        if (btn && btn->isChecked()){
                            emit to_chSOC(i,val);
                        }
                    }
                    return;
                case channelSet_mode::cap:
                    for (int i = 1; i <= 36; ++i) {
                        auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                        if (btn && btn->isChecked()){
                            emit to_chCAP(i,val);
                        }
                    }
                    return;
                default:return;
            }
        }});
    delete ui->keyframe;
}

channelsetting::~channelsetting()
{
    delete ui;
}

void channelsetting::sendModecmd(float val,quint8 func){
    QByteArray data(4,0);
    qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));

    for (int i = 1; i <= 36; ++i) {
        auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
        if (btn && btn->isChecked()){
            to_Channel(i,JV6xxx::SETTING,func, data);
        }
    }
}

void channelsetting::to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param){
    // Single channel send
    switch(channel) {
        case 0:
            #define CHANNEL(n) emit to_UartChannel##n(cmd, func, param,false);
            CHANNEL_COUNT
            #undef CHANNEL
            return;
        #define CHANNEL(n) case n: return emit to_UartChannel##n(cmd, func, param,false);
        CHANNEL_COUNT
        #undef CHANNEL
        default:return;
    }
}


void channelsetting::on_back_pushButton_clicked(){emit backRequested();}

void channelsetting::on_allFO_pushButton_clicked(){setAllChannelsChecked(false);}
void channelsetting::on_allNO_pushButton_clicked(){setAllChannelsChecked(true);}
void channelsetting::setAllChannelsChecked(bool checked){
    for (int i = 1; i <= 36; ++i) {
        auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
        if (btn){
            btn->setChecked(checked);
        }
    }
}

void channelsetting::on_cv_pushButton_clicked(){m_setmode = channelSet_mode::cv;}
void channelsetting::on_cc_pushButton_clicked(){m_setmode = channelSet_mode::cc;}
void channelsetting::on_ovp_pushButton_clicked(){m_setmode = channelSet_mode::ovp;}
void channelsetting::on_soc_pushButton_clicked(){m_setmode = channelSet_mode::soc;}
void channelsetting::on_cap_pushButton_clicked(){m_setmode = channelSet_mode::cap;}
