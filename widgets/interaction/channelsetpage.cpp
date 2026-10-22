#include "widgets/component/channelcard.h"
#include "auxiliary/global_static.h"
#include "auxiliary/battery_model.h"
#include "channel/uart_protocol.h"
#include "auxiliary/signal_bus.h"
#include "ui_channelsetpage.h"
#include "channelsetpage.h"
#include <QtEndian>

/* slots function */
void channelsetting::on_cv_pushButton_clicked()
{
    ui->keyframe->setPlaceholderText("请输入CV");
    m_setmode = channelSet_mode::cv;
}
void channelsetting::on_cc_pushButton_clicked()
{
    ui->keyframe->setPlaceholderText("请输入CC");
    m_setmode = channelSet_mode::cc;
}
void channelsetting::on_ovp_pushButton_clicked()
{
    ui->keyframe->setPlaceholderText("请输入OVP");
    m_setmode = channelSet_mode::ovp;
}
void channelsetting::on_soc_pushButton_clicked()
{
    ui->keyframe->setPlaceholderText("请输入SOC");
    m_setmode = channelSet_mode::soc;
}
void channelsetting::on_cap_pushButton_clicked()
{
    ui->keyframe->setPlaceholderText("请输入CAP");
    m_setmode = channelSet_mode::cap;
}
void channelsetting::on_back_pushButton_clicked(){emit backRequested();}
void channelsetting::on_allFO_pushButton_clicked(){setAllChannelsChecked(false);}
void channelsetting::on_allNO_pushButton_clicked(){setAllChannelsChecked(true);}
void channelsetting::setAllChannelsChecked(bool checked){
    for (int i = 1; i <= 36; ++i) {
        auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
        if (btn && btn->isEnabled()){
            btn->setChecked(checked);
        }
    }
}

channelsetting::channelsetting(QWidget *parent) :QWidget(parent),
    ui(new Ui::channelsetting)
{
    ui->setupUi(this);

    setCombModel();
    on_cv_pushButton_clicked();
    ui->hardver_label->setText(QString("HardVer :   ") + Device::HardVer);
    ui->softver_label->setText(QString("SoftVer :   ") + Device::SoftVer);

    QObject::connect(ui->onoff_comboBox, &QComboBox::currentTextChanged,this,[this](const QString &text){
        if (text == "OFF"){ // 1-off outputed==true
            ChannelBus::instance().to_Channel(0,JV6xxx::OUTPUT,JV6xxx::Output::SET_OUTPSTATUS_OFF,"",false);
            ui->onoff_comboBox->setProperty("Outputed",false);
        }else{
            ChannelBus::instance().to_Channel(0,JV6xxx::OUTPUT,JV6xxx::Output::SET_OUTPSTATUS_ON,"",false);
            ui->onoff_comboBox->setProperty("Outputed",true);
        }

        ui->onoff_comboBox->style()->unpolish(ui->onoff_comboBox);
        ui->onoff_comboBox->style()->polish(ui->onoff_comboBox);
        on_allFO_pushButton_clicked();
        }, Qt::DirectConnection);
    QObject::connect(ui->unit_comboBox, &QComboBox::currentTextChanged,this, [this](const QString &text){
        if(text == "A"){
            ChannelBus::instance().to_Channel(0,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSCURR_RANGE,QByteArray(1,JV6xxx::Param::A),false);
        }else if(text == "mA"){
            ChannelBus::instance().to_Channel(0,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSCURR_RANGE,QByteArray(1,JV6xxx::Param::mA),false);
        }else{
            ChannelBus::instance().to_Channel(0,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSCURR_RANGE,QByteArray(1,JV6xxx::Param::Auto),false);
        }

        on_allFO_pushButton_clicked();
        }, Qt::DirectConnection);
    QObject::connect(ui->model_comboBox, &QComboBox::currentTextChanged,this,[this](const QString &text){
        for (int i = 1; i <= 36; ++i) {
            auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
            if (btn && btn->isChecked()){
                emit to_chModel(i,text);
            }
        }

        on_allFO_pushButton_clicked();
        }, Qt::DirectConnection);
    QObject::connect(ui->cardpage_comboBox, &QComboBox::currentTextChanged,this,[this](const QString &text){
        for (int i = 1; i <= 36; ++i) {
            auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
            if (btn && btn->isChecked()){
                if (text == "status"){
                    emit to_chpage(i,ChCardindex::Status);
                }else{
                    emit to_chpage(i,ChCardindex::Version);
                }
            }
        }

        on_allFO_pushButton_clicked();
        }, Qt::DirectConnection);

    QObject::connect(ui->keyframe, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){
            float val = value.toFloat();

            switch (m_setmode) {
                case channelSet_mode::cv:
                    sendModecmd(val,JV6xxx::Setting::SET_SOURVOLT);
                    break;
                case channelSet_mode::cc:
                    sendModecmd(val,JV6xxx::Setting::SET_SOURCURR);
                    break;
                case channelSet_mode::ovp:
                    sendModecmd(val,JV6xxx::Setting::SET_SOURVOLT_PROT);
                    break;
                case channelSet_mode::soc:
                    for (int i = 1; i <= 36; ++i) {
                        auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                        if (btn && btn->isChecked()){
                            emit to_chSOC(i,val);
                        }
                    }
                    break;
                case channelSet_mode::cap:
                    for (int i = 1; i <= 36; ++i) {
                        auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                        if (btn && btn->isChecked()){
                            emit to_chCAP(i,val);
                        }
                    }
                    break;
                default:break;
            }

            on_allFO_pushButton_clicked();
        }}, Qt::DirectConnection);
}
void channelsetting::sendModecmd(float val,quint8 func){
    QByteArray data(4,0);
    qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));

    for (int i = 1; i <= 36; ++i) {
        auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
        if (btn && btn->isChecked()){
            ChannelBus::instance().to_Channel(i,JV6xxx::SETTING,func, data,false);
        }
    }
}
channelsetting::~channelsetting()
{
    delete ui;
}


/* update card style */
void channelsetting::setCombModel()
{
    ui->model_comboBox->clear();
    ui->model_comboBox->addItems(BatteryModelManager::instance().getAvailableModels());
}
void channelsetting::update_ChEn(int ch)
{
    auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(ch));
    if (btn && !btn->isEnabled()){
        btn->setEnabled(true);
    }
}
