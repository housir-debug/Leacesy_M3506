#pragma once
#include <QWidget>
namespace Ui {class channelsetting;}

class channelsetting : public QWidget
{
    Q_OBJECT

private:
    Ui::channelsetting *ui;
    int m_setmode{0};

signals:
    void to_chModel(int ch,const QString model);
    void to_chCAP(int ch,float val);
    void to_chSOC(int ch,float val);
    void backRequested();

private slots:
    void on_cv_pushButton_clicked();
    void on_cc_pushButton_clicked();
    void on_ovp_pushButton_clicked();
    void on_soc_pushButton_clicked();
    void on_cap_pushButton_clicked();
    void on_back_pushButton_clicked();
    void on_allFO_pushButton_clicked();
    void on_allNO_pushButton_clicked();
    void setAllChannelsChecked(bool checked);
    void sendModecmd(float val,quint8 func);

public:
    explicit channelsetting(QWidget *parent = nullptr);
    ~channelsetting();

    void setCombModel();
    void update_ChEn(int ch);
};

namespace channelSet_mode {
    constexpr const int cv             = 0;
    constexpr const int cc             = 1;
    constexpr const int ovp            = 2;
    constexpr const int soc            = 3;
    constexpr const int cap            = 4;
}
