#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class DigitalCardWidget;
class BatteryHomePage;
class SettingPage;
class FunctionPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onUartBridgeChanged();
    void showDigitalHomePage();
    void showBatteryHomePage();
    void showSettingPage();
    void showFunctionPage(int channel);
    void onBackRequested();

private:
    void setupUI();
    void setupConnections();
    void createLockOverlay();

    QStackedWidget *m_stackedWidget;
    QWidget *m_digitalHomePage;
    QWidget *m_batteryHomePage;
    QWidget *m_settingPage;
    QWidget *m_functionPage;

    QWidget *m_lockOverlay;
    QLabel *m_logoLabel;
    QPushButton *m_unlockButton;

    int m_homePageModel{0};
    int m_functionChannel{1};
};

#endif // MAINWINDOW_H
