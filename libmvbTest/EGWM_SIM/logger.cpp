#include "logger.h"
#include <QCoreApplication>
#include <QDebug>

Logger* Logger::s_instance = nullptr;

Logger* Logger::getInstance()
{
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

Logger::Logger()
    : m_consoleOutput(true)
    , m_initialized(false)
{
}

Logger::~Logger()
{
    close();
}

bool Logger::initialize(const QString& logDir)
{
    QMutexLocker locker(&m_mutex);

    if (m_initialized) {
        return true;
    }

    // 创建日志目录
    QDir dir;
    if (!dir.exists(logDir)) {
        if (!dir.mkpath(logDir)) {
            qWarning() << "无法创建日志目录:" << logDir;
            return false;
        }
    }

    // 生成日志文件名：mvb_send_YYYYMMDD_HHmmss.log
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString fileName = QString("%1/mvb_send_%2.log").arg(logDir).arg(timestamp);

    // 打开日志文件
    m_logFile.setFileName(fileName);
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qWarning() << "无法打开日志文件:" << fileName << ", 错误:" << m_logFile.errorString();
        return false;
    }

    m_logStream.setDevice(&m_logFile);
    // m_logStream.setCodec("UTF-8");  // 设置UTF-8编码

    m_initialized = true;

    // 写入日志文件头
    QString header = QString(80, '=');
    m_logStream << header << "\n";
    m_logStream << "    EGWM设备仿真器日志文件\n";
    m_logStream << "    启动时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    m_logStream << header << "\n\n";
    m_logStream.flush();

    qInfo() << "日志系统已初始化，日志文件:" << fileName;
    return true;
}

void Logger::close()
{
    QMutexLocker locker(&m_mutex);

    if (m_initialized && m_logFile.isOpen()) {
        // 写入日志文件尾
        QString footer = QString(80, '=');
        m_logStream << "\n" << footer << "\n";
        m_logStream << "    日志结束时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
        m_logStream << footer << "\n";
        m_logStream.flush();

        m_logFile.close();
        m_initialized = false;
    }
}

void Logger::log(LogLevel level, const QString& message)
{
    QMutexLocker locker(&m_mutex);

    if (!m_initialized) {
        return;
    }

    QString timestamp = getCurrentTimestamp();
    QString levelStr = levelToString(level);
    QString logLine = QString("[%1] [%2] %3").arg(timestamp).arg(levelStr).arg(message);

    // 写入日志文件
    m_logStream << logLine << "\n";
    m_logStream.flush();

    // 如果启用了控制台输出，也输出到控制台
    if (m_consoleOutput) {
        switch (level) {
        case DEBUG:
        case INFO:
            qInfo().noquote() << logLine;
            break;
        case WARNING:
            qWarning().noquote() << logLine;
            break;
        case ERROR:
            qCritical().noquote() << logLine;
            break;
        }
    }
}

void Logger::debug(const QString& message)
{
    log(DEBUG, message);
}

void Logger::info(const QString& message)
{
    log(INFO, message);
}

void Logger::warning(const QString& message)
{
    log(WARNING, message);
}

void Logger::error(const QString& message)
{
    log(ERROR, message);
}

void Logger::logFrame(quint8 pdType, const QString& frameData, qint64 bytesWritten, qint64 totalBytes)
{
    QMutexLocker locker(&m_mutex);

    if (!m_initialized) {
        return;
    }

    QString timestamp = getCurrentTimestamp();
    QString pdTypeStr;
    
    // 如果PD=0x00，表示是分段帧的续传部分（不含PD字段）
    if (pdType == 0x00 && frameData.contains("续")) {
        pdTypeStr = "----";  // 用----表示续传分段
    } else {
        pdTypeStr = QString("0x%1").arg(pdType, 2, 16, QChar('0')).toUpper();
    }
    
    // 格式化帧数据日志
    QString frameLine = QString("[%1] [FRAME] PD=%2 | 数据: %3 | 发送: %4/%5 字节")
                            .arg(timestamp)
                            .arg(pdTypeStr)
                            .arg(frameData)
                            .arg(bytesWritten)
                            .arg(totalBytes);

    // 写入日志文件
    m_logStream << frameLine << "\n";
    m_logStream.flush();

    // 如果启用了控制台输出，也输出到控制台
    if (m_consoleOutput) {
        qInfo().noquote() << frameLine;
    }
}

void Logger::setConsoleOutput(bool enable)
{
    QMutexLocker locker(&m_mutex);
    m_consoleOutput = enable;
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
    case DEBUG:   return "DEBUG  ";
    case INFO:    return "INFO   ";
    case WARNING: return "WARNING";
    case ERROR:   return "ERROR  ";
    default:      return "UNKNOWN";
    }
}

QString Logger::getCurrentTimestamp() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
}

