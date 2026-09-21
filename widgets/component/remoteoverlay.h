#pragma once
#include <QFrame>

namespace Ui {class remoteoverlay;}

class remoteoverlay : public QFrame
{
    Q_OBJECT

private:
    Ui::remoteoverlay *ui;

public:
    explicit remoteoverlay(QWidget *parent = nullptr);
    ~remoteoverlay();

signals:
    void exitRemote();

private slots:
    void on_gobackpushButton_clicked();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
};
