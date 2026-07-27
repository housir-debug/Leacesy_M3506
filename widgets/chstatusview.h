#pragma once
#include <QFrame>

namespace Ui {class chstatusview;}

class chstatusview : public QFrame
{
    Q_OBJECT

private:
    Ui::chstatusview *ui;

public:
    void setChtatus(const QString &status);

    explicit chstatusview(QWidget *parent = nullptr);
    ~chstatusview();
};

