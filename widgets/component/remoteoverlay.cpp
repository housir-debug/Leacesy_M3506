#include "remoteoverlay.h"
#include "ui_remoteoverlay.h"
#include <QGraphicsOpacityEffect>
#include <QShowEvent>
#include <QHideEvent>

remoteoverlay::remoteoverlay(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::remoteoverlay)
{
    QGraphicsOpacityEffect *logoOpacity = new QGraphicsOpacityEffect(this);
    logoOpacity->setOpacity(0.6);
    ui->setupUi(this);

    ui->remotelogo->setGraphicsEffect(logoOpacity);
    hide();
}

remoteoverlay::~remoteoverlay()
{
    delete ui;
}

void remoteoverlay::showEvent(QShowEvent *event)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    QFrame::showEvent(event);
}

void remoteoverlay::hideEvent(QHideEvent *event)
{
    QFrame::hideEvent(event);
}

void remoteoverlay::on_gobackpushButton_clicked()
{
    emit exitRemote();
    hide();
}
