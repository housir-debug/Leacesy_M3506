#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "ui_devicesetpage.h"
#include "devicesetpage.h"
#include <QTimer>

/* slots function */
void devicesetpage::on_ipradioButton_clicked()
{
    m_setmode = Setmode::ip;
    ui->ipradioButton->setChecked(true);
}
void devicesetpage::on_maskradioButton_clicked()
{
    m_setmode = Setmode::mask;
    ui->maskradioButton->setChecked(true);
}
void devicesetpage::on_gateradioButton_clicked()
{
    m_setmode = Setmode::gate;
    ui->gateradioButton->setChecked(true);
}
void devicesetpage::on_canidradioButton_clicked()
{
    m_setmode = Setmode::canid;
    ui->canidradioButton->setChecked(true);
}
void devicesetpage::on_canBaud_comboBox_clicked(int index)
{
    ConfigManager::s_canBaudIndex = index;
    emit ChannelBus::instance().set_canbaud();
}
void devicesetpage::on_rs32Baud_comboBox_clicked(int index)
{
    ConfigManager::s_rs232BaudIndex = index;
    emit ChannelBus::instance().set_RS232Baud();
}

void devicesetpage::on_back_pushButton_clicked(){emit backRequested();}
void devicesetpage::on_dhcp_pushButton_clicked()
{
    ui->ipradioButton->setEnabled(false);
    ui->maskradioButton->setEnabled(false);
    ui->gateradioButton->setEnabled(false);
    ui->canidradioButton->setChecked(true);
    m_setmode = Setmode::canid;

    emit ChannelBus::instance().set_network(false);
    ui->iplabel->setText("---.---.---.---");
    ui->masklabel->setText("---.---.---.---");
    ui->gatelabel->setText("---.---.---.---");
}
void devicesetpage::on_static_pushButton_clicked()
{
    ui->ipradioButton->setEnabled(true);
    ui->maskradioButton->setEnabled(true);
    ui->gateradioButton->setEnabled(true);
    ui->ipradioButton->setChecked(true);

    m_setmode = Setmode::ip;
    ui->refreshpushButton->setEnabled(false);
}
void devicesetpage::on_refreshpushButton_clicked()
{
    if (ConfigManager::getNetworkConfig()){
        ui->iplabel->setText(ConfigManager::IP);
        ui->masklabel->setText(ConfigManager::SM);
        ui->gatelabel->setText(ConfigManager::Gate);
    }
}

devicesetpage::devicesetpage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::devicesetpage)
{
    ui->setupUi(this);

    ui->hardver_label->setText(Device::HardVer);
    ui->softver_label->setText(Device::SoftVer);
    ui->macaddress_label->setText(ConfigManager::MAC);

    ui->iplabel->setText(ConfigManager::IP);
    ui->masklabel->setText(ConfigManager::SM);
    ui->gatelabel->setText(ConfigManager::Gate);
    ui->canidlabel->setText(QString::number(ConfigManager::s_CANid.load()));
    ui->canBaud_comboBox->setCurrentIndex(ConfigManager::s_canBaudIndex);
    ui->rs32Baud_comboBox->setCurrentIndex(ConfigManager::s_rs232BaudIndex);

    if (ConfigManager::isDHCP){
        ui->dhcp_pushButton->setChecked(true);
        ui->refreshpushButton->setEnabled(true);

        ui->ipradioButton->setEnabled(false);
        ui->maskradioButton->setEnabled(false);
        ui->gateradioButton->setEnabled(false);
        ui->canidradioButton->setChecked(true);
        m_setmode = Setmode::canid;
    }else{
        ui->static_pushButton->setChecked(true);
        ui->refreshpushButton->setEnabled(false);

        ui->ipradioButton->setEnabled(true);
        ui->maskradioButton->setEnabled(true);
        ui->gateradioButton->setEnabled(true);
        ui->ipradioButton->setChecked(true);
        m_setmode = Setmode::ip;
    }

    QObject::connect(ui->keyframe, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){
            switch (m_setmode) {
                case Setmode::ip:case Setmode::mask:case Setmode::gate:{
                    QString *fields[] = { &ConfigManager::IP, &ConfigManager::SM, &ConfigManager::Gate };
                    QLabel *valueLabels[] = { ui->iplabel, ui->masklabel, ui->gatelabel };
                    valueLabels[m_setmode]->setText(value);
                    *fields[m_setmode] = value;

                    if (ConfigManager::IP != "---.---.---.---" && ConfigManager::SM != "---.---.---.---"
                            && ConfigManager::Gate != "---.---.---.---") {
                            emit ChannelBus::instance().set_network(true);
                    }
                    return;
                }
                case Setmode::canid:
                    if (ConfigManager::saveNewValue(ConfAddress::CanId,value.toUInt())){
                        ConfigManager::s_CANid = value.toUInt();
                        ui->canidlabel->setText(value);
                    }
                    return;
                default:return;
            }
        }});
}
devicesetpage::~devicesetpage()
{
    delete ui;
}

/* update card style */
void devicesetpage::responseUpdate()
{
    ui->canidlabel->setText(QString::number(ConfigManager::s_CANid.load()));
    ui->canBaud_comboBox->setCurrentIndex(ConfigManager::s_canBaudIndex);
    ui->rs32Baud_comboBox->setCurrentIndex(ConfigManager::s_rs232BaudIndex);

    if (ConfigManager::isDHCP){
        QTimer::singleShot(8100, this,[this]() {
            ui->refreshpushButton->setEnabled(true);
            on_refreshpushButton_clicked();
            }); // 8.1s
    }else{
        ui->refreshpushButton->setEnabled(false);
        on_refreshpushButton_clicked();
    }
}
