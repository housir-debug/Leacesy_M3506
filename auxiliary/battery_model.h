#pragma once
#include <QLoggingCategory>

struct BatteryDataPoint {float soc;float ocv;float imp;};

Q_DECLARE_LOGGING_CATEGORY(battery)

class BatteryModel : public QObject
{
    Q_OBJECT

private:
    float interpolate(float soc, bool isocv)  const;

public:
    explicit BatteryModel(QObject *parent = nullptr);
    QVector<BatteryDataPoint> data_points;
    QString name;

    float getOCV(float soc) const;
    float getESR(float soc) const;
    bool isOver(float soc) const;
};

class BatteryModelManager : public QObject
{
    Q_OBJECT

public:
    BatteryModelManager(const QString& parentPath, QObject *parent = nullptr);
    ~BatteryModelManager() override = default;

    QSharedPointer<BatteryModel> getModel(const QString &modelName) const;
    QStringList getAvailableModels() const;

    QMap<QString, QSharedPointer<BatteryModel>> m_models;

    bool saveModel(QSharedPointer<BatteryModel> model, const QString &modelName);
    bool removeModel(const QString &modelName);
    bool loadAllModels();

private:
    QSharedPointer<BatteryModel> parseCSV(const QString &filePath);
    QString m_modelDirectory;
};
