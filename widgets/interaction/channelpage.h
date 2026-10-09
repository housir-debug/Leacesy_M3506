#pragma once
#include <QWidget>
#include "auxiliary/config_manager.h"
#include "../component/channelcard.h"

namespace Ui {class channelpage;}

class channelpage : public QWidget
{
    Q_OBJECT

public:
    explicit channelpage(QWidget *parent = nullptr);
    ~channelpage();

    QJsonArray getAllChannelsData();
    void update_cardtest();

    void update_SoftVer(int ch,const QString &ver);
    void update_HardVer(int ch,const QString &ver);
    channelcard* create_chcard(int ch);

    void update_IsOutput(int ch,bool status);
    void update_Voltage(int ch,float voltage);
    void update_CurrentAndUnit(int ch,float current);
    void update_Cv(int ch,float cv);
    void update_Cc(int ch,float cc);
    void update_Ovp(int ch,float ovp);
    void update_Imp(int ch,float imp);
    void update_Range(int ch,int range);
    void update_Status(int ch,quint16 status);

    bool load_BatteryModel();
    QStringList m_currentModelList;
    std::shared_ptr<BatteryModelManager> m_modelManager{nullptr};

private:
    Ui::channelpage *ui;
    QMap<int,channelcard*> m_channels;

    QMap<int,QString> m_range;
    QMap<QString, int> m_rangeReverse;
    void to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param);

signals:
    #define CHANNEL(n) void to_UartChannel##n(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);

    CHANNEL_COUNT
    #undef CHANNEL

    void jump_chsetpage();
    void jump_devicesetpage();

private slots:
    void on_set_pushButton_clicked();
    void on_ver_pushButton_clicked();
    void on_batt_pushButton_clicked();
    void on_dig_pushButton_clicked();
    void on_pre_pushButton_clicked();
    void on_next_pushButton_clicked();
};

namespace Chindex {
    constexpr int Digital      = 0x00;
    constexpr int Battery      = 0x01;
    constexpr int Version      = 0x02;
}
