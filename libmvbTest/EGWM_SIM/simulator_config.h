#ifndef SIMULATOR_CONFIG_H
#define SIMULATOR_CONFIG_H

#include <QString>
#include <QVector>

// 轨迹数据点
struct TrajectoryPoint {
    double relativeTime;    // 相对时间 (s)
    double position;        // 位置 (m)
    double speed;           // 速度 (km/h)
    double force;           // 力 (kN)
    int operationMode;      // 操纵工况 (0=牵引, 1=恒速, 2=惰行, 3=制动)
    
    TrajectoryPoint() 
        : relativeTime(0), position(0), speed(0), force(0), operationMode(0) {}
    
    TrajectoryPoint(double t, double p, double s, double f, int m)
        : relativeTime(t), position(p), speed(s), force(f), operationMode(m) {}
};

// 区间信息
struct SectionInfo {
    QString railwayLine;        // 线路名称 (FZ601/FZ602)
    int startStation;           // 起始站 ID
    int endStation;             // 终点站 ID
    double startPosition;       // 起始位置 (m)
    double endPosition;         // 终点位置 (m)
    double plannedTime;         // 计划运行时间 (s)
    QVector<TrajectoryPoint> trajectory;  // 轨迹数据
    QString csvFilePath;        // CSV文件路径
    
    SectionInfo() 
        : startStation(0), endStation(0), startPosition(0), 
          endPosition(0), plannedTime(0) {}
};

// 限速配置
struct SpeedLimitEntry {
    double start;       // 起始位置 (m)
    double end;         // 结束位置 (m)
    int limit;          // 限速值 (km/h)
    int inchain;        // 是否在链中
    
    SpeedLimitEntry(double s, double e, int l, int ic)
        : start(s), end(e), limit(l), inchain(ic) {}
};

// 仿真配置
class SimulatorConfig {
public:
    // 运行模式
    enum RunMode {
        SECTION_MODE,           // 区间模式
        LINE_MODE               // 线路模式
    };
    
    // 数据处理方式
    enum DataProcessMode {
        MAINTAIN_MODE,          // 维持
        INTERPOLATION_MODE      // 插值
    };
    
    SimulatorConfig();
    
    // 从JSON文件加载配置
    bool loadFromFile(const QString &filePath);
    
    // 保存到JSON文件
    bool saveToFile(const QString &filePath) const;
    
    // Getters
    QString getRailwayLine() const { return m_railwayLine; }
    int getLineID() const { return m_lineID; }
    quint16 getTrainNum() const { return m_trainNum; }
    quint32 getTrainInfo() const { return m_trainInfo; }
    double getTrainLoad() const { return m_trainLoad; }
    QString getPortName() const { return m_portName; }
    int getBaudRate() const { return m_baudRate; }
    RunMode getRunMode() const { return m_runMode; }
    DataProcessMode getDataProcessMode() const { return m_dataProcessMode; }
    QString getCsvFile() const { return m_csvFile; }
    int getStopTimeMin() const { return m_stopTimeMin; }
    int getStopTimeMax() const { return m_stopTimeMax; }
    bool isFrameSplitEnabled() const { return m_enableFrameSplit; }
    int getRunInfoPeriodMs() const { return m_runInfoPeriodMs; }  // 状态数据周期(ms)
    bool isRunInfoLoggingEnabled() const { return m_enableRunInfoLogging; }  // RunInfo数据记录
    
    // Setters
    void setRailwayLine(const QString &line) { m_railwayLine = line; }
    void setLineID(int id) { m_lineID = id; }
    void setTrainNum(quint16 num) { m_trainNum = num; }
    void setTrainInfo(quint32 info) { m_trainInfo = info; }
    void setTrainLoad(double load) { m_trainLoad = load; }
    void setPortName(const QString &port) { m_portName = port; }
    void setBaudRate(int rate) { m_baudRate = rate; }
    void setRunMode(RunMode mode) { m_runMode = mode; }
    void setDataProcessMode(DataProcessMode mode) { m_dataProcessMode = mode; }
    void setCsvFile(const QString &file) { m_csvFile = file; }
    void setStopTimeMin(int min) { m_stopTimeMin = min; }
    void setStopTimeMax(int max) { m_stopTimeMax = max; }
    void setEnableFrameSplit(bool enable) { m_enableFrameSplit = enable; }
    void setRunInfoPeriodMs(int periodMs) { m_runInfoPeriodMs = periodMs; }
    void setEnableRunInfoLogging(bool enable) { m_enableRunInfoLogging = enable; }
    
private:
    // 线路配置
    QString m_railwayLine;      // FZ601 或 FZ602
    int m_lineID;               // 线路ID (6)
    
    // 列车配置
    quint16 m_trainNum;         // 列车号
    quint32 m_trainInfo;        // 车辆号
    double m_trainLoad;         // 列车载荷 (t)
    
    // 串口配置
    QString m_portName;         // 串口号
    int m_baudRate;             // 波特率
    
    // 仿真配置
    RunMode m_runMode;
    DataProcessMode m_dataProcessMode;
    QString m_csvFile;          // 区间模式下的CSV文件路径
    int m_stopTimeMin;          // 停站时间最小值 (s)
    int m_stopTimeMax;          // 停站时间最大值 (s)
    bool m_enableFrameSplit;    // 是否启用帧截断
    int m_runInfoPeriodMs;      // 状态数据(PD 0xA0)发送周期 (ms), 可选: 128, 256, 512
    bool m_enableRunInfoLogging; // 是否启用RunInfo数据记录到CSV
};

#endif // SIMULATOR_CONFIG_H

