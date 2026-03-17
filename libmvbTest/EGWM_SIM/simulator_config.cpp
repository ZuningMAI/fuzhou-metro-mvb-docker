#include "simulator_config.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

SimulatorConfig::SimulatorConfig()
    : m_railwayLine("FZ602")
    , m_lineID(6)
    , m_trainNum(1)
    , m_trainInfo(6011)
    , m_trainLoad(216.0)
    , m_portName("COM1")
    , m_baudRate(115200)
    , m_runMode(SECTION_MODE)
    , m_dataProcessMode(INTERPOLATION_MODE)
    , m_stopTimeMin(30)
    , m_stopTimeMax(60)
    , m_enableFrameSplit(false)
    , m_runInfoPeriodMs(512)  // 默认512ms
    , m_enableRunInfoLogging(false)  // 默认不启用RunInfo记录
{
}

bool SimulatorConfig::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开配置文件:" << filePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析错误:" << parseError.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qWarning() << "配置文件格式错误：根元素不是对象";
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // 解析railway配置
    if (root.contains("railway")) {
        QJsonObject railway = root["railway"].toObject();
        m_railwayLine = railway["line"].toString("FZ602");
        m_lineID = railway["lineID"].toInt(6);
    }
    
    // 解析train配置
    if (root.contains("train")) {
        QJsonObject train = root["train"].toObject();
        m_trainNum = static_cast<quint16>(train["trainNum"].toInt(1));
        m_trainInfo = static_cast<quint32>(train["trainInfo"].toInt(6011));
        m_trainLoad = train["trainLoad"].toDouble(216.0);
    }
    
    // 解析serial配置
    if (root.contains("serial")) {
        QJsonObject serial = root["serial"].toObject();
        m_portName = serial["portName"].toString("COM1");
        m_baudRate = serial["baudRate"].toInt(115200);
    }
    
    // 解析simulation配置
    if (root.contains("simulation")) {
        QJsonObject simulation = root["simulation"].toObject();
        
        QString runModeStr = simulation["runMode"].toString("SECTION");
        m_runMode = (runModeStr == "LINE") ? LINE_MODE : SECTION_MODE;
        
        QString dataProcessModeStr = simulation["dataProcessMode"].toString("INTERPOLATION");
        m_dataProcessMode = (dataProcessModeStr == "MAINTAIN") ? MAINTAIN_MODE : INTERPOLATION_MODE;
        
        m_csvFile = simulation["csvFile"].toString();
        m_stopTimeMin = simulation["stopTimeMin"].toInt(30);
        m_stopTimeMax = simulation["stopTimeMax"].toInt(60);
        m_enableFrameSplit = simulation["enableFrameSplit"].toBool(false);
        
        // 读取状态数据周期，默认512ms，支持 128/256/512
        m_runInfoPeriodMs = simulation["runInfoPeriodMs"].toInt(512);
        // 验证周期值是否合法
        if (m_runInfoPeriodMs != 128 && m_runInfoPeriodMs != 256 && m_runInfoPeriodMs != 512) {
            qWarning() << "状态数据周期值无效:" << m_runInfoPeriodMs << ", 使用默认值512ms";
            m_runInfoPeriodMs = 512;
        }
        
        // 读取RunInfo数据记录开关
        m_enableRunInfoLogging = simulation["enableRunInfoLogging"].toBool(false);
    }
    
    qDebug() << "配置文件加载成功:" << filePath;
    return true;
}

bool SimulatorConfig::saveToFile(const QString &filePath) const
{
    QJsonObject root;
    
    // railway配置
    QJsonObject railway;
    railway["line"] = m_railwayLine;
    railway["lineID"] = m_lineID;
    root["railway"] = railway;
    
    // train配置
    QJsonObject train;
    train["trainNum"] = static_cast<int>(m_trainNum);
    train["trainInfo"] = static_cast<qint64>(m_trainInfo);
    train["trainLoad"] = m_trainLoad;
    root["train"] = train;
    
    // serial配置
    QJsonObject serial;
    serial["portName"] = m_portName;
    serial["baudRate"] = m_baudRate;
    root["serial"] = serial;
    
    // simulation配置
    QJsonObject simulation;
    simulation["runMode"] = (m_runMode == LINE_MODE) ? "LINE" : "SECTION";
    simulation["dataProcessMode"] = (m_dataProcessMode == MAINTAIN_MODE) ? "MAINTAIN" : "INTERPOLATION";
    simulation["csvFile"] = m_csvFile;
    simulation["stopTimeMin"] = m_stopTimeMin;
    simulation["stopTimeMax"] = m_stopTimeMax;
    simulation["enableFrameSplit"] = m_enableFrameSplit;
    simulation["runInfoPeriodMs"] = m_runInfoPeriodMs;
    simulation["enableRunInfoLogging"] = m_enableRunInfoLogging;
    root["simulation"] = simulation;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法创建配置文件:" << filePath;
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "配置文件保存成功:" << filePath;
    return true;
}

