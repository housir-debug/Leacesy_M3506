// DigitalCardWidget.cpp
#include "digitalcardwidget.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>

// 实例初始化

DigitalCardWidget::DigitalCardWidget(QWidget *parent): QWidget(parent)
{
    setupUI();

    m_pressAnimation = new QPropertyAnimation(this, "scale");
    m_pressAnimation->setDuration(110);
    m_pressAnimation->setStartValue(1.0);
    m_pressAnimation->setEndValue(0.96);

    m_releaseAnimation = new QPropertyAnimation(this, "scale");
    m_releaseAnimation->setDuration(110);
    m_releaseAnimation->setStartValue(0.96);
    m_releaseAnimation->setEndValue(1.0);
    m_releaseAnimation->setEasingCurve(QEasingCurve::OutQuad);

    // Longpress timer
    m_longPressTimer = new QTimer(this);
    m_longPressTimer->setSingleShot(true); //
    m_longPressTimer->setInterval(600);
    connect(m_longPressTimer, &QTimer::timeout, this,[this]() {
        if (!isEnabled()) {return;}

        m_longPressed = true;
        emit pressAndHold();
    });
}

DigitalCardWidget::~DigitalCardWidget()
{
}

// 绘制 UI

void DigitalCardWidget::setupUI()
{
    setFixedSize(280, 400);
    setStyleSheet("background: transparent;");

    m_cardWidget = new QWidget(this);
    m_cardWidget->setGeometry(0, 0, width(), height());
    m_cardWidget->setStyleSheet("opacity: 0.9;");

    // 执行 setupUI 绘制 UI 之后，若后续不再调用，则不需要持久化 - 成员变量
    QVBoxLayout *mainLayout = new QVBoxLayout(m_cardWidget);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(18);

    // ************ header ************
    m_channelLabel = new QLabel("CH_1");
    m_channelLabel->setStyleSheet("color: #E0E0E0; font-weight: bold; font-size: 36px;");

    m_dotLabel = new QLabel;
    m_dotLabel->setFixedSize(36, 36);
    m_dotLabel->setStyleSheet(
        "background-color: #6A6A7E;"
        "border: 1.2px solid #363636;"
        "border-radius: 18px;"
    );

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(m_channelLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_dotLabel);

    mainLayout->addLayout(topLayout);
    // ------------------------------

    // ************ measure ************
    QWidget *measureCard = new QWidget;
    measureCard->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #181824, stop:1 #090918);"
        "border: 1.2px solid #3A3A4E;"
        "border-radius: 18px;"
    );
    measureCard->setFixedHeight(160);

    m_voltageLabel = new QLabel("0.0000 V");
    m_voltageLabel->setStyleSheet("color: #0DAF65; font-weight: bold; font-size: 36px;");
    m_voltageLabel->setAlignment(Qt::AlignCenter);

    m_currentLabel = new QLabel("0.0000");
    m_currentLabel->setStyleSheet("color: #DF1D32; font-weight: bold; font-size: 36px;");
    m_currentUnitLabel = new QLabel("A");
    m_currentUnitLabel->setStyleSheet("color: #DF1D32; font-weight: bold; font-size: 36px;");
    m_voltageLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *currentLayout = new QHBoxLayout;
    currentLayout->setAlignment(Qt::AlignCenter);
    currentLayout->addWidget(m_currentUnitLabel);
    currentLayout->addWidget(m_currentLabel);
    currentLayout->setSpacing(6);

    QVBoxLayout *measureLayout = new QVBoxLayout(measureCard);
    measureLayout->setAlignment(Qt::AlignCenter);
    measureLayout->addWidget(m_voltageLabel);
    measureLayout->addLayout(currentLayout);
    measureLayout->setSpacing(12);

    mainLayout->addWidget(measureCard);
    // ------------------------------

    // ************ bottom ************
    QVBoxLayout *setpointsLayout = new QVBoxLayout;
    setpointsLayout->setSpacing(6);

    QHBoxLayout *cvLayout = createSetpointRow("CV", "#1DBF75", &m_cvValueLabel, &m_cvIndicator);
    setpointsLayout->addLayout(cvLayout);

    QHBoxLayout *ccLayout = createSetpointRow("CC", "#FF3D52", &m_ccValueLabel, &m_ccIndicator);
    setpointsLayout->addLayout(ccLayout);

    QHBoxLayout *ovpLayout = createSetpointRow("OVP", "#E5C27D", &m_ovpValueLabel, &m_ovpIndicator);
    setpointsLayout->addLayout(ovpLayout);

    mainLayout->addLayout(setpointsLayout);
    mainLayout->addStretch();   // 插入弹簧
    // ------------------------------

    updateStyles();
}

void DigitalCardWidget::updateStyles()
{
    QString dotColor = m_channelOutput ? "#1AF080" : "#6A6A7E";
    m_dotLabel->setStyleSheet(QString(
        "background-color: %1;"
        "border: 1.2px solid #363636;"
        "border-radius: 18px;"
    ).arg(dotColor));

    // 触发当前 widget 重绘
    update();   // repaint();
}

QHBoxLayout* DigitalCardWidget::createSetpointRow(const QString &label, const QString &color,
                                                   QLabel **valueLabel, QWidget **indicator)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(6, 0, 6, 0);

    QLabel *nameLabel = new QLabel(label);
    nameLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 18px;").arg(color));
    layout->addWidget(nameLabel);
    layout->addStretch();

    *valueLabel = (label == "CC") ? new QLabel("0.000 A") : new QLabel("0.000 V");
    (*valueLabel)->setStyleSheet("color: #A0A0B0; font-size: 18px;");
    layout->addWidget(*valueLabel);

    *indicator = new QWidget;
    (*indicator)->setFixedSize(22, 22);
    (*indicator)->setStyleSheet(
        "border: 1.8px solid " + color + ";"
        "border-radius: 11px;"
        "background: transparent;"
    );
    layout->addWidget(*indicator);

    return layout;
}

// 接收事件

void DigitalCardWidget::paintEvent(QPaintEvent *event)
{
    // 绘制时自动调用，paint - 绘制背景 Qt会自动绘制子控件
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿

    QRectF cardRect(0, 0, width(), height());
    QPainterPath path;
    path.addRoundedRect(cardRect, 36, 36); // 创建圆角矩形路径

    // background
    QColor bgColor = m_channelOutput ? QColor(QString("#363648")) : QColor(QString("#181824"));
    painter.fillPath(path, bgColor);

    // border
    QPen pen;
    pen.setColor(m_channelOutput ? QColor(QString("#4A6FA5")) : QColor(QString("#2B2B3C")));
    pen.setWidth(2); // int
    painter.setPen(pen);
    painter.drawPath(path);

    // 上下装饰线
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_channelOutput ? QColor(QString("#6B8ABC")) : QColor(QString("#404050")));
    // {x,y,width,heigth,radius,radius}
    painter.drawRoundedRect(width() * 0.15, 8, width() * 0.7, static_cast<qreal>(0.9), 0.45, 0.45);
    painter.drawRoundedRect(width() * 0.15, height() - 8, width() * 0.7, static_cast<qreal>(0.9), 0.45, 0.45);
}

void DigitalCardWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (!isEnabled()) {return;}

    if (m_pressed){
        m_pressed = false;
        m_longPressTimer->stop();
        m_releaseAnimation->start();

        if (!m_longPressed) {
            emit clicked();
        }
    }
}

void DigitalCardWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (!isEnabled()) return;

    m_pressed = true;
    m_longPressed = false;

    m_pressPos = event->pos();
    m_longPressTimer->start();
    m_pressAnimation->start();
}

void DigitalCardWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (!isEnabled()) return;

    if (m_pressed) {
        int dx = event->pos().x() - m_pressPos.x();
        int dy = event->pos().y() - m_pressPos.y();

        if (dx*dx + dy*dy > 100) {
            m_longPressTimer->stop();
        }
    }
}

// external settings

void DigitalCardWidget::setScale(qreal scale)
{
    m_scale = scale;
    m_cardWidget->setGeometry(
        (width() - width() * scale) / 2,
        (height() - height() * scale) / 2,
        width() * scale,
        height() * scale
    );

    // update(); scale change auto to update()
}

void DigitalCardWidget::setEnabled(bool enabled)
{
    QWidget::setEnabled(enabled);

    if (!enabled) {
        m_pressAnimation->stop();
        m_releaseAnimation->stop();
        m_longPressTimer->stop();
    }
}

void DigitalCardWidget::setChannelOutput(bool output)
{
    m_channelOutput = output;
    updateStyles();
}

void DigitalCardWidget::setChannelName(const QString &name)
{
    m_channelLabel->setText(name);
}

void DigitalCardWidget::setVoltage(double voltage, const QString &unit)
{
    m_voltageLabel->setText(QString("%1 %2").arg(voltage, 0, 'f', 4).arg(unit));
}

void DigitalCardWidget::setCurrent(double current, const QString &unit)
{
    m_currentLabel->setText(QString::number(current, 'f', 4));
    m_currentUnitLabel->setText(unit);

    if (unit == "mA") {
        m_currentUnitLabel->setStyleSheet("color: #DF1D32; font-weight: bold; font-size: 18px;");
    } else {
        m_currentUnitLabel->setStyleSheet("color: #DF1D32; font-weight: bold; font-size: 36px;");
    }
}

void DigitalCardWidget::setCVSetpoint(double value)
{
    m_cvValueLabel->setText(QString("%1 V").arg(value, 0, 'f', 3));
}

void DigitalCardWidget::setCVMode(bool enabled)
{
    m_cvIndicator->setStyleSheet(QString(
        "border: 1.8px solid #1DBF75;"
        "border-radius: 11px;"
        "background: %1;"
    ).arg(enabled ? "#1DBF75" : "transparent"));
}

void DigitalCardWidget::setCCSetpoint(double value)
{
    m_ccValueLabel->setText(QString("%1 A").arg(value, 0, 'f', 3));
}

void DigitalCardWidget::setCCMode(bool enabled)
{
    m_ccIndicator->setStyleSheet(QString(
        "border: 1.8px solid #FF3D52;"
        "border-radius: 11px;"
        "background: %1;"
    ).arg(enabled ? "#FF3D52" : "transparent"));
}

void DigitalCardWidget::setOVPSetpoint(double value)
{
    m_ovpValueLabel->setText(QString("%1 V").arg(value, 0, 'f', 3));
}

void DigitalCardWidget::setOVPMode(bool enabled)
{
    m_ovpIndicator->setStyleSheet(QString(
        "border: 1.8px solid #E5C27D;"
        "border-radius: 11px;"
        "background: %1;"
    ).arg(enabled ? "#E5C27D" : "transparent"));
}
