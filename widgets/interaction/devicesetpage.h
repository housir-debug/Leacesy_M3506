#pragma once
#include <QWidget>
namespace Ui {class devicesetpage;}

class devicesetpage : public QWidget
{
    Q_OBJECT

private:
    Ui::devicesetpage *ui;
    int m_setmode{0};

signals:
    void backRequested();

private slots:
    void on_ipradioButton_clicked();
    void on_maskradioButton_clicked();
    void on_gateradioButton_clicked();
    void on_canidradioButton_clicked();
    void on_canBaud_comboBox_activated(int index);
    void on_rs32Baud_comboBox_activated(int index);

    void on_back_pushButton_clicked();
    void on_dhcp_pushButton_clicked();
    void on_static_pushButton_clicked();
    void on_refreshpushButton_clicked();

public:
    explicit devicesetpage(QWidget *parent = nullptr);
    ~devicesetpage();

    void responseUpdate();
};

namespace Setmode {
    constexpr const int ip             = 0;
    constexpr const int mask           = 1;
    constexpr const int gate           = 2;
    constexpr const int canid          = 3;
}
