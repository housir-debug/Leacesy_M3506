#pragma once
#include <QFrame>

namespace Ui {
    class chstatusview;
}

class chstatusview : public QFrame
{
    Q_OBJECT

public:
    explicit chstatusview(QWidget *parent = nullptr);
    ~chstatusview();

    void setChtatus(const QString &status);

private:
    Ui::chstatusview *ui;
};

