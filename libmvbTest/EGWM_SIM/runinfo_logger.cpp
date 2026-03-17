#include "runinfo_logger.h"
#include "logger.h"
#include <QDir>
#include <QDateTime>
#include <QDebug>

RunInfoLogger::RunInfoLogger(QObject *parent)
    : QObject(parent)
    , m_enabled(false)
    , m_firstRecordTime(0)
    , m_sectionActive(false)
{
}

RunInfoLogger::~RunInfoLogger()
{
    if (m_sectionActive) {
        endSection();
    }
}

void RunInfoLogger::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (enabled) {
        LOG_INFO("RunInfo数据记录器已启用");
    } else {
        LOG_INFO("RunInfo数据记录器已禁用");
    }
}

bool RunInfoLogger::startSection(const QString &sectionName, const QString &logDir)
{
    if (!m_enabled) {
        return false;
    }

    // 如果有活动的区间，先结束它
    if (m_sectionActive) {
        endSection();
    }

    m_currentSectionName = sectionName;
    m_logDir = logDir;
    m_records.clear();
    m_firstRecordTime = 0;
    
    // 创建日志目录
    QDir dir;
    if (!dir.exists(logDir)) {
        if (!dir.mkpath(logDir)) {
            qWarning() << "无法创建RunInfo日志目录:" << logDir;
            LOG_ERROR(QString("无法创建RunInfo日志目录: %1").arg(logDir));
            return false;
        }
    }

    // 生成CSV文件路径
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    m_csvFilePath = QString("%1/RunInfo_%2_%3.csv")
                        .arg(logDir)
                        .arg(sectionName)
                        .arg(timestamp);

    // 启动计时器
    m_timer.start();
    m_sectionActive = true;

    LOG_INFO(QString("开始记录RunInfo数据 - 区间: %1, 文件: %2").arg(sectionName).arg(m_csvFilePath));
    qDebug() << "RunInfo记录器已启动，区间:" << sectionName;

    return true;
}

void RunInfoLogger::logData(const RunInfoStruct &data, double position, int limitSpeed)
{
    if (!m_enabled || !m_sectionActive) {
        return;
    }

    // 获取当前时间戳
    qint64 currentTime = m_timer.elapsed();
    
    // 如果是第一条记录，设置基准时间
    if (m_firstRecordTime == 0) {
        m_firstRecordTime = currentTime;
    }

    // 计算相对时间（秒）
    double relativeTime = (currentTime - m_firstRecordTime) / 1000.0;

    // 创建记录
    RunInfoRecord record;
    record.relativeTime = relativeTime;
    record.lifeSignal = data.lifeSignal;
    record.endStationID = data.endStationID;
    record.nextStationID = data.nextStationID;
    record.currentStationID = data.currentStationID;
    record.targetDistance = data.targetDistance;
    record.startDistance = data.startDistance;
    record.trainLoad = data.trainLoad;
    record.limitSpeed = limitSpeed;
    record.netVoltage = data.netVoltage;
    record.netCurrent = data.netElectric;
    record.speed = data.Speed;
    record.tractionForce = data.tractionForce;
    record.ebrakeForce = data.ebrakeForce;
    record.airbrakeForce = data.airbrakeForce;
    record.endStationIdAvaliable = (data.flags>>15) & 0x01;
    record.nextStationIdAvaliable = (data.flags>>14) & 0x01;
    record.currentStationIdAvaliable = (data.flags>>13) & 0x01;
    record.targetDistanceAvaliable = (data.flags>>12) & 0x01;
    record.startDistanceAvaliable = (data.flags>>11) & 0x01;
    record.ATOMode = (data.flags>>10) & 0x01;
    record.TmcActivity_1 = (data.flags>>9) & 0x01;
    record.TmcActivity_2 = (data.flags>>8) & 0x01;
    record.coasting = (data.flags>>7) & 0x01;
    record.traction = (data.flags>>6) & 0x01;
    record.ebraking = (data.flags>>5) & 0x01;
    record.loadAW_0 = (data.flags>>4) & 0x01;
    record.loadAW_2 = (data.flags>>3) & 0x01;
    record.loadAW_3 = (data.flags>>2) & 0x01;
    record.position = position;
    
    
    

    // 添加到记录列表
    m_records.append(record);

    // 调试输出（可选，避免太多输出）
    if (m_records.size() % 10 == 0) {
        qDebug() << "RunInfo已记录" << m_records.size() << "条数据";
    }
}

void RunInfoLogger::endSection()
{
    if (!m_enabled || !m_sectionActive) {
        return;
    }

    // 保存到CSV文件
    if (saveToFile()) {
        LOG_INFO(QString("RunInfo数据已保存 - 区间: %1, 记录数: %2, 文件: %3")
                 .arg(m_currentSectionName)
                 .arg(m_records.size())
                 .arg(m_csvFilePath));
        qDebug() << "RunInfo数据已保存:" << m_csvFilePath << ", 共" << m_records.size() << "条";
    } else {
        LOG_ERROR(QString("RunInfo数据保存失败 - 区间: %1").arg(m_currentSectionName));
        qWarning() << "RunInfo数据保存失败";
    }

    // 清理数据
    clear();
}

void RunInfoLogger::clear()
{
    m_records.clear();
    m_firstRecordTime = 0;
    m_sectionActive = false;
    m_currentSectionName.clear();
    m_csvFilePath.clear();
}

bool RunInfoLogger::saveToFile()
{
    if (m_records.isEmpty()) {
        qDebug() << "没有RunInfo数据需要保存";
        return true;
    }

    QFile file(m_csvFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法创建RunInfo CSV文件:" << m_csvFilePath << ", 错误:" << file.errorString();
        return false;
    }

    // 写入UTF-8 BOM（字节顺序标记）
    // file.write("\xEF\xBB\xBF");
    QTextStream stream(&file);
    // stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(false);  // 我们已经手动写入BOM了

    // 写入CSV表头
    stream <<"time,lifeSignal,railwayID,endStationID,nextStationID,currentStationID,targetDistance,startDistance,trainLoad,limitSpeed,netElectric,netVoltage,Speed,tractionForce,ebrakeForce,airbrakeForce,endStationIDAvaliable,nextStationIDAvaliable,currentStationIDAvaliable,targetDistanceAval,startDistanceAvaliable,ATOMode,TmcActivity_1,TmcActivity_2,coasting,traction,ebraking,loadAW_0,loadAW_2,loadAW_3,pos\n";
    // stream << "时间(s),目标距离(m),起始距离(m),运行速度(km/h),位置(m),牵引力(kN),电制动力(kN),空气制动力(kN),网侧电压(V),网侧电流(A),限速(km/h)\n";

    // 写入数据
    for (const RunInfoRecord &record : m_records) {
        stream << formatCsvLine(record) << "\n";
    }

    stream.flush();
    file.close();
    
    qDebug() << "CSV文件已保存为UTF-8编码（带BOM）:" << m_csvFilePath;
    return true;
}

QString RunInfoLogger::formatCsvLine(const RunInfoRecord &record) const
{
    // 时间：保留3位小数
    QString time = QString::number(record.relativeTime, 'f', 3);
    // 生命信号
    QString lifeSignal = QString::number(record.lifeSignal);
    // 线路ID
    QString railwayID = QString::number(record.railwayID);
    // 终点站ID
    QString endStationID = QString::number(record.endStationID);
    // 下一站ID
    QString nextStationID = QString::number(record.nextStationID);
    // 当前站ID
    QString currentStationID = QString::number(record.currentStationID);

    // 目标距离、起始距离：原始值（m）
    QString targetDist = QString::number(record.targetDistance);
    QString startDist = QString::number(record.startDistance);
    
    // 列车载荷(1=0.1t)
    QString trainLoad = QString::number(record.trainLoad);
    // 限速（km/h）
    QString limitSpeed = QString::number(record.limitSpeed);
    // 网侧电压（V）
    QString netVoltage = QString::number(record.netVoltage);
    // 网侧电流（0.1A）
    QString netCurrent = QString::number(record.netCurrent * 0.1, 'f', 1);

    // 运行速度：原始单位是0.01km/h，转换为km/h（保留2位小数）
    QString speed = QString::number(record.speed * 0.01, 'f', 2);
    
    // 牵引力（0.1kN）
    QString tractionForce = QString::number(record.tractionForce * 0.1, 'f', 1);
    // 电制动力（0.1kN）
    QString ebrakeForce = QString::number(record.ebrakeForce * 0.1, 'f', 1);
    // 空气制动力（0.1kN）
    QString airbrakeForce = QString::number(record.airbrakeForce * 0.1, 'f', 1);
    // 终点站ID有效
    QString endStationIdAvaliable = QString::number(record.endStationIdAvaliable ? 1 : 0);
    // 下一站ID有效
    QString nextStationIdAvaliable = QString::number(record.nextStationIdAvaliable ? 1 : 0);
    // 当前站ID有效
    QString currentStationIdAvaliable = QString::number(record.currentStationIdAvaliable ? 1 : 0);
    // 目标距离有效
    QString targetDistanceAvaliable = QString::number(record.targetDistanceAvaliable ? 1 : 0);
    // 起始距离有效
    QString startDistanceAvaliable = QString::number(record.startDistanceAvaliable ? 1 : 0);
    // ATO模式
    QString ATOMode = QString::number(record.ATOMode ? 1 : 0);
    // Tmc1司机室
    QString TmcActivity_1 = QString::number(record.TmcActivity_1 ? 1 : 0);
    // Tmc2司机室
    QString TmcActivity_2 = QString::number(record.TmcActivity_2 ? 1 : 0);
    // 惰行
    QString coasting = QString::number(record.coasting ? 1 : 0);
    // 牵引
    QString traction = QString::number(record.traction ? 1 : 0);
    // 制动
    QString ebraking = QString::number(record.ebraking ? 1 : 0);
    // 载荷AW0
    QString loadAW_0 = QString::number(record.loadAW_0 ? 1 : 0);
    // 载荷AW2
    QString loadAW_2 = QString::number(record.loadAW_2 ? 1 : 0);
    // 载荷AW3
    QString loadAW_3 = QString::number(record.loadAW_3 ? 1 : 0);
    // 位置：保留1位小数
    QString position = QString::number(record.position, 'f', 1);
    
    
    
    return QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,%21,%22,%23,%24,%25,%26,%27,%28,%29,%30,%31")
        .arg(time)
        .arg(lifeSignal)
        .arg(railwayID)
        .arg(endStationID)
        .arg(nextStationID)
        .arg(currentStationID)
        .arg(targetDist)
        .arg(startDist)
        .arg(trainLoad)
        .arg(limitSpeed)
        .arg(netVoltage)
        .arg(netCurrent)
        .arg(speed)
        .arg(tractionForce)
        .arg(ebrakeForce)
        .arg(airbrakeForce)
        .arg(endStationIdAvaliable)
        .arg(nextStationIdAvaliable)
        .arg(currentStationIdAvaliable)
        .arg(targetDistanceAvaliable)
        .arg(startDistanceAvaliable)
        .arg(ATOMode)
        .arg(TmcActivity_1)
        .arg(TmcActivity_2)
        .arg(coasting)
        .arg(traction)
        .arg(ebraking)
        .arg(loadAW_0)
        .arg(loadAW_2)
        .arg(loadAW_3)
        .arg(position);
}

