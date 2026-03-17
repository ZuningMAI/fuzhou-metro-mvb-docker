#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QString>
#include <QDir>

/**
 * @brief 日志管理器类
 * 
 * 负责将程序运行信息和发送的USART-PPP帧数据记录到日志文件中
 */
class Logger : public QObject
{
    Q_OBJECT

public:
    enum LogLevel {
        DEBUG,    // 调试信息
        INFO,     // 一般信息
        WARNING,  // 警告信息
        ERROR     // 错误信息
    };

    /**
     * @brief 获取日志管理器单例
     */
    static Logger* getInstance();

    /**
     * @brief 初始化日志系统
     * @param logDir 日志目录（默认为 "bin/Log"）
     * @return 是否初始化成功
     */
    bool initialize(const QString& logDir = "Log");

    /**
     * @brief 关闭日志系统
     */
    void close();

    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 日志消息
     */
    void log(LogLevel level, const QString& message);

    /**
     * @brief 记录调试信息
     */
    void debug(const QString& message);

    /**
     * @brief 记录一般信息
     */
    void info(const QString& message);

    /**
     * @brief 记录警告信息
     */
    void warning(const QString& message);

    /**
     * @brief 记录错误信息
     */
    void error(const QString& message);

    /**
     * @brief 记录USART-PPP帧数据
     * @param pdType PD号（如0xFF, 0xF1, 0xA0等）
     * @param frameData 帧数据的十六进制字符串
     * @param bytesWritten 成功发送的字节数
     * @param totalBytes 尝试发送的总字节数
     */
    void logFrame(quint8 pdType, const QString& frameData, qint64 bytesWritten, qint64 totalBytes);

    /**
     * @brief 设置是否同时输出到控制台
     */
    void setConsoleOutput(bool enable);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QString levelToString(LogLevel level) const;
    QString getCurrentTimestamp() const;

    static Logger* s_instance;
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;
    bool m_consoleOutput;
    bool m_initialized;
};

// 便捷的全局宏定义
#define LOG_DEBUG(msg) Logger::getInstance()->debug(msg)
#define LOG_INFO(msg) Logger::getInstance()->info(msg)
#define LOG_WARNING(msg) Logger::getInstance()->warning(msg)
#define LOG_ERROR(msg) Logger::getInstance()->error(msg)
#define LOG_FRAME(pd, data, written, total) Logger::getInstance()->logFrame(pd, data, written, total)

#endif // LOGGER_H

