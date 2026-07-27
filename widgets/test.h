#pragma once
#include <QWidget>

namespace Ui {class test;}

class test : public QWidget
{
    Q_OBJECT

private:
    Ui::test *ui;

public:
    explicit test(QWidget *parent = nullptr);
    ~test();
};

