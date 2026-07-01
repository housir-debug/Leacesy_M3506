#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QStandardItemModel>
#include "digitalcard.h"

namespace Ui {
class Mainwindow;
}

class Mainwindow : public QWidget
{
    Q_OBJECT

public:
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow();

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

    digitalcard* findCardByChannelName(const QString &name) const;

    void addCardsFromList(const QList<QString> &channelNames);
private:
    Ui::Mainwindow *ui;
    QStandardItemModel *m_model;
    QList<digitalcard*> m_cards;
    QMap<quint8,QString> m_channel;
};

#endif // MAINWINDOW_H
