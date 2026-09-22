#include "devicesetpage.h"
#include "ui_devicesetpage.h"
#include "auxiliary/config_manager.h"
#include <QTimer>

devicesetpage::devicesetpage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::devicesetpage)
{
    ui->setupUi(this);

    ui->hardver_label->setText(Device::HardVer);
    ui->softver_label->setText(Device::SoftVer);
    ui->macaddress_label->setText(ConfigManager::s_MAC);

    ui->iplabel->setText(ConfigManager::s_IP);
    ui->masklabel->setText(ConfigManager::s_SM);
    ui->gatelabel->setText(ConfigManager::s_Gateway);
    ui->canidlabel->setText(QString::number(ConfigManager::s_CANid.load()));
    ui->canbaudlabel->setText(QString::number(ConfigManager::s_canBaudRate));
    ui->rs232baudlabel->setText(QString::number(ConfigManager::s_rs232BaudRate));

    if (ConfigManager::s_isDHCP){
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

    m_setnmbkeycard = new numberkeypad(ui->keyframe->parentWidget());
    m_setnmbkeycard->setGeometry(ui->keyframe->geometry());
    QObject::connect(m_setnmbkeycard, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){
            switch (m_setmode) {
                case 0:case 1:case 2:{
                    QString *fields[] = { &ConfigManager::s_IP, &ConfigManager::s_SM, &ConfigManager::s_Gateway };
                    QLabel *valueLabels[] = { ui->iplabel, ui->masklabel, ui->gatelabel };
                    valueLabels[m_setmode]->setText(value);
                    *fields[m_setmode] = value;

                    if (ConfigManager::s_IP != "---.---.---.---" && ConfigManager::s_SM != "---.---.---.---"
                            && ConfigManager::s_Gateway != "---.---.---.---") {
                            emit set_network(true);
                    }
                    return;
                }
                case 3:
                    ui->canidlabel->setText(value);
                    ConfigManager::s_CANid = value.toUInt();
                    if (ConfigManager::setConfigValue("Device/CANID",ConfigManager::s_CANid.load())){
                        ui->canidlabel->setText(QString::number(ConfigManager::s_CANid.load()));
                    }
                    return;
                case 4:
                    ConfigManager::s_canBaudRate = value.toUInt();
                    ui->canbaudlabel->setText(value);
                    emit set_canbaud();
                    return;
                case 5:
                    ConfigManager::s_rs232BaudRate = value.toUInt();
                    ui->rs232baudlabel->setText(value);
                    emit set_RS232Baud();
                    return;
                default:return;
            }
        }});
    delete ui->keyframe;
}

devicesetpage::~devicesetpage()
{
    delete ui;
}

void devicesetpage::on_back_pushButton_clicked(){emit jump_back();}

void devicesetpage::on_dhcp_pushButton_clicked()
{
    ui->ipradioButton->setEnabled(false);
    ui->maskradioButton->setEnabled(false);
    ui->gateradioButton->setEnabled(false);
    ui->canidradioButton->setChecked(true);
    m_setmode = Setmode::canid;

    emit set_network(false);
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
        ui->iplabel->setText(ConfigManager::s_IP);
        ui->masklabel->setText(ConfigManager::s_SM);
        ui->gatelabel->setText(ConfigManager::s_Gateway);
    }
}


void devicesetpage::responseUpdate()
{
    ui->canidlabel->setText(QString::number(ConfigManager::s_CANid.load()));
    ui->canbaudlabel->setText(QString::number(ConfigManager::s_canBaudRate));
    ui->rs232baudlabel->setText(QString::number(ConfigManager::s_rs232BaudRate));

    if (ConfigManager::s_isDHCP){
        QTimer::singleShot(8100, this,[this]() {on_refreshpushButton_clicked();ui->refreshpushButton->setEnabled(true);}); // 8.1s
    }else{
        on_refreshpushButton_clicked();ui->refreshpushButton->setEnabled(false);
    }
}


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

void devicesetpage::on_canbaudradioButton_clicked()
{
    m_setmode = Setmode::canbuat;
    ui->canbaudradioButton->setChecked(true);
}

void devicesetpage::on_rs232baudradioButton_clicked()
{
    m_setmode = Setmode::rs232buat;
    ui->rs232baudradioButton->setChecked(true);
}
