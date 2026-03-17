#include <QCoreApplication>
#include "simulation_controller.h"
#include "simulator_config.h"
#include "logger.h"
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QDir>

void printUsage()
{
    qDebug() << "====================================";
    qDebug() << "    EGWM 设备仿真器 v1.0";
    qDebug() << "====================================";
}

bool checkDataDirectory()
{
    QDir dataDir("data");
    if (!dataDir.exists()) {
        qWarning() << "数据目录不存在: data/";
        qWarning() << "请确保data目录及CSV文件存在";
        return false;
    }
    
    QDir fz601Dir("data/FZ601");
    QDir fz602Dir("data/FZ602");
    
    bool hasFZ601 = fz601Dir.exists();
    bool hasFZ602 = fz602Dir.exists();
    
    if (!hasFZ601 && !hasFZ602) {
        qWarning() << "未找到FZ601或FZ602数据目录";
        return false;
    }
    
    qDebug() << "数据目录检查:";
    if (hasFZ601) {
        qDebug() << "  ✓ FZ601 数据目录存在";
    }
    if (hasFZ602) {
        qDebug() << "  ✓ FZ602 数据目录存在";
    }
    
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    // 打印使用说明
    printUsage();
    
    // 初始化日志系统
    Logger* logger = Logger::getInstance();
    if (!logger->initialize("Log")) {
        qWarning() << "日志系统初始化失败";
        return -1;
    }
    logger->setConsoleOutput(true);  // 同时输出到控制台
    
    LOG_INFO("========== EGWM 设备仿真器启动 ==========");
    LOG_INFO(QString("Qt版本: %1").arg(QT_VERSION_STR));
    
    // 检查数据目录
    if (!checkDataDirectory()) {
        LOG_ERROR("数据目录检查失败");
        qWarning() << "数据目录检查失败";
        return -1;
    }
    LOG_INFO("数据目录检查通过");

    
    // 解析命令行参数
    QString configFile = "config_line_mode.json";
    if (argc > 1) {
        configFile = argv[1];
    }
    
    qDebug() << "";
    qDebug() << "使用配置文件:" << configFile;
    
    // 检查配置文件是否存在
    if (!QFile::exists(configFile)) {
        qWarning() << "配置文件不存在:" << configFile;
        qWarning() << "请创建配置文件或使用 -h 查看帮助";
        return -1;
    }
    
    // 加载配置
    SimulatorConfig config;
    if (!config.loadFromFile(configFile)) {
        LOG_ERROR(QString("配置文件加载失败: %1").arg(configFile));
        qWarning() << "配置文件加载失败";
        return -1;
    }
    
    LOG_INFO(QString("配置文件加载成功: %1").arg(configFile));
    
    // 显示配置信息
    qDebug() << "";
    qDebug() << "========== 配置信息 ==========";
    // qDebug() << "线路:" << config.getRailwayLine() << "(" << config.getLineID() << "号线)";
    // qDebug() << "列车号:" << config.getTrainNum();
    // qDebug() << "车辆号:" << config.getTrainInfo();
    // qDebug() << "列车载荷:" << config.getTrainLoad() << "吨";
    // qDebug() << "串口:" << config.getPortName() << "@" << config.getBaudRate();
    // qDebug() << "运行模式:" << (config.getRunMode() == SimulatorConfig::SECTION_MODE ? "区间模式" : "线路模式");
    // qDebug() << "数据处理:" << (config.getDataProcessMode() == SimulatorConfig::INTERPOLATION_MODE ? "插值" : "维持");
    
    // 记录配置到日志
    LOG_INFO(QString("线路: %1 (%2号线)").arg(config.getRailwayLine()).arg(config.getLineID()));
    LOG_INFO(QString("列车号: %1, 车辆号: %2, 载荷: %3吨").arg(config.getTrainNum()).arg(config.getTrainInfo()).arg(config.getTrainLoad()));
    LOG_INFO(QString("串口: %1@%2").arg(config.getPortName()).arg(config.getBaudRate()));
    LOG_INFO(QString("运行模式: %1").arg(config.getRunMode() == SimulatorConfig::SECTION_MODE ? "区间模式" : "线路模式"));
    LOG_INFO(QString("数据处理: %1").arg(config.getDataProcessMode() == SimulatorConfig::INTERPOLATION_MODE ? "插值" : "维持"));
    LOG_INFO(QString("状态数据周期: %1ms").arg(config.getRunInfoPeriodMs()));
    LOG_INFO(QString("帧截断: %1").arg(config.isFrameSplitEnabled() ? "启用" : "禁用"));
    LOG_INFO(QString("RunInfo数据记录: %1").arg(config.isRunInfoLoggingEnabled() ? "启用" : "禁用"));
    LOG_INFO(QString("停站时间: %1~%2秒").arg(config.getStopTimeMin()).arg(config.getStopTimeMax()));
    LOG_INFO(QString("CSV文件: %1").arg(config.getCsvFile()));
    LOG_INFO(QString("=============================="));
    LOG_INFO(QString(""));
    
    if (config.getRunMode() == SimulatorConfig::SECTION_MODE) {
        qDebug() << "CSV文件:" << config.getCsvFile();
    }
  
    
    // 创建仿真控制器
    SimulationController controller(config);
    
    // 连接信号
    QObject::connect(&controller, &SimulationController::sectionCompleted,
                    [](int index, const QString &name) {
        qDebug() << "";
        qDebug() << "===== 区间完成 =====";
        qDebug() << "区间" << (index + 1) << ":" << name;
        qDebug() << "====================";
        qDebug() << "";
    });
    
    QObject::connect(&controller, &SimulationController::lineCompleted,
                    []() {
        // 完成信息已在 SimulationController 中根据模式显示
        // 这里不需要重复输出
    });
    
    QObject::connect(&controller, &SimulationController::error,
                    [](const QString &msg) {
        qWarning() << "错误:" << msg;
    });
    
    QObject::connect(&controller, &SimulationController::statusUpdate,
                    [](const QString &status) {
        qDebug() << "[状态]" << status;
    });
    
    // 启动仿真
    if (!controller.start()) {
        LOG_ERROR("仿真启动失败");
        qWarning() << "仿真启动失败";
        logger->close();
        return -1;
    }
    
    LOG_INFO("仿真已成功启动");
    qDebug() << "";
    qDebug() << "仿真正在运行...";
    qDebug() << "按 Ctrl+C 停止仿真";
    qDebug() << "";
    
    int result = a.exec();
    
    // 程序退出，关闭日志系统
    LOG_INFO("========== EGWM 设备仿真器退出 ==========");
    logger->close();
    
    return result;
}


