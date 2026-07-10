#include "chstatusview.h"
#include "ui_chstatusview.h"
#include <QStyle>
#include <QDebug>

chstatusview::chstatusview(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::chstatusview)
{
    ui->setupUi(this);
}

chstatusview::~chstatusview()
{
    delete ui;
}

void chstatusview::setChtatus(const QString &status)
{
    if (status.size() == 16){
        for (int i = 0; i < 16; ++i) {
            QString btnName = QString("pushButton%1").arg(i+1);
            QPushButton *btn = findChild<QPushButton*>(btnName);

            bool statuschar  = status.at(i) == "1";
            btn->setProperty("status", statuschar);

            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
            btn->update();
        }
    }
}
