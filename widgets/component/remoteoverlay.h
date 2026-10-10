#pragma once
#include <QFrame>

namespace Ui {class remoteoverlay;}

class remoteoverlay : public QFrame
{
    Q_OBJECT

private:
    Ui::remoteoverlay *ui;

signals:
    void exitRemote();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

public:
    explicit remoteoverlay(QWidget *parent = nullptr);
    ~remoteoverlay();

private slots:
    void on_gobackpushButton_clicked();
};
