#pragma once
#include <QFrame>

namespace Ui {class devicesetting;}

class devicesetting : public QFrame
{
    Q_OBJECT

signals:
    void set_network(bool isstatic);
    void set_canbaud();
    void set_RS232Baud();

public:
    explicit devicesetting(QWidget *parent = nullptr);
    ~devicesetting();

    void setting(const QString &value);
    void responseUpdate();

private:
    Ui::devicesetting *ui;

    void reenterUpdate();
    int m_setmode{0};

private slots:
    void on_ipradioButton_clicked();
    void on_maskradioButton_clicked();
    void on_gateradioButton_clicked();
    void on_canidradioButton_clicked();
    void on_canbaudradioButton_clicked();
    void on_rs232baudradioButton_clicked();

    void on_dhcpradioButton_clicked();
    void on_refreshpushButton_clicked();
    void on_setstaticradioButton_clicked();
};
