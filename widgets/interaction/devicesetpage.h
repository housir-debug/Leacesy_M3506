#pragma once
#include <QWidget>
#include "../component/numberkeypad.h"

namespace Ui {class devicesetpage;}

class devicesetpage : public QWidget
{
    Q_OBJECT

public:
    explicit devicesetpage(QWidget *parent = nullptr);
    ~devicesetpage();

    void responseUpdate();

private:
    numberkeypad* m_setnmbkeycard;
    Ui::devicesetpage *ui;
    int m_setmode{0};

signals:
    void jump_back();
    void set_network(bool isstatic);
    void set_canbaud();
    void set_RS232Baud();

private slots:
    void on_back_pushButton_clicked();
    void on_dhcp_pushButton_clicked();
    void on_static_pushButton_clicked();
    void on_refreshpushButton_clicked();

    void on_ipradioButton_clicked();
    void on_maskradioButton_clicked();
    void on_gateradioButton_clicked();
    void on_canidradioButton_clicked();
    void on_canbaudradioButton_clicked();
    void on_rs232baudradioButton_clicked();
};

namespace Setmode {
    constexpr const int ip             = 0;
    constexpr const int mask           = 1;
    constexpr const int gate           = 2;
    constexpr const int canid          = 3;
    constexpr const int canbuat        = 4;
    constexpr const int rs232buat      = 5;
}
