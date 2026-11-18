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
    ui->keyframe->setEnabled(true);
}
void channelsetting::on_cc_pushButton_clicked()
{
    ui->keyframe->setPlaceholderText("请输入CC");
    m_setmode = channelSet_mode::cc;
    ui->keyframe->setEnabled(true);
}
void channelsetting::on_ovp_pushButton_clicked()
{
    ui->keyframe->setPlaceholderText("请输入OVP");
    m_setmode = channelSet_mode::ovp;
    ui->keyframe->setEnabled(true);
}
void channelsetting::on_soc_pushButton_clicked()
{
    ui->keyframe->setPlaceholderText("请输入SOC");
    m_setmode = channelSet_mode::soc;
    ui->keyframe->setEnabled(true);
}
void channelsetting::on_cap_pushButton_clicked()
{
    ui->keyframe->setPlaceholderText("请输入CAP");
    m_setmode = channelSet_mode::cap;
    ui->keyframe->setEnabled(true);
}
void channelsetting::on_back_pushButton_clicked(){emit backRequested();}
void channelsetting::on_sure_pushButton_clicked(){ui->setselectframe->setEnabled(true);}
void channelsetting::on_allFO_pushButton_clicked(){setAllChannelsChecked(false);}
void channelsetting::on_allNO_pushButton_clicked(){setAllChannelsChecked(true);}
void channelsetting::setAllChannelsChecked(bool checked)
{
    for (int i = 1; i <= 36; ++i) {
        auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
        if (btn && btn->isEnabled()){
            btn->setChecked(checked);
        }
    }

    ui->setselectframe->setEnabled(checked);
    ui->keyframe->setEnabled(false);
}

channelsetting::channelsetting(QWidget *parent) :QWidget(parent),
    ui(new Ui::channelsetting)
{
    ui->setupUi(this);

    setCombModel();
    on_cv_pushButton_clicked();
    ui->keyframe->setEnabled(false);
    ui->setselectframe->setEnabled(false);
    ui->hardver_label->setText(QString("HardVer :   ") + Device::HardVer);
    ui->softver_label->setText(QString("SoftVer :   ") + Device::SoftVer);

    QObject::connect(ui->onoff_comboBox, &QComboBox::currentTextChanged,this,[this](const QString &text){
        if (text == "OFF"){ // 1-off outputed==true
            for (int i = 1; i <= 36; ++i) {
                auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                if (btn && btn->isChecked()){
                    ChannelBus::instance().to_Channel(i,JV6xxx::UartCMD::OUTP_OFF,"",false);
                }
            }
            ui->onoff_comboBox->setProperty("Outputed",false);
        }else{
            for (int i = 1; i <= 36; ++i) {
                auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                if (btn && btn->isChecked()){
                    ChannelBus::instance().to_Channel(i,JV6xxx::UartCMD::OUTP_ON,"",false);
                }
            }
            ui->onoff_comboBox->setProperty("Outputed",true);
        }

        ui->onoff_comboBox->style()->unpolish(ui->onoff_comboBox);
        ui->onoff_comboBox->style()->polish(ui->onoff_comboBox);
        on_allFO_pushButton_clicked();
        }, Qt::DirectConnection);
    QObject::connect(ui->unit_comboBox, &QComboBox::currentTextChanged,this, [this](const QString &text){
        if(text == "A"){
            for (int i = 1; i <= 36; ++i) {
                auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                if (btn && btn->isChecked()){
                    ChannelBus::instance().to_Channel(i,JV6xxx::UartCMD::CurrRANG,QByteArray(1,JV6xxx::Param::A),false);
                }
            }
        }else if(text == "mA"){
            for (int i = 1; i <= 36; ++i) {
                auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                if (btn && btn->isChecked()){
                    ChannelBus::instance().to_Channel(i,JV6xxx::UartCMD::CurrRANG,QByteArray(1,JV6xxx::Param::mA),false);
                }
            }
        }else{
            for (int i = 1; i <= 36; ++i) {
                auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                if (btn && btn->isChecked()){
                    ChannelBus::instance().to_Channel(i,JV6xxx::UartCMD::CurrRANG,QByteArray(1,JV6xxx::Param::Auto),false);
                }
            }
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
            QByteArray data(4,0);
            float val = value.toFloat();
            qToBigEndian(val, reinterpret_cast<uint8_t*>(data.data()));

            for (int i = 1; i <= 36; ++i) {
                auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(i));
                if (btn && btn->isChecked()){
                    switch (m_setmode) {
                        case channelSet_mode::cv:
                            ChannelBus::instance().to_Channel(i,JV6xxx::UartCMD::VOLT,data,false);
                            break;
                        case channelSet_mode::cc:
                            ChannelBus::instance().to_Channel(i,JV6xxx::UartCMD::CURR,data,false);
                            break;
                        case channelSet_mode::ovp:
                            ChannelBus::instance().to_Channel(i,JV6xxx::UartCMD::OVP,data,false);
                            break;
                        case channelSet_mode::soc:
                            emit to_chSOC(i,val);
                            break;
                        case channelSet_mode::cap:
                            emit to_chCAP(i,val);
                            break;
                    }
                }
            }

            on_allFO_pushButton_clicked();
        }}, Qt::DirectConnection);
}
channelsetting::~channelsetting()
{
    delete ui;
}


/* update card style */
void channelsetting::setCombModel()
{
    ui->model_comboBox->clear();
    ui->model_comboBox->addItems(BatteryModelManager::instance().m_models.keys());
}
void channelsetting::update_ChEn(int ch)
{
    auto *btn = ui->chselectframe->findChild<QPushButton*>(QString("pushButton%1").arg(ch));
    if (btn && !btn->isEnabled()){
        btn->setEnabled(true);
    }
}
