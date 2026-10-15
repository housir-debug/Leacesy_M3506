#include "global_static.h"
#include "battery_model.h"
#include <QDir>

bool BatteryModelManager::saveModel(QVector<BatteryDataPoint>& pts,const QString &modelName)
{
    QDir modelDir(m_modelDirectory);
    if (!modelDir.exists()) {modelDir.mkpath(".");}
    QString filePath = modelDir.filePath(modelName + ".csv");

    if(!QFile::exists(filePath)){ // check , not create
        std::sort(pts.begin(), pts.end(),[]
            (const BatteryDataPoint &a, const BatteryDataPoint &b){
              // Ascending (from smallest to largest)
              return a.soc < b.soc;
        });

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) { // create
            QTextStream stream(&file);
            stream << "SOC_%,OCV_V,IMP_Ohm\n";

            for (const auto &point : pts) {
                stream << QString::number(point.soc, 'f', 3) << ","
                       << QString::number(point.ocv, 'f', 3) << ","
                       << QString::number(point.imp, 'f', 3) << "\n";
            }

            m_models.insert(modelName, std::move(pts));
            file.close();
            return true;
        }

        qCWarning(battMod)<<"[saveModel]: not create model file: "<<filePath;
        return false;
    }

    qCWarning(battMod)<<"[saveModel]: model file already exist!";
    return false;
}
bool BatteryModelManager::removeModel(const QString &modelName)
{
    if (m_models.contains(modelName)) {
        const QDir dir(m_modelDirectory);
        const QStringList candidates = {
            dir.filePath(modelName + ".csv"),
            dir.filePath(modelName + ".CSV")
        };

        for (const QString& path : candidates) {
            QFile file(path);
            if (file.exists() && file.remove()) {
                m_models.remove(modelName);
                return true;
            }

            qCWarning(battMod)<<"[removeModel]: delete model: "<<modelName<<" | error:"<<file.errorString();
        }
    }

    return false;
}

QStringList BatteryModelManager::getAvailableModels() const{return m_models.keys();}
bool BatteryModelManager::loadAllModels()
{
    m_models.clear();
    QDir modelDir(m_modelDirectory);
    if (!modelDir.exists()) {modelDir.mkpath(".");}

    QFileInfoList fileList = modelDir.entryInfoList({"*.csv", "*.CSV"}, QDir::Files);
    if (!fileList.isEmpty()) {
        for (const auto &fileInfo : qAsConst(fileList)) {
            QFile file(fileInfo.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                QStringList columns = stream.readLine().trimmed().split(','); // first line

                int socCol = -1, ocvCol = -1, esrCol = -1;
                for (int i = 0; i < columns.size(); ++i) {
                    QString normalized = columns[i].trimmed();
                    if (normalized=="SOC_%")        {socCol = i;}
                    else if (normalized=="OCV_V")   {ocvCol = i;}
                    else if (normalized=="IMP_Ohm") {esrCol = i;}
                }

                if (socCol != -1 && ocvCol != -1 && esrCol != -1){
                    QVector<BatteryDataPoint> pts;
                    bool readOver = true;

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
                                        pts.append(point);
                                        continue; // access next line
                                    }
                                }
                            }
                        }

                        qCWarning(battMod)<<"[parseCSV]: line failed: "<< columns;
                        readOver = false;
                        break;
                    }

                    // be sorted when savemodel
                    if (readOver){
                        m_models.insert(fileInfo.baseName(), std::move(pts));
                        continue;
                    }
                }

                qCWarning(battMod)<<"[parseCSV]: format failed!" << fileInfo.absoluteFilePath();
                continue;
            }

            qCWarning(battMod)<<"[parseCSV]:failed open file!" << fileInfo.absoluteFilePath();
        }

        return true;
    }

    qCWarning(battMod)<<"[loadAllModels]:No model file exists.";
    return false;
}

BatteryModelManager& BatteryModelManager::instance()
{
    static BatteryModelManager inst;
    return inst;
}

bool BatteryModelManager::isOver(QString model,float soc) const
{
    auto it = m_models.constFind(model);
    if (it != m_models.constEnd() && !it.value().isEmpty()) {
        const QVector<BatteryDataPoint>& pts = it.value();
        if (soc >= pts.first().soc && soc <= pts.last().soc) {
            return false; // 1-100 %
        }
    }

    return true;
}
float BatteryModelManager::getOCV(QString model,float soc) const
{
    auto it = m_models.constFind(model);
    if (it != m_models.constEnd() && !it.value().isEmpty()) {
        const QVector<BatteryDataPoint>& pts = it.value();
        // From top to bottom, from small to large,Beyond the scope of the model
        if (soc <= pts.first().soc){
            return pts.first().ocv;
        }else if (soc >= pts.last().soc){
            return pts.last().ocv;
        }

        return interpolate(pts,soc,true);
    }

    return 0.0f;
}
float BatteryModelManager::getESR(QString model,float soc) const
{
    auto it = m_models.constFind(model);
    if (it != m_models.constEnd() && !it.value().isEmpty()) {
        const QVector<BatteryDataPoint>& pts = it.value();
        // From top to bottom, from small to large,Beyond the scope of the model
        if (soc <= pts.first().soc){
            return pts.first().imp;
        }else if (soc >= pts.last().soc){
            return pts.last().imp;
        }

        return interpolate(pts,soc,false);
    }

    return 0.0f;
}
float BatteryModelManager::interpolate(const QVector<BatteryDataPoint>& pts,float soc,bool isocv) const
{
    int left = 0; int right = pts.size() - 1;

    // From top to bottom, from small to large
    while (right - left > 1) {
        int mid = (left + right) / 2;
        (pts[mid].soc <= soc) ? (left = mid) : (right = mid);
    }

    const auto& p1 = pts[left];
    float y1 = isocv ? p1.ocv : p1.imp;
    const auto& p2 = pts[left + 1];
    float y2 = isocv ? p2.ocv : p2.imp;

    return y1 + (y2 - y1)*(soc - p1.soc)/(p2.soc - p1.soc);
}

BatteryModelManager::BatteryModelManager(QObject* parent): QObject(parent)
{}
