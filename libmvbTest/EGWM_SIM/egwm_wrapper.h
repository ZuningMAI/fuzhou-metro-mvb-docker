#ifndef EGWM_WRAPPER_H
#define EGWM_WRAPPER_H

#include <QString>
#include <QByteArray>
#include <functional>

/**
 * @brief EGWM仿真器封装类
 * 提供简单的接口供外部程序调用
 */
class EGWMWrapper
{
public:
    /**
     * @brief 数据回调函数类型
     * @param pdAddress PD端口地址
     * @param data 数据内容
     */
    using DataCallback = std::function<void(quint16 pdAddress, const QByteArray &data)>;

    /**
     * @brief 初始化EGWM仿真器
     * @param configFile 配置文件路径
     * @param logDir 日志目录
     * @return 成功返回true，失败返回false
     */
    static bool initialize(const QString &configFile, const QString &logDir = "Log");

    /**
     * @brief 设置数据回调函数
     * @param callback 回调函数
     */
    static void setDataCallback(DataCallback callback);

    /**
     * @brief 启动仿真器
     * @return 成功返回true，失败返回false
     */
    static bool start();

    /**
     * @brief 停止仿真器
     */
    static void stop();

    /**
     * @brief 处理Qt事件（需要在主循环中定期调用）
     */
    static void processEvents();

    /**
     * @brief 检查仿真器是否正在运行
     * @return 运行中返回true，否则返回false
     */
    static bool isRunning();

    /**
     * @brief 清理资源
     */
    static void cleanup();
};

#endif // EGWM_WRAPPER_H
