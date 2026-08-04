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
    ui->canidlabel->setText(QString::number(ConfigManager::s_CANid.load()));
    ui->canbaudlabel->setText(QString::number(ConfigManager::s_canBaudRate));
    ui->rs232baudlabel->setText(QString::number(ConfigManager::s_rs232BaudRate));

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

void devicesetting::on_canbaudradioButton_clicked()
{
    m_setmode = 4;
    ui->canbaudradioButton->setChecked(true);
}

void devicesetting::on_rs232baudradioButton_clicked()
{
    m_setmode = 5;
    ui->rs232baudradioButton->setChecked(true);
}


void devicesetting::on_dhcpradioButton_clicked()
{
    ui->canidradioButton->setChecked(true);
    reenterUpdate();
    m_setmode = 3;

    ui->ipradioButton->setEnabled(false);
    ui->maskradioButton->setEnabled(false);
    ui->gateradioButton->setEnabled(false);
    ui->iplabel->setText("---.---.---.---");
    ui->masklabel->setText("---.---.---.---");
    ui->gatelabel->setText("---.---.---.---");

    emit set_network(false);
    QTimer::singleShot(8100, this,[this]() {on_refreshpushButton_clicked();ui->refreshpushButton->setEnabled(true);}); // 8.1s
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


void devicesetting::setting(const QString &value)
{
    switch (m_setmode) {
        case 0:case 1:case 2:{
            QString *fields[] = { &ConfigManager::s_IP, &ConfigManager::s_SM, &ConfigManager::s_Gateway };
            QLabel *valueLabels[] = { ui->ipvaluelabel, ui->maskvaluelabel, ui->gatevaluelabel };
            valueLabels[m_setmode]->setText(value);
            *fields[m_setmode] = value;

            if (ConfigManager::s_IP != "---.---.---.---" &&
                ConfigManager::s_SM != "---.---.---.---" &&
                ConfigManager::s_Gateway != "---.---.---.---") {
                    emit set_network(true);
            }
            return;
        }
        case 3:
            ui->canidvaluelabel->setText(value);
            ConfigManager::s_CANid = value.toUInt();
            if (ConfigManager::setConfigValue("Device/CANID",ConfigManager::s_CANid.load())){
                ui->canidlabel->setText(QString::number(ConfigManager::s_CANid.load()));
            }
            return;
        case 4:
            ConfigManager::s_canBaudRate = value.toUInt();
            ui->canbaudvaluelabel->setText(value);
            emit set_canbaud();
            return;
        case 5:
            ConfigManager::s_rs232BaudRate = value.toUInt();
            ui->rs232baudvaluelabel->setText(value);
            emit set_RS232Baud();
            return;
        default:return;
    }
}

void devicesetting::responseUpdate()
{
    ui->iplabel->setText(ConfigManager::s_IP);
    ui->masklabel->setText(ConfigManager::s_SM);
    ui->gatelabel->setText(ConfigManager::s_Gateway);
    ui->canidlabel->setText(QString::number(ConfigManager::s_CANid.load()));
    ui->canbaudlabel->setText(QString::number(ConfigManager::s_canBaudRate));
    ui->rs232baudlabel->setText(QString::number(ConfigManager::s_rs232BaudRate));
}

void devicesetting::reenterUpdate()
{
    ui->ipvaluelabel->setText("");
    ui->maskvaluelabel->setText("");
    ui->gatevaluelabel->setText("");
    ui->canidvaluelabel->setText("");
    ui->canbaudvaluelabel->setText("");
    ui->rs232baudvaluelabel->setText("");
}
