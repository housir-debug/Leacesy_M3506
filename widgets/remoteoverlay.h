#pragma once
#include <QFrame>

namespace Ui {
    class remoteoverlay;
}

class remoteoverlay : public QFrame
{
    Q_OBJECT

public:
    explicit remoteoverlay(QWidget *parent = nullptr);
    ~remoteoverlay();

signals:
    void exitRemote();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void on_gobackpushButton_clicked();

private:
    Ui::remoteoverlay *ui;
};
