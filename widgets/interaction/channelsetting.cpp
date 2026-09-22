#include "channelsetting.h"
#include "ui_channelsetting.h"

channelsetting::channelsetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::channelsetting)
{
    ui->setupUi(this);
}

channelsetting::~channelsetting()
{
    delete ui;
}
