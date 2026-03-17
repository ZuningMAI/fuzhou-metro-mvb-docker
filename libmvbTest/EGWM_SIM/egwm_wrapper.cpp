#include "egwm_wrapper.h"
#include "simulation_controller.h"
#include "simulator_config.h"
#include "logger.h"
#include "data_loader.h"
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QDebug>

// 静态成员变量
static SimulationController* g_controller = nullptr;
static SimulatorConfig* g_config = nullptr;
static Logger* g_logger = nullptr;
static bool g_initialized = false;
static bool g_running = false;

bool EGWMWrapper::initialize(const QString &configFile, const QString &logDir)
{
    if (g_initialized) {
        qWarning() << "EGWM仿真器已经初始化";
        return true;
    }

    // 初始化日志系统
    g_logger = Logger::getInstance();
    if (!g_logger->initialize(logDir)) {
        qWarning() << "日志系统初始化失败";
        return false;
    }
    g_logger->setConsoleOutput(true);
    
    LOG_INFO("========== EGWM仿真器初始化 ==========");
    LOG_INFO(QString("Qt版本: %1").arg(QT_VERSION_STR));

    // 检查数据目录
    QDir dataDir("data");
    if (!dataDir.exists()) {
        LOG_ERROR("数据目录不存在: data/");
        qWarning() << "数据目录不存在: data/";
        return false;
    }
    
    QDir fz601Dir("data/FZ601");
    QDir fz602Dir("data/FZ602");
    
    bool hasFZ601 = fz601Dir.exists();
    bool hasFZ602 = fz602Dir.exists();
    
    if (!hasFZ601 && !hasFZ602) {
        LOG_ERROR("未找到FZ601或FZ602数据目录");
        qWarning() << "未找到FZ601或FZ602数据目录";
        return false;
    }
    
    LOG_INFO("数据目录检查通过");
    if (hasFZ601) {
        LOG_INFO("  ✓ FZ601 数据目录存在");
    }
    if (hasFZ602) {
        LOG_INFO("  ✓ FZ602 数据目录存在");
    }

    // 检查配置文件
    if (!QFile::exists(configFile)) {
        LOG_ERROR(QString("配置文件不存在: %1").arg(configFile));
        qWarning() << "配置文件不存在:" << configFile;
        return false;
    }
    
    // 加载配置
    g_config = new SimulatorConfig();
    if (!g_config->loadFromFile(configFile)) {
        LOG_ERROR(QString("配置文件加载失败: %1").arg(configFile));
        qWarning() << "配置文件加载失败";
        delete g_config;
        g_config = nullptr;
        return false;
    }
    
    LOG_INFO(QString("配置文件加载成功: %1").arg(configFile));
    
    // 显示配置信息
    LOG_INFO("========== 配置信息 ==========");
    LOG_INFO(QString("线路: %1 (%2号线)").arg(g_config->getRailwayLine()).arg(g_config->getLineID()));
    LOG_INFO(QString("列车号: %1, 车辆号: %2, 载荷: %3吨")
             .arg(g_config->getTrainNum())
             .arg(g_config->getTrainInfo())
             .arg(g_config->getTrainLoad()));
    LOG_INFO(QString("串口: %1@%2").arg(g_config->getPortName()).arg(g_config->getBaudRate()));
    LOG_INFO(QString("运行模式: %1")
             .arg(g_config->getRunMode() == SimulatorConfig::SECTION_MODE ? "区间模式" : "线路模式"));
    LOG_INFO(QString("数据处理: %1")
             .arg(g_config->getDataProcessMode() == SimulatorConfig::INTERPOLATION_MODE ? "插值" : "维持"));
    LOG_INFO(QString("状态数据周期: %1ms").arg(g_config->getRunInfoPeriodMs()));
    LOG_INFO("==============================");

    g_initialized = true;
    return true;
}

void EGWMWrapper::setDataCallback(DataCallback callback)
{
    if (!g_initialized) {
        qWarning() << "EGWM仿真器未初始化";
        return;
    }

    // 设置DataLoader回调函数
    DataLoader::instance().setDataCallback(
        [callback](quint16 pdAddress, const QByteArray &data) {
            if (callback) {
                callback(pdAddress, data);
            }
        }
    );
    
    LOG_INFO("DataLoader回调函数已设置");
}

bool EGWMWrapper::start()
{
    if (!g_initialized) {
        qWarning() << "EGWM仿真器未初始化";
        return false;
    }

    if (g_running) {
        qWarning() << "EGWM仿真器已经在运行";
        return true;
    }

    // 创建仿真控制器
    g_controller = new SimulationController(*g_config);
    
    // 连接信号
    QObject::connect(g_controller, &SimulationController::sectionCompleted,
                    [](int index, const QString &name) {
        qDebug() << "";
        qDebug() << "===== 区间完成 =====";
        qDebug() << "区间" << (index + 1) << ":" << name;
        qDebug() << "====================";
        qDebug() << "";
    });
    
    QObject::connect(g_controller, &SimulationController::lineCompleted, []() {
        // 完成信息已在 SimulationController 中根据模式显示
    });
    
    QObject::connect(g_controller, &SimulationController::error, [](const QString &msg) {
        qWarning() << "仿真器错误:" << msg;
    });
    
    QObject::connect(g_controller, &SimulationController::statusUpdate, [](const QString &status) {
        qDebug() << "[仿真状态]" << status;
    });

    // 启动仿真器
    if (!g_controller->start()) {
        LOG_ERROR("仿真器启动失败");
        qWarning() << "仿真器启动失败";
        delete g_controller;
        g_controller = nullptr;
        return false;
    }
    
    LOG_INFO("仿真器已成功启动");
    g_running = true;
    return true;
}

void EGWMWrapper::stop()
{
    if (!g_running) {
        return;
    }

    if (g_controller) {
        delete g_controller;
        g_controller = nullptr;
    }

    g_running = false;
    LOG_INFO("仿真器已停止");
}

void EGWMWrapper::processEvents()
{
    if (QCoreApplication::instance()) {
        QCoreApplication::processEvents();
    }
}

bool EGWMWrapper::isRunning()
{
    return g_running;
}

void EGWMWrapper::cleanup()
{
    stop();

    // 清理DataLoader回调
    DataLoader::instance().clearCallback();

    if (g_config) {
        delete g_config;
        g_config = nullptr;
    }

    if (g_logger) {
        LOG_INFO("========== EGWM仿真器清理完成 ==========");
        g_logger->close();
        g_logger = nullptr;
    }

    g_initialized = false;
}
