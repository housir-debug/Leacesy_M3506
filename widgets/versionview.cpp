#include "versionview.h"
#include "ui_versionview.h"
#include "auxiliary/config_manager.h"

versionview::versionview(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::versionview)
{
    ui->setupUi(this);

    ui->deswvaluelabel->setText(ConfigManager::s_firmwareVersion);
    ui->dehwvaluelabel->setText(ConfigManager::s_hardwareVersion);
}

versionview::~versionview()
{
    delete ui;
}

void versionview::setChannelName(int channel){
    QString name = QString("CH-%1").arg(channel, 2, 10, QChar('0'));
    ui->channellabel->setText(name);
}

void versionview::setChSWVersion(const QString &ver){ui->chswvaluelabel->setText(ver);}

void versionview::setChHWVersion(const QString &ver){ui->chhwvaluelabel->setText(ver);}
