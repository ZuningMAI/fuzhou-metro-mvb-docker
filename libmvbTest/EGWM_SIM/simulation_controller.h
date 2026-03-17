#ifndef SIMULATION_CONTROLLER_H
#define SIMULATION_CONTROLLER_H

#include "simulator_config.h"
#include "csv_reader.h"
#include "data_interpolator.h"
#include "speed_limiter.h"
#include "data_generator.h"
#include "serial_sender.h"
#include "runinfo_logger.h"
#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QElapsedTimer>

class SimulationController : public QObject {
    Q_OBJECT
    
public:
    SimulationController(const SimulatorConfig &config, QObject *parent = nullptr);
    ~SimulationController();
    
    // 开始仿真
    bool start();
    
    // 停止仿真
    void stop();
    
    // 暂停/恢复
    void pause();
    void resume();
    
    // 获取仿真状态
    bool isRunning() const { return m_isRunning; }
    bool isPaused() const { return m_isPaused; }
    
signals:
    void sectionCompleted(int sectionIndex, const QString &sectionName);
    void lineCompleted();
    void error(const QString &message);
    void statusUpdate(const QString &status);
    
private slots:
    void onTimer512ms();        // 处理512ms定时器（时间数据和生命信号）
    void onTimer1024ms();       // 处理1024ms定时器（车号数据）
    void onTimerRunInfo();      // 处理状态数据定时器（可配置周期）
    
private:
    // 初始化串口
    bool initSerial();
    
    // 加载数据
    bool loadData();
    
    // 切换到下一个区间
    void switchToNextSection();
    
    // 处理停站
    void handleStopAtStation();
    
    // 更新当前状态
    void updateCurrentState();
    
    // 获取当前轨迹点
    TrajectoryPoint getCurrentTrajectoryPoint();
    
    // 生成并发送时间数据
    void sendTimeData();
    
    // 生成并发送车号数据
    void sendTrainData();
    
    // 生成并发送运行状态数据
    void sendRunInfoData();
    
    // 计算当前站点ID
    int getCurrentStationID() const;
    int getNextStationID() const;
    int getEndStationID() const;
    
    // 获取区间名称
    QString getSectionName(int sectionIndex) const;
    
private:
    SimulatorConfig m_config;
    QSerialPort *m_serial;
    SerialSender *m_sender;
    SpeedLimiter *m_speedLimiter;
    RunInfoLogger *m_runInfoLogger;  // RunInfo数据记录器
    
    // 数据
    QVector<SectionInfo> m_sections;
    int m_currentSectionIndex;
    
    // 仿真状态
    bool m_isRunning;
    bool m_isPaused;
    double m_simulationTime;        // 当前仿真时间（从区间开始计算，秒）
    double m_sectionStartTime;      // 区间开始时间（相对于整个仿真开始）
    bool m_isStopAtStation;         // 是否在站台停车
    double m_stopStartTime;         // 停车开始时间
    double m_stopDuration;          // 停车时长（秒）
    
    // 生命信号
    quint16 m_lifeSignal;
    
    // 定时器
    QTimer *m_timer512ms;
    QTimer *m_timer1024ms;
    QTimer *m_timerRunInfo;         // 状态数据定时器（可配置周期）
    QElapsedTimer m_elapsedTimer;   // 用于计时
    qint64 m_lastUpdateTime;        // 上次更新时间（毫秒）
};

#endif // SIMULATION_CONTROLLER_H

