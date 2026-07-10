#include "numberkeypad.h"
#include "ui_numberkeypad.h"
#include <QTimer>

numberkeypad::numberkeypad(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::numberkeypad)
{
    ui->setupUi(this);

    connect(ui->deletebtn, &QPushButton::pressed, this, [this]() {
        m_isPressed = true;
        m_longPressTriggered = false;
        m_longPressTimer->start();
    });

    connect(ui->deletebtn, &QPushButton::released, this, [this]() {
        if (m_isPressed){
            m_longPressTimer->stop();
        }

        m_isPressed = false;
    });

    m_longPressTimer = new QTimer(this);
    m_longPressTimer->setSingleShot(true);
    m_longPressTimer->setInterval(900);   // 900 ms

    connect(m_longPressTimer, &QTimer::timeout, this, [this]{
        if (m_isPressed && !m_longPressTriggered) {
            m_longPressTriggered = true;
            ui->displayfield->clear();
        }
    });
}

numberkeypad::~numberkeypad()
{
    delete ui;
}

void numberkeypad::on_btn0_clicked()
{
    insertText("0");
}

void numberkeypad::on_btn1_clicked()
{
    insertText("1");
}

void numberkeypad::on_btn2_clicked()
{
    insertText("2");
}

void numberkeypad::on_btn3_clicked()
{
    insertText("3");
}

void numberkeypad::on_btn4_clicked()
{
    insertText("4");
}

void numberkeypad::on_btn5_clicked()
{
    insertText("5");
}

void numberkeypad::on_btn6_clicked()
{
    insertText("6");
}

void numberkeypad::on_btn7_clicked()
{
    insertText("7");
}

void numberkeypad::on_btn8_clicked()
{
    insertText("8");
}

void numberkeypad::on_btn9_clicked()
{
    insertText("9");
}

void numberkeypad::on_btndot_clicked()
{
    insertText(".");
}

void numberkeypad::on_btnenter_clicked()
{
    insertText("↵");
}

void numberkeypad::on_deletebtn_clicked()
{
    QString current = ui->displayfield->text();

    if (!current.isEmpty()){
        int pos = ui->displayfield->cursorPosition();
        if (pos < 0 || pos > current.length()) {
            pos = current.length();
        }

        current.remove(pos - 1, 1);
        ui->displayfield->setText(current);
        ui->displayfield->setCursorPosition(pos - 1);
    }
}

void numberkeypad::insertText(const QString &text)
{
    QString current = ui->displayfield->text();

    if (text != "↵"){
        int pos = ui->displayfield->cursorPosition();
        if (pos < 0 || pos > current.length()) {
            pos = current.length();
        }

        current.insert(pos, text);
        ui->displayfield->setText(current);
        ui->displayfield->setCursorPosition(pos + 1);
    }else {
        if (!current.isEmpty()){
            emit valueEntered(current);
            ui->displayfield->setText("");
        }
    }
}


QString numberkeypad::getvalue() const
{
    return ui->displayfield->text();
}

void numberkeypad::setValue(const QString &value)
{
    ui->displayfield->setText(value);
}
