#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLoggingCategory>
#include <QStandardItemModel>
#include "digitalcard.h"
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
    void load_BatteryModel();
    // QJsonArray getAllChannelsData();

    // Screen trigger function
    QString setChannel_CurrentUnit(int channel);
    void setChannel_Setstatus(int channel,int model,const QString& val);

    void setChannel_BatteryOutput(int channel,bool switchs);
    void setChannel_InitSOC(int channel,const QString& val);
    void setChannel_Capacity(int channel,const QString& val);
    void setChannel_Batterymode(int channel,bool staticmode);
    QString setChannel_BatteryModel(int channel);

    QStringList m_currentModelList;
    std::shared_ptr<BatteryModelManager> m_modelManager{nullptr};

public:
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow();

    void update_remotemodel(quint8 reface);
    void update_Configuration(int model,const QString& val);
    void to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param);

    /* channel slot function */
    void update_SoftVer(int ch,const QString &ver);
    void update_HardVer(int ch,const QString &ver);

    void update_Voltage(int ch,float voltage);
    void update_CurrentAndUnit(int ch,float current);
    void update_Status(int ch,quint16 status);

    void update_Cv(int ch,float cv);
    void update_Cc(int ch,float cc);
    void update_Ovp(int ch,float ovp);
    void update_IsOutput(int ch,bool status);
    void update_Imp(int ch,float imp);
    /* ********************* */

private:
    Ui::Mainwindow *ui;
    QStandardItemModel *m_model;
    QMap<quint8,digitalcard*> m_numbercards;

    const int CARD_WIDTH = 206;
    const int CARD_HEIGHT = 310;

    int m_currentRow = 0;
    int m_totalRows = 0;

    void scrollToRow(int row);
};

#endif // MAINWINDOW_H
