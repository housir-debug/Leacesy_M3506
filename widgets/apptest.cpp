#include "interaction/channelsetpage.h"
#include "interaction/devicesetpage.h"
#include "interaction/channelpage.h"
#include "auxiliary/global_static.h"
#include "ui_apptest.h"
#include "apptest.h"

test::test(QWidget *parent) : QWidget(parent),
    ui(new Ui::test)
{
    ui->setupUi(this);

    channelpage* m_ChPage = new channelpage(this);
    ui->stackedWidget->addWidget(m_ChPage);

    /*channelsetting* m_ChSetPage = new channelsetting(this);
    ui->stackedWidget->addWidget(m_ChSetPage);// 1*/

    QList<int> channels = {3, 1, 4,2,8,9,24,15,32,12,18};
    //for (int i = 1; i <= 36; i++) {channels.append(i);}
    std::sort(channels.begin(), channels.end());

    for (const int ch : channels) {
        m_ChPage->update_SoftVer(ch,"1.2.3.4");
        m_ChPage->update_HardVer(ch,"4.3.2.1");
        //m_ChSetPage->update_ChEn(ch);
    }

    /*devicesetpage* m_DeviceSetPage = new devicesetpage(this);
    ui->stackedWidget->addWidget(m_DeviceSetPage);// 2*/

    qCDebug(aPTest) << "objectName:" << m_ChPage->objectName();

    //numberkeypad *key = new numberkeypad(this);
    //key->setContentsMargins(0, 0, 0, 0);
}

test::~test()
{
    delete ui;
}
