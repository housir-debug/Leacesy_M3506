#pragma once
#include <QWidget>
#include "../component/channelcard.h"

namespace Ui {class channelpage;}

class channelpage : public QWidget
{
    Q_OBJECT

private:
    Ui::channelpage *ui;
    QMap<int,channelcard*> m_channels;

    QMap<int,QString> m_range;
    QMap<QString, int> m_rangeReverse;

signals:
    void jump_chsetpage();
    void jump_devicesetpage();

private slots:
    void on_setDe_pushButton_clicked();
    void on_setCh_pushButton_clicked();
    void on_dig_pushButton_clicked();
    void on_batt_pushButton_clicked();
    void on_stat_pushButton_clicked();
    void on_ver_pushButton_clicked();
    void on_pre_pushButton_clicked();
    void on_next_pushButton_clicked();
    void to_channelpage(int newindex);

public:
    explicit channelpage(QWidget *parent = nullptr);
    ~channelpage();

    /* create channel card */
    void update_cardtest();
    void update_SoftVer(int ch,const QString &ver);
    void update_HardVer(int ch,const QString &ver);
    channelcard* create_chcard(int ch);

    /* update card style */
    void update_IsOutput(int ch,bool status);
    void update_Voltage(int ch,float voltage);
    void update_CurrentAndUnit(int ch,float current);
    void update_Cv(int ch,float cv);
    void update_Cc(int ch,float cc);
    void update_Ovp(int ch,float ovp);
    void update_Imp(int ch,float imp);
    void update_Range(int ch,int range);
    void update_Status(int ch,quint16 status);

    /* web or main call function */
    bool load_BatteryModel();
    QJsonArray getAllChannelsData();

    QStringList m_currentModelList;
    std::shared_ptr<BatteryModelManager> m_modelManager{nullptr};
};

namespace ChPageCard {
// (1138,316) 220*5=1100 1138-1100=38 38/6=6-2
    constexpr int PageCard     = 0x05;
    constexpr int CardSpace    = 0x06;
    constexpr int TBMargin     = 0x02;
    constexpr int RLMargin     = 0x07;
}
