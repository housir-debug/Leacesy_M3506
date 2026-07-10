#include "test.h"
#include "ui_test.h"
#include "digitalcard.h"
#include "batterycard.h"
#include "numberkeypad.h"
#include <QDebug>

test::test(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::test)
{
    ui->setupUi(this);

    /*digitalcard *card = new digitalcard(this);
    card->setContentsMargins(0, 0, 0, 0);*/

    /*batterycard *card = new batterycard(this);
    card->setContentsMargins(0, 0, 0, 0);*/

    //qDebug() << "objectName:" << card->objectName();

    numberkeypad *key = new numberkeypad(this);
    key->setContentsMargins(0, 0, 0, 0);
}

test::~test()
{
    delete ui;
}
