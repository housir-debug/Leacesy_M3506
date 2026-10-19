#pragma once
#include <QMap>
#include <QObject>

struct BatteryDataPoint
{
    float soc;
    float ocv;
    float imp;
};

class BatteryModelManager : public QObject
{
    Q_OBJECT

public:
    bool saveModel(QVector<BatteryDataPoint>& pts, const QString &modelName);
    bool removeModel(const QString &modelName);

    QStringList getAvailableModels() const;
    bool loadAllModels();

    QMap<QString, QVector<BatteryDataPoint>> m_models;
    static BatteryModelManager& instance();
    QString m_modelDirectory{""};

    bool isOver(QString model,float soc) const;
    float getOCV(QString model,float soc) const;
    float getESR(QString model,float soc) const;

private:
    float interpolate(const QVector<BatteryDataPoint>& pts,float soc, bool isocv)  const;

    BatteryModelManager(QObject *parent = nullptr);
    ~BatteryModelManager() = default;
    Q_DISABLE_COPY(BatteryModelManager)
};
