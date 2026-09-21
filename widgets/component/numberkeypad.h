#pragma once
#include <QFrame>

namespace Ui {class numberkeypad;}

class numberkeypad : public QFrame
{
    Q_OBJECT

private:
    Ui::numberkeypad *ui;

    bool m_isPressed;
    QTimer *m_longPressTimer;
    bool m_longPressTriggered;

public:
    explicit numberkeypad(QWidget *parent = nullptr);
    ~numberkeypad();

    QString getvalue() const;
    void setValue(const QString &value);

signals:
    void valueEntered(const QString &value);

private slots:
    void on_deletebtn_clicked();
    void on_btn0_clicked();
    void on_btn1_clicked();
    void on_btn2_clicked();
    void on_btn3_clicked();
    void on_btn4_clicked();
    void on_btn5_clicked();
    void on_btn6_clicked();
    void on_btn7_clicked();
    void on_btn8_clicked();
    void on_btn9_clicked();
    void on_btndot_clicked();
    void on_btnenter_clicked();
    void insertText(const QString &text);
};

