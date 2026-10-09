#include "test.h"
#include "ui_test.h"
#include "component/numberkeypad.h"
#include <QDebug>

test::test(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::test)
{
    ui->setupUi(this);

    //qDebug() << "objectName:" << card->objectName();

    numberkeypad *key = new numberkeypad(this);
    key->setContentsMargins(0, 0, 0, 0);
}

test::~test()
{
    delete ui;
}
