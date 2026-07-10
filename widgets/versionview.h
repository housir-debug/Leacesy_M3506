#pragma once
#include <QFrame>

namespace Ui {
    class versionview;
}

class versionview : public QFrame
{
    Q_OBJECT

public:
    explicit versionview(QWidget *parent = nullptr);
    ~versionview();

    void setChannelName(const QString &name);
    void setChSWVersion(const QString &ver);
    void setChHWVersion(const QString &ver);

private:
    Ui::versionview *ui;
};
