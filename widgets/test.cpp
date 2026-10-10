#include "test.h"
#include "ui_test.h"
#include "widgets/interaction/channelsetting.h"
#include "widgets/interaction/devicesetpage.h"
#include "widgets/interaction/channelpage.h"
#include "component/numberkeypad.h"
#include <QDebug>

test::test(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::test)
{
    ui->setupUi(this);

    channelpage* m_ChPage = new channelpage(this);
    ui->stackedWidget->addWidget(m_ChPage);
    m_ChPage->update_cardtest();

    /*channelsetting* m_ChSetPage = new channelsetting(this);
    ui->stackedWidget->addWidget(m_ChSetPage);// 1*/

    /*devicesetpage* m_DeviceSetPage = new devicesetpage(this);
    ui->stackedWidget->addWidget(m_DeviceSetPage);// 2*/

    //qDebug() << "objectName:" << card->objectName();

    //numberkeypad *key = new numberkeypad(this);
    //key->setContentsMargins(0, 0, 0, 0);
}

test::~test()
{
    delete ui;
}
