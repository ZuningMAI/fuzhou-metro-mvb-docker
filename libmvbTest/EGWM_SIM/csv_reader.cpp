#include "csv_reader.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bool CSVReader::readCSVFile(const QString &filePath, SectionInfo &sectionInfo)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开CSV文件:" << filePath;
        return false;
    }
    
    QTextStream in(&file);
    // in.setCodec("UTF-8");  // 设置编码为UTF-8
    
    // 读取第一行头信息
    if (!in.atEnd()) {
        QString headerLine = in.readLine();
        if (!parseHeaderLine(headerLine, sectionInfo)) {
            qWarning() << "解析头信息失败:" << headerLine;
            file.close();
            return false;
        }
    }
    
    // 读取第二行列标题（跳过）
    if (!in.atEnd()) {
        in.readLine();
    }
    
    // 读取数据行
    sectionInfo.trajectory.clear();
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        
        TrajectoryPoint point;
        if (parseDataLine(line, point)) {
            sectionInfo.trajectory.append(point);
        }
    }
    
    file.close();
    
    // 根据轨迹数据设置起始和结束位置
    if (!sectionInfo.trajectory.isEmpty()) {
        sectionInfo.startPosition = sectionInfo.trajectory.first().position;
        sectionInfo.endPosition = sectionInfo.trajectory.last().position;
    }
    
    sectionInfo.csvFilePath = filePath;
    
    qDebug() << "成功读取CSV文件:" << filePath;
    qDebug() << "  区间:" << sectionInfo.startStation << "->" << sectionInfo.endStation;
    qDebug() << "  位置:" << sectionInfo.startPosition << "~" << sectionInfo.endPosition;
    qDebug() << "  数据点数:" << sectionInfo.trajectory.size();
    
    return true;
}

bool CSVReader::readLineData(const QString &railwayLine, QVector<SectionInfo> &sections)
{
    sections.clear();
    
    int stationCount = getLineStationCount(railwayLine);
    if (stationCount <= 0) {
        qWarning() << "未知的线路:" << railwayLine;
        return false;
    }
    
    // 读取每个区间
    for (int i = 1; i < stationCount; ++i) {
        QString csvFile = findSectionCSV(railwayLine, i, i + 1);
        if (csvFile.isEmpty()) {
            qWarning() << "未找到区间" << i << "->" << (i + 1) << "的CSV文件";
            continue;
        }
        
        SectionInfo section;
        if (readCSVFile(csvFile, section)) {
            sections.append(section);
        }
    }
    
    qDebug() << "成功读取线路" << railwayLine << "的" << sections.size() << "个区间";
    return !sections.isEmpty();
}

bool CSVReader::parseFileName(const QString &fileName, QString &line, int &startStn, int &endStn, int &time)
{
    // 匹配模式: OptReslog.2025-10-26_18-00-30-FZ602-1-2-85.csv
    // 或者: OptReslog.2025-10-26_18-00-30-FZ601-13-14-125.csv
    QRegularExpression re(R"((FZ\d{3})-(\d+)-(\d+)-(\d+))");
    QRegularExpressionMatch match = re.match(fileName);
    
    if (!match.hasMatch()) {
        return false;
    }
    
    line = match.captured(1);           // FZ602 或 FZ601
    startStn = match.captured(2).toInt(); // 1
    endStn = match.captured(3).toInt();   // 2
    time = match.captured(4).toInt();     // 85
    
    return true;
}

bool CSVReader::parseHeaderLine(const QString &line, SectionInfo &info)
{
    // 头信息格式: 线路：FZ602,优化区间：1->2,设定时间：85,实际时间：84.998,...
    
    // 解析线路
    QRegularExpression reRailway(QString::fromUtf8("线路[：:]\\s*([A-Z0-9]+)"));
    QRegularExpressionMatch matchRailway = reRailway.match(line);
    if (matchRailway.hasMatch()) {
        info.railwayLine = matchRailway.captured(1);
    }
    
    // 解析区间
    QRegularExpression reSection(QString::fromUtf8("优化区间[：:]\\s*(\\d+)\\s*->\\s*(\\d+)"));
    QRegularExpressionMatch matchSection = reSection.match(line);
    if (matchSection.hasMatch()) {
        info.startStation = matchSection.captured(1).toInt();
        info.endStation = matchSection.captured(2).toInt();
    }
    
    // 解析设定时间
    QRegularExpression reTime(QString::fromUtf8("设定时间[：:]\\s*([\\d.]+)"));
    QRegularExpressionMatch matchTime = reTime.match(line);
    if (matchTime.hasMatch()) {
        info.plannedTime = matchTime.captured(1).toDouble();
    }
    
    return !info.railwayLine.isEmpty();
}

bool CSVReader::parseDataLine(const QString &line, TrajectoryPoint &point)
{
    QStringList parts = line.split(',');
    if (parts.size() < 5) {
        return false;
    }
    
    bool ok;
    point.relativeTime = parts[0].toDouble(&ok);
    if (!ok) return false;
    
    point.position = parts[1].toDouble(&ok);
    if (!ok) return false;
    
    point.speed = parts[2].toDouble(&ok);
    if (!ok) return false;
    
    point.force = parts[3].toDouble(&ok);
    if (!ok) return false;
    
    point.operationMode = parts[4].toInt(&ok);
    if (!ok) return false;
    
    return true;
}

QString CSVReader::findSectionCSV(const QString &railwayLine, int startStn, int endStn)
{
    // 从JSON配置文件加载计划运行时间
    QVector<int> plannedTimes = loadPlannedTimesFromJson(railwayLine);
    
    // 构建目录路径
    QString baseDir = QString("data/%1").arg(railwayLine);
    
    QDir dir(baseDir);
    if (!dir.exists()) {
        qWarning() << "数据目录不存在:" << baseDir;
        return QString();
    }
    
    // 获取该区间的计划运行时间
    int sectionIndex = startStn - 1;  // 区间索引从0开始
    int targetTime = -1;
    
    if (sectionIndex >= 0 && sectionIndex < plannedTimes.size()) {
        targetTime = plannedTimes[sectionIndex];
    }
    
    if (targetTime < 0) {
        qWarning() << "无法获取区间" << startStn << "->" << endStn << "的计划运行时间";
        return QString();
    }
    
    // 遍历子目录
    QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QString &subDir : subDirs) {
        // 检查目录名是否包含区间信息
        QString pattern = QString("%1-%2-%3").arg(railwayLine).arg(startStn).arg(endStn);
        if (!subDir.contains(pattern)) {
            continue;
        }
        
        // 查找该目录下的CSV文件
        QDir sectionDir(baseDir + "/" + subDir);
        QStringList csvFiles = sectionDir.entryList(QStringList() << "OptReslog*.csv", QDir::Files);
        
        // 查找匹配计划运行时间的CSV文件
        for (const QString &csvFile : csvFiles) {
            QString line;
            int fileStn1, fileStn2, fileTime;
            if (parseFileName(csvFile, line, fileStn1, fileStn2, fileTime)) {
                // 检查线路、区间和时间是否匹配
                if (line == railwayLine && 
                    fileStn1 == startStn && 
                    fileStn2 == endStn && 
                    fileTime == targetTime) {
                    QString fullPath = sectionDir.absoluteFilePath(csvFile);
                    qDebug() << "找到匹配的CSV文件:" << fullPath << "计划时间:" << targetTime;
                    return fullPath;
                }
            }
        }
        
        // 如果没有找到精确匹配的时间，返回第一个匹配区间的文件
        if (!csvFiles.isEmpty()) {
            qWarning() << "未找到计划时间为" << targetTime << "的CSV文件，使用第一个匹配文件";
            return sectionDir.absoluteFilePath(csvFiles.first());
        }
    }
    
    return QString();
}

int CSVReader::getLineStationCount(const QString &railwayLine)
{
    // FZ601 和 FZ602 都有19个站（18个区间）
    if (railwayLine == "FZ601" || railwayLine == "FZ602") {
        return 19;
    }
    return 0;
}


QVector<int> CSVReader::loadPlannedTimesFromJson(const QString &railwayLine)
{
    // 默认的计划运行时间（18个区间）
    static const QVector<int> FZ602_DEFAULT = {86, 145, 165, 92, 455, 130, 110, 87, 120, 131, 95, 244, 173, 100, 100, 100, 100, 120};
    static const QVector<int> FZ601_DEFAULT = {120, 100, 100, 100, 100, 173, 244, 95, 131, 120, 87, 110, 130, 455, 92, 165, 145, 86};
    
    // 尝试从JSON文件读取配置
    QString configPath = "railway_config.json";
    QFile configFile(configPath);
    
    if (!configFile.exists()) {
        qWarning() << "配置文件不存在:" << configPath << "，使用默认值";
        return (railwayLine == "FZ602") ? FZ602_DEFAULT : FZ601_DEFAULT;
    }
    
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开配置文件:" << configPath << "，使用默认值";
        return (railwayLine == "FZ602") ? FZ602_DEFAULT : FZ601_DEFAULT;
    }
    
    // 读取并解析JSON
    QByteArray jsonData = configFile.readAll();
    configFile.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析错误:" << parseError.errorString() << "，使用默认值";
        return (railwayLine == "FZ602") ? FZ602_DEFAULT : FZ601_DEFAULT;
    }
    
    if (!doc.isObject()) {
        qWarning() << "JSON格式错误：根节点不是对象，使用默认值";
        return (railwayLine == "FZ602") ? FZ602_DEFAULT : FZ601_DEFAULT;
    }
    
    QJsonObject root = doc.object();
    
    if (!root.contains(railwayLine)) {
        qWarning() << "配置文件中未找到线路" << railwayLine << "的配置，使用默认值";
        return (railwayLine == "FZ602") ? FZ602_DEFAULT : FZ601_DEFAULT;
    }
    
    QJsonObject lineConfig = root[railwayLine].toObject();
    
    if (!lineConfig.contains("plannedTimes")) {
        qWarning() << "配置文件中未找到plannedTimes字段，使用默认值";
        return (railwayLine == "FZ602") ? FZ602_DEFAULT : FZ601_DEFAULT;
    }
    
    QJsonArray timesArray = lineConfig["plannedTimes"].toArray();
    
    if (timesArray.size() != 18) {
        qWarning() << "计划运行时间数组长度不匹配（期望18个，实际" << timesArray.size() << "个），使用默认值";
        return (railwayLine == "FZ602") ? FZ602_DEFAULT : FZ601_DEFAULT;
    }
    
    // 转换为QVector<int>
    QVector<int> plannedTimes;
    for (int i = 0; i < timesArray.size(); ++i) {
        if (timesArray[i].isDouble()) {
            plannedTimes.append(timesArray[i].toInt());
        } else {
            qWarning() << "计划运行时间数组中第" << i << "个元素不是数字，使用默认值";
            return (railwayLine == "FZ602") ? FZ602_DEFAULT : FZ601_DEFAULT;
        }
    }
    
    qDebug() << "成功从配置文件加载线路" << railwayLine << "的计划运行时间";
    return plannedTimes;
}
