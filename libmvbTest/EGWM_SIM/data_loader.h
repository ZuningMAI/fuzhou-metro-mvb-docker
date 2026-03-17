#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <QString>
#include <QByteArray>
#include <QMutex>
#include <functional>

// 数据回调函数类型：参数为 PD号(uint16) 和 数据(QByteArray)
using MvbDataCallback = std::function<void(quint16 pdAddress, const QByteArray &data)>;

class DataLoader {
public:
    static DataLoader& instance();
    
    // 记录发送的MVB数据（从USART-PPP帧中提取PD和data）
    // 如果设置了回调函数，则调用回调；否则不做任何操作
    void logSentData(const QByteArray &usartPppFrame);
    
    // 设置数据回调函数（替代写入文件）
    void setDataCallback(MvbDataCallback callback);
    
    // 清除回调函数
    void clearCallback();
    
    // 检查是否设置了回调
    bool hasCallback() const;
    
private:
    DataLoader();
    ~DataLoader();
    
    // 禁止拷贝
    DataLoader(const DataLoader&) = delete;
    DataLoader& operator=(const DataLoader&) = delete;
    
    MvbDataCallback m_callback;
    QMutex m_mutex;
};

#endif // DATA_LOADER_H
