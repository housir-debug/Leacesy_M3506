#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "ui_devicesetpage.h"
#include "devicesetpage.h"
#include <QStandardItem>
#include <QTimer>

/* slots function */
void devicesetpage::on_ipradioButton_clicked()
{
    m_Inmode = Setmode::ip;
    ui->ipradioButton->setChecked(true);
    ui->keyframe->setPlaceholderText("请输入地址");
}
void devicesetpage::on_maskradioButton_clicked()
{
    m_Inmode = Setmode::mask;
    ui->maskradioButton->setChecked(true);
    ui->keyframe->setPlaceholderText("请输入掩码");
}
void devicesetpage::on_gateradioButton_clicked()
{
    m_Inmode = Setmode::gate;
    ui->gateradioButton->setChecked(true);
    ui->keyframe->setPlaceholderText("请输入网关");
}
void devicesetpage::on_canidradioButton_clicked()
{
    m_Inmode = Setmode::canid;
    ui->canidradioButton->setChecked(true);
    ui->keyframe->setPlaceholderText("请输入ID");
}
void devicesetpage::on_canBaud_comboBox_activated(int index)
{
    m_Inmode = Setmode::canbaud;
    ui->canBaud_comboBox->setCurrentIndex(9);
    ui->canbaudradioButton->setChecked(true);
    ui->keyframe->setPlaceholderText("设置中...");

    ConfigManager::s_canBaudIndex = index;
    emit ChannelBus::instance().set_canbaud();
}
void devicesetpage::on_rs32Baud_comboBox_activated(int index)
{
    m_Inmode = Setmode::rs32baud;
    ui->rs32Baud_comboBox->setCurrentIndex(8);
    ui->rs232baudradioButton->setChecked(true);
    ui->keyframe->setPlaceholderText("设置中...");

    ConfigManager::s_rs232BaudIndex = index;
    emit ChannelBus::instance().set_RS232Baud();
}

void devicesetpage::on_back_pushButton_clicked(){emit backRequested();}
void devicesetpage::on_dhcp_pushButton_clicked()
{
    on_canidradioButton_clicked();
    ui->ipradioButton->setEnabled(false);
    ui->maskradioButton->setEnabled(false);
    ui->gateradioButton->setEnabled(false);
    ui->iplabel->setText("---.---.---.---");
    ui->masklabel->setText("---.---.---.---");
    ui->gatelabel->setText("---.---.---.---");
    ui->keyframe->setPlaceholderText("设置中...");

    emit ChannelBus::instance().set_network(false);
}
void devicesetpage::on_static_pushButton_clicked()
{
    on_ipradioButton_clicked();
    ui->ipradioButton->setEnabled(true);
    ui->maskradioButton->setEnabled(true);
    ui->gateradioButton->setEnabled(true);
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

devicesetpage::devicesetpage(QWidget *parent) :QWidget(parent),
    ui(new Ui::devicesetpage)
{
    ui->setupUi(this);
    ui->macaddress_label->setText(ConfigManager::MAC);

    auto* canmodel = qobject_cast<QStandardItemModel*>(ui->canBaud_comboBox->model());
    if (canmodel) {
        QStandardItem* item = canmodel->item(9);
        if (item){item->setEnabled(false);}
    }
    auto* r23model = qobject_cast<QStandardItemModel*>(ui->rs32Baud_comboBox->model());
    if (r23model) {
        QStandardItem* item = r23model->item(8);
        if (item){item->setEnabled(false);}
    }

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
        on_canidradioButton_clicked();
    }else{
        ui->static_pushButton->setChecked(true);
        ui->refreshpushButton->setEnabled(false);

        ui->ipradioButton->setEnabled(true);
        ui->maskradioButton->setEnabled(true);
        ui->gateradioButton->setEnabled(true);
        on_ipradioButton_clicked();
    }

    QObject::connect(ui->keyframe, &numberkeypad::valueEntered, this, [this](const QString &value) {
        if (!value.isEmpty()){
            switch (m_Inmode) {
                case Setmode::ip:case Setmode::mask:case Setmode::gate:{
                    QString *fields[] = { &ConfigManager::IP, &ConfigManager::SM, &ConfigManager::Gate };
                    QLabel *valueLabels[] = { ui->iplabel, ui->masklabel, ui->gatelabel };
                    valueLabels[m_Inmode]->setText("Temp: " + value);
                    *fields[m_Inmode] = value;

                    if (ConfigManager::IP == "---.---.---.---"){
                        on_ipradioButton_clicked();
                    }else if(ConfigManager::SM == "---.---.---.---"){
                        on_maskradioButton_clicked();
                    }else if(ConfigManager::Gate == "---.---.---.---"){
                        on_gateradioButton_clicked();
                    }else{
                        emit ChannelBus::instance().set_network(true);
                        ui->keyframe->setPlaceholderText("设置中...");
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
        }}, Qt::DirectConnection);
}
devicesetpage::~devicesetpage()
{
    delete ui;
}

/* update card style */
void devicesetpage::responseUpdate()
{
    if (m_Inmode == Setmode::canbaud){
        ui->canidlabel->setText(QString::number(ConfigManager::s_CANid.load()));
        ui->canBaud_comboBox->setCurrentIndex(ConfigManager::s_canBaudIndex);
        ui->keyframe->setPlaceholderText("Please input");
    }else if(m_Inmode == Setmode::rs32baud){
        ui->keyframe->setPlaceholderText("Please input");
        ui->rs32Baud_comboBox->setCurrentIndex(ConfigManager::s_rs232BaudIndex);
    }else if(m_Inmode == Setmode::canid){
        QTimer::singleShot(8100, this,[this]() { // 8.1s
            ui->keyframe->setPlaceholderText("Please input");
            ui->refreshpushButton->setEnabled(true);
            on_refreshpushButton_clicked();
            });
    }else{
        ui->keyframe->setPlaceholderText("Please input");
        ui->refreshpushButton->setEnabled(false);
        on_refreshpushButton_clicked();
    }
}
