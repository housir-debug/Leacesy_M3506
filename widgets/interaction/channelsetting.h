#ifndef CHANNELSETTING_H
#define CHANNELSETTING_H

#include <QWidget>

namespace Ui {
class channelsetting;
}

class channelsetting : public QWidget
{
    Q_OBJECT

public:
    explicit channelsetting(QWidget *parent = nullptr);
    ~channelsetting();

private:
    Ui::channelsetting *ui;
};

#endif // CHANNELSETTING_H
