#include "numberkeypad.h"
#include "ui_numberkeypad.h"

numberkeypad::numberkeypad(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::numberkeypad)
{
    ui->setupUi(this);
}

numberkeypad::~numberkeypad()
{
    delete ui;
}
