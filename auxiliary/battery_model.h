#pragma once
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(battery)

struct BatteryDataPoint {
    float soc;      // State of Charge (0-100%)
    float ocv;      // Open Circuit Voltage (V)
    float imp;      // Equivalent Series Resistance (Ω)
};

class BatteryModel : public QObject
{
    Q_OBJECT

public:
    explicit BatteryModel(QObject *parent = nullptr);

    QVector<BatteryDataPoint> data_points;
    QString name;

    bool isOver(float soc) const;
    float getOCV(float soc) const;
    float getESR(float soc) const;

private:
    float interpolate(float soc,bool isocv) const;
};

class BatteryModelManager : public QObject
{
    Q_OBJECT

public:
    BatteryModelManager(const QString& parentPath, QObject *parent = nullptr);
    ~BatteryModelManager() override = default;

    bool loadAllModels();
    bool removeModel(const QString &modelName);
    bool saveModel(QSharedPointer<BatteryModel> model, const QString &modelName);

    QStringList getAvailableModels() const;
    QSharedPointer<BatteryModel> getModel(const QString &modelName) const;

    QMap<QString, QSharedPointer<BatteryModel>> m_models;

private:
    QSharedPointer<BatteryModel> parseCSV(const QString &filePath);
    QString m_modelDirectory;
};
