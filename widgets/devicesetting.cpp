#include "devicesetting.h"
#include "ui_devicesetting.h"
#include "auxiliary/config_manager.h"
#include <QTimer>

devicesetting::devicesetting(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::devicesetting)
{
    ui->setupUi(this);

    ui->iplabel->setText(ConfigManager::s_IP);
    ui->masklabel->setText(ConfigManager::s_SM);
    ui->maclabel->setText(ConfigManager::s_MAC);
    ui->gatelabel->setText(ConfigManager::s_Gateway);
    ui->canidlabel->setText(QString::number(ConfigManager::s_CANid));
    ui->gpibidlabel->setText(QString::number(ConfigManager::s_GPIBid));

    if (ConfigManager::s_isDHCP){
        ui->dhcpradioButton->setChecked(true);
        ui->refreshpushButton->setEnabled(true);
        ui->canidradioButton->setChecked(true);
        m_setmode = 3;

        ui->ipradioButton->setEnabled(false);
        ui->maskradioButton->setEnabled(false);
        ui->gateradioButton->setEnabled(false);

    }else{
        ui->setstaticradioButton->setChecked(true);
        ui->refreshpushButton->setEnabled(false);
        ui->ipradioButton->setChecked(true);
        m_setmode = 0;

        ui->ipradioButton->setEnabled(true);
        ui->maskradioButton->setEnabled(true);
        ui->gateradioButton->setEnabled(true);
    }
}

devicesetting::~devicesetting()
{
    delete ui;
}


void devicesetting::on_ipradioButton_clicked()
{
    m_setmode = 0;
    ui->ipradioButton->setChecked(true);
}

void devicesetting::on_maskradioButton_clicked()
{
    m_setmode = 1;
    ui->maskradioButton->setChecked(true);
}

void devicesetting::on_gateradioButton_clicked()
{
    m_setmode = 2;
    ui->gateradioButton->setChecked(true);
}

void devicesetting::on_canidradioButton_clicked()
{
    m_setmode = 3;
    ui->canidradioButton->setChecked(true);
}

void devicesetting::on_gpibidradioButton_clicked()
{
    m_setmode = 4;
    ui->gpibidradioButton->setChecked(true);
}


void devicesetting::on_dhcpradioButton_clicked()
{
    ui->canidradioButton->setChecked(true);
    reenterUpdate();
    m_setmode = 3;

    ui->ipradioButton->setEnabled(false);
    ui->maskradioButton->setEnabled(false);
    ui->gateradioButton->setEnabled(false);
    QTimer::singleShot(6000, ui->refreshpushButton,[btn = ui->refreshpushButton]() {btn->setEnabled(true);}); // 6s

    ui->iplabel->setText("---.---.---.---");
    ui->masklabel->setText("---.---.---.---");
    ui->gatelabel->setText("---.---.---.---");
    ConfigManager::setinterfaces(false, "", "", "");
}

void devicesetting::on_refreshpushButton_clicked()
{
    if (ConfigManager::getNetworkConfig()){
        ui->iplabel->setText(ConfigManager::s_IP);
        ui->masklabel->setText(ConfigManager::s_SM);
        ui->gatelabel->setText(ConfigManager::s_Gateway);
    }
}

void devicesetting::on_setstaticradioButton_clicked()
{
    ui->ipradioButton->setChecked(true);
    reenterUpdate();
    m_setmode = 0;

    ui->ipradioButton->setEnabled(true);
    ui->maskradioButton->setEnabled(true);
    ui->gateradioButton->setEnabled(true);
    ui->refreshpushButton->setEnabled(false);
}


void devicesetting::reenterUpdate()
{
    ui->ipvaluelabel->setText("");
    ui->maskvaluelabel->setText("");
    ui->gatevaluelabel->setText("");
    ui->gpibvaluelabel->setText("");
    ui->canidvaluelabel->setText("");
}

void devicesetting::setting(const QString &value)
{
    QString *fields[] = { &ConfigManager::s_IP, &ConfigManager::s_SM, &ConfigManager::s_Gateway };
    QLabel *valueLabels[] = { ui->ipvaluelabel, ui->maskvaluelabel, ui->gatevaluelabel };
    QLabel *displayLabels[] = { ui->iplabel, ui->masklabel, ui->gatelabel };

    if (m_setmode < 3) {
        valueLabels[m_setmode]->setText(value);
        *fields[m_setmode] = value;

        if (ConfigManager::s_IP != "---.---.---.---" && ConfigManager::s_SM != "---.---.---.---" && ConfigManager::s_Gateway != "---.---.---.---") {
            ConfigManager::setinterfaces(true, ConfigManager::s_IP, ConfigManager::s_SM, ConfigManager::s_Gateway);
            for (int i = 0; i < 3; i++) {displayLabels[i]->setText(*fields[i]);} // update show
        }
    } else if (m_setmode == 3) {
        ConfigManager::setConfigValue("Device/CANID", value);
        ConfigManager::s_CANid = value.toUInt();
        ui->canidvaluelabel->setText(value);
        ui->canidlabel->setText(value);
    }
}
