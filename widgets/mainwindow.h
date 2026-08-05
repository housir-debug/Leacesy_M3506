#pragma once
#include <QWidget>
#include <QLoggingCategory>
#include "auxiliary/battery_model.h"
#include "auxiliary/config_manager.h"
#include "digitalcard.h"
#include "batterycard.h"
#include "versionview.h"
#include "chstatusview.h"
#include "numberkeypad.h"
#include "devicesetting.h"
#include "remoteoverlay.h"

Q_DECLARE_LOGGING_CATEGORY(widget)

namespace Ui {class Mainwindow;}

class Mainwindow : public QWidget
{
    Q_OBJECT

private:
    Ui::Mainwindow *ui;
    versionview* m_vercard;
    chstatusview* m_chstatuscard;
    digitalcard* m_funcdigitalcard;
    batterycard* m_funcbatterycard;
    numberkeypad* m_funcnmbkeycard;
    devicesetting* m_devicesetcard;
    numberkeypad* m_setnmbkeycard;
    remoteoverlay* m_remoteOverlay;

    QMap<int,digitalcard*> m_digitalcards;
    QMap<int,batterycard*> m_batterycards;

    QMap<int,QString> m_ChsoftVer;
    QMap<int,QString> m_ChhardVer;
    QMap<int,QString> m_range = {
        {0x00, "A"},
        {0x01, "mA"},
        {0x10, "Auto"},
    };

    int m_functioncsetmode{0};
    int m_functioncCh{0};
    int m_initalpage{0};

public:
    void update_cardtest();
    void update_SoftVer(int ch,const QString &ver);
    void update_HardVer(int ch,const QString &ver);
    void update_IsOutput(int ch,bool status);
    void update_Voltage(int ch,float voltage);
    void update_CurrentAndUnit(int ch,float current);
    void update_Cv(int ch,float cv);
    void update_Cc(int ch,float cc);
    void update_Ovp(int ch,float ovp);
    void update_Range(int ch,int range);
    void update_Status(int ch,quint16 status);

    void update_Imp(int ch,float imp);
    void update_remotemodel(int reface);

    bool load_BatteryModel();
    QJsonArray getAllChannelsData();

    void update_setting();
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow();

    QStringList m_currentModelList;
    std::shared_ptr<BatteryModelManager> m_modelManager{nullptr};

signals:
    void set_network(bool isstatic);
    void set_canbaud();
    void set_RS232Baud();

    #define CHANNEL(n) void to_UartChannel##n(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);

    CHANNEL_COUNT
    #undef CHANNEL

private slots:
    void to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param);
    void allONrefresh();

    // digital / battery to switchs cards
    void on_digitalsettingspushButton_clicked();
    void on_batterysettingspushButton_clicked();
    void on_digitalmodepushButton_clicked();
    void on_batterymodepushButton_clicked();

    void on_digitalrowsbackpushButton_clicked();
    void on_digitalrowsnextpushButton_clicked();
    void on_batteryrowsbackpushButton_clicked();
    void on_batteryrowsnextpushButton_clicked();

    void on_digitalallONpushButton_clicked();
    void on_digitalallunitpushButton_clicked();
    void on_batteryallONpushButton_clicked();
    void on_batteryallmodelpushButton_clicked();

    // setting page switchs
    void on_settingrowsbackpushButton_clicked();

    // function page switchs
    void on_functionrowsbackpushButton_clicked();
    void on_functionsettingspushButton_clicked();
    void on_functionallapplypushButton_clicked();

    void on_cvradioButton_clicked();
    void on_ccradioButton_clicked();
    void on_ovpradioButton_clicked();
    void on_functionunitpushButton_clicked();

    void on_socradioButton_clicked();
    void on_capradioButton_clicked();
    void on_functionmodelpushButton_clicked();
};

