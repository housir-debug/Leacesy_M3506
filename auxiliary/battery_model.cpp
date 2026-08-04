#include "battery_model.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <algorithm>

BatteryModel::BatteryModel(QObject *parent) : QObject(parent) {}

Q_LOGGING_CATEGORY(battery, "BATTERY:")

// *************************** BatteryModel ***********************************

float BatteryModel::interpolate(float soc,bool isocv) const {
    int left = 0; int right = data_points.size() - 1;

    // From top to bottom, from small to large
    while (right - left > 1) {
        int mid = (left + right) / 2;
        (data_points[mid].soc <= soc) ? (left = mid) : (right = mid);
    }

    const auto& p1 = data_points[left];
    float y1 = isocv ? p1.ocv : p1.imp;
    const auto& p2 = data_points[left + 1];
    float y2 = isocv ? p2.ocv : p2.imp;

    float ad = (y2 - y1) * (soc - p1.soc) / (p2.soc - p1.soc);
    return y1 + ad;
}

float BatteryModel::getOCV(float soc) const {
    if (!data_points.isEmpty()) {
        // From top to bottom, from small to large,Beyond the scope of the model
        if (soc <= data_points.first().soc){
            return data_points.first().ocv;
        }

        if (soc >= data_points.last().soc){
            return data_points.last().ocv;
        }

        return interpolate(soc,true);
    }

    return 0.0f;
}

float BatteryModel::getESR(float soc) const {
    if (!data_points.isEmpty()) {
        // From top to bottom, from small to large,Beyond the scope of the model
        if (soc <= data_points.first().soc){
            return data_points.first().imp;
        }

        if (soc >= data_points.last().soc){
            return data_points.last().imp;
        }

        return interpolate(soc,false);
    }

    return 0.0;
}

bool BatteryModel::isOver(float soc) const{
    if (!data_points.isEmpty() && soc >= data_points.first().soc && soc <= data_points.last().soc) {
        // From top to bottom, from small to large,Beyond the scope of the model
        return false; // 1-100 %
    }

    return true;
}

// ********************** BatteryModelManager *********************************

BatteryModelManager::BatteryModelManager(const QString &parentPath, QObject *parent):
    QObject(parent), m_modelDirectory(parentPath + "/battery_models"){}

QSharedPointer<BatteryModel> BatteryModelManager::getModel(const QString &modelName) const{return m_models.value(modelName);}

QStringList BatteryModelManager::getAvailableModels() const{return m_models.keys();}


bool BatteryModelManager::saveModel(QSharedPointer<BatteryModel> model,const QString &modelName) {
    QDir modelDir(m_modelDirectory);
    if (!modelDir.exists()) {modelDir.mkpath(".");}
    QString filePath = modelDir.filePath(modelName + ".csv");

    if(!QFile::exists(filePath)){ // check , not create
        std::sort(model->data_points.begin(), model->data_points.end(),[]
          (const BatteryDataPoint &a, const BatteryDataPoint &b){
              // Ascending (from smallest to largest)
              return a.soc < b.soc;
        });

        model->name = modelName;
        m_models[modelName] = model;

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "SOC_%,OCV_V,IMP_ohm\n";

            const auto &points = model->data_points;
            for (const auto &point : points) {
                stream << QString::number(point.soc, 'f', 3) << ","
                       << QString::number(point.ocv, 'f', 3) << ","
                       << QString::number(point.imp, 'f', 3) << "\n";
            }

            file.close();
            return true;
        }

        qCWarning(battery)<<"[saveModel]: not create model file: "<<filePath;
        return false;
    }

    qCWarning(battery)<<"[saveModel]: model file already exist!";
    return false;
}

bool BatteryModelManager::removeModel(const QString &modelName) {
    if (m_models.contains(modelName)) {
        QFile file(QDir(m_modelDirectory).filePath(modelName + ".csv"));
        if (file.exists() && file.remove()) {
            m_models.remove(modelName);
            return true;
        }

        qCWarning(battery)<<"[removeModel]: delete model: "<<modelName<<" | error:"<<file.errorString();
        return false;
    }

    return true;
}


bool BatteryModelManager::loadAllModels() {
    m_models.clear();

    QDir modelDir(m_modelDirectory);
    if (!modelDir.exists()) {modelDir.mkpath(".");}
    QFileInfoList fileList = modelDir.entryInfoList({"*.csv", "*.CSV"}, QDir::Files);

    if (!fileList.isEmpty()) {
        for (const auto &fileInfo : qAsConst(fileList)) {
            auto model = parseCSV(fileInfo.absoluteFilePath());
            if (!model.isNull()) {
                model->name = fileInfo.baseName();
                m_models[model->name] = model;
            }
        }

        return true;
    }

    qCWarning(battery)<<"[loadAllModels]:No model file exists.";
    return false;
}

QSharedPointer<BatteryModel> BatteryModelManager::parseCSV(const QString &filePath) {
    auto model = QSharedPointer<BatteryModel>::create();
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        QStringList columns = stream.readLine().trimmed().split(','); // first line

        int socCol = -1, ocvCol = -1, esrCol = -1;
        for (int i = 0; i < columns.size(); ++i) {
            QString normalized = columns[i].trimmed();
            if (normalized=="SOC_%")        {socCol = i;}
            else if (normalized=="OCV_V")   {ocvCol = i;}
            else if (normalized=="IMP_ohm") {esrCol = i;}
        }

        if (socCol != -1 && ocvCol != -1 && esrCol != -1){
            while (!stream.atEnd()) {
                columns = stream.readLine().trimmed().split(',');

                if (columns.size() == 3) {
                    BatteryDataPoint point;
                    point.soc = columns[socCol].toFloat();
                    if (0.0f <= point.soc && point.soc <= 100.0f) {
                        point.ocv = columns[ocvCol].toFloat();
                        if (0.0f <= point.ocv && point.ocv <= 6.0f) {
                            point.imp = columns[esrCol].toFloat();
                            if (0.0f <= point.imp && point.imp <= 1.0f) {
                                model->data_points.append(point);
                                continue; // access next line
                            }
                        }
                    }
                }

                qCWarning(battery)<<"[parseCSV]: line failed: "<< columns;
                return nullptr;
            }

            // be sorted when savemodel
            return model;
        }

        qCWarning(battery)<<"[parseCSV]: format failed!" << filePath;
        return nullptr;
    }

    qCWarning(battery)<<"[parseCSV]:failed open file!" << filePath;
    return nullptr;
}
