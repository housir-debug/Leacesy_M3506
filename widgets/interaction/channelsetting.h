#pragma once
#include <QWidget>
#include "auxiliary/battery_model.h"
#include "auxiliary/config_manager.h"
#include "../component/numberkeypad.h"

namespace Ui {class channelsetting;}

class channelsetting : public QWidget
{
    Q_OBJECT

public:
    explicit channelsetting(QWidget *parent = nullptr);
    ~channelsetting();

    std::shared_ptr<BatteryModelManager> m_modelManager{nullptr};

signals:
    #define CHANNEL(n) void to_UartChannel##n(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
    CHANNEL_COUNT
    #undef CHANNEL

    void to_chModel(int ch,const QString model);
    void to_chSOC(int ch,float val);
    void to_chCAP(int ch,float val);
    void backRequested();

private slots:
    void on_back_pushButton_clicked();
    void on_allFO_pushButton_clicked();
    void on_allNO_pushButton_clicked();
    void setAllChannelsChecked(bool checked);
    void on_cv_pushButton_clicked();
    void on_cc_pushButton_clicked();
    void on_ovp_pushButton_clicked();
    void on_soc_pushButton_clicked();
    void on_cap_pushButton_clicked();

private:
    numberkeypad* m_setnmbkeycard;
    Ui::channelsetting *ui;
    int m_setmode{0};

    void sendModecmd(float val,quint8 func);
    void to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param);
};

namespace channelSet_mode {
    constexpr const int cv             = 0;
    constexpr const int cc             = 1;
    constexpr const int ovp            = 2;
    constexpr const int soc            = 3;
    constexpr const int cap            = 4;
}
