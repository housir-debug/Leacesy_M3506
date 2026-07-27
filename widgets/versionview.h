#pragma once
#include <QFrame>

namespace Ui {class versionview;}

class versionview : public QFrame
{
    Q_OBJECT

private:
    Ui::versionview *ui;

public:
    void setChannelName(int channel);
    void setChSWVersion(const QString &ver);
    void setChHWVersion(const QString &ver);

    explicit versionview(QWidget *parent = nullptr);
    ~versionview();
};
