#include "test.h"
#include "ui_test.h"
#include "digitalcard.h"
#include <QDebug>

test::test(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::test)
{
    ui->setupUi(this);

    digitalcard *card = new digitalcard(this);
    card->setContentsMargins(0, 0, 0, 0);
    card->setChannelName("CH-1");

    qDebug() << "objectName:" << card->objectName();
}

test::~test()
{
    delete ui;
}
