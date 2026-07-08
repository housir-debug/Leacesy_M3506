#ifndef NUMBERKEYPAD_H
#define NUMBERKEYPAD_H

#include <QFrame>

namespace Ui {
class numberkeypad;
}

class numberkeypad : public QFrame
{
    Q_OBJECT

public:
    explicit numberkeypad(QWidget *parent = nullptr);
    ~numberkeypad();

private:
    Ui::numberkeypad *ui;
};

#endif // NUMBERKEYPAD_H
