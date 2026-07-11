#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLoggingCategory>
#include <QStandardItemModel>
#include "digitalcard.h"
#include "batterycard.h"
#include "versionview.h"
#include "chstatusview.h"
#include "numberkeypad.h"
#include "remoteoverlay.h"
#include "auxiliary/battery_model.h"
#include "auxiliary/config_manager.h"

Q_DECLARE_LOGGING_CATEGORY(widget)

namespace Ui {
    class Mainwindow;
}

class Mainwindow : public QWidget
{
    Q_OBJECT

signals:
    void to_CANid(QString id);
    void to_GPIBid(QString id);

    #define CHANNEL(n) \
        void to_UartChannel##n(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);

    CHANNEL_COUNT
    #undef CHANNEL

public:

    // QJsonArray getAllChannelsData();
    //void setChannel_Setstatus(int channel,int model,const QString& val);

    void update_remotemodel(quint8 reface);
    void update_Configuration(int model,const QString& val);

public:
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow();

    void load_BatteryModel();
    QStringList m_currentModelList;
    std::shared_ptr<BatteryModelManager> m_modelManager{nullptr};

    void update_SoftVer(int ch,const QString &ver);
    void update_HardVer(int ch,const QString &ver);

    void update_IsOutput(int ch,bool status);
    void update_Voltage(int ch,float voltage);
    void update_CurrentAndUnit(int ch,float current);
    void update_Range(int ch,quint8 range);
    void update_Status(int ch,quint16 status);
    void update_Cv(int ch,float cv);
    void update_Cc(int ch,float cc);
    void update_Ovp(int ch,float ovp);
    void update_Imp(int ch,float imp);

private slots:
    void to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param);

    // channel card switchs
    void on_digitalsettingspushButton_clicked();
    void on_batterysettingspushButton_clicked();

    void on_digitalrowsbackpushButton_clicked();
    void on_digitalrowsnextpushButton_clicked();
    void on_batteryrowsbackpushButton_clicked();
    void on_batteryrowsnextpushButton_clicked();

    void on_digitalmodepushButton_clicked();
    void on_batterymodepushButton_clicked();

    void on_digitalallONpushButton_clicked();
    void on_digitalallunitpushButton_clicked();
    void on_batteryallONpushButton_clicked();
    void on_batteryallmodelpushButton_clicked();

    // function page switchs
    void on_functionsettingspushButton_clicked();
    void on_functionrowsbackpushButton_clicked();
    void on_functionallapplypushButton_clicked();

    void on_cvradioButton_clicked();
    void on_ccradioButton_clicked();
    void on_ovpradioButton_clicked();
    void on_functionunitpushButton_clicked();

    void on_socradioButton_clicked();
    void on_capradioButton_clicked();
    void on_functionmodelpushButton_clicked();

private:
    Ui::Mainwindow *ui;
    versionview* m_vercard;
    chstatusview* m_chstatuscard;
    numberkeypad* m_funcnmbkeycard;
    digitalcard* m_funcdigitalcard;
    batterycard* m_funcbatterycard;
    remoteoverlay *m_remoteOverlay;

    //QMap<quint8,bool> m_ChOutmode;
    QMap<quint8,QString> m_ChsoftVer;
    QMap<quint8,QString> m_ChhardVer;

    QStandardItemModel *m_digitalModel;
    QMap<quint8,digitalcard*> m_digitalcards;

    QStandardItemModel *m_batteryModel;
    QMap<quint8,batterycard*> m_batterycards;

    const int CARD_WIDTH = 211;
    const int CARD_HEIGHT = 316;

    quint8 m_functioncsetmode{0};
    quint8 m_functioncCh{0};
    quint8 m_initalpage{0};
};

#endif // MAINWINDOW_H
