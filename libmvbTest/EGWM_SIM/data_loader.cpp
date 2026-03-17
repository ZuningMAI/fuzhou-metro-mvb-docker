#include "data_loader.h"
#include <QDebug>

// 数据反转义函数
QByteArray unescapeData(const QByteArray &escapedData)
{
    QByteArray unescapedData;
    unescapedData.reserve(escapedData.size());

    for (int i = 0; i < escapedData.size(); ++i) {
        quint8 currentByte = static_cast<quint8>(escapedData[i]);

        // 检查是否是转义序列的开始 (0x7D)
        if (currentByte == 0x7D && i + 1 < escapedData.size()) {
            quint8 nextByte = static_cast<quint8>(escapedData[i + 1]);

            // 处理转义序列
            if (nextByte == 0x5D) {
                // 0x7D 0x5D 反转义为 0x7D
                unescapedData.append(static_cast<char>(0x7D));
                i++; // 跳过下一个字节
            } else if (nextByte == 0x5E) {
                // 0x7D 0x5E 反转义为 0x7E
                unescapedData.append(static_cast<char>(0x7E));
                i++; // 跳过下一个字节
            } else {
                // 无效的转义序列，保持原样
                unescapedData.append(escapedData[i]);
            }
        } else {
            // 普通字节，直接添加
            unescapedData.append(escapedData[i]);
        }
    }

    return unescapedData;
}



DataLoader::DataLoader()
    : m_callback(nullptr)
{
}

DataLoader::~DataLoader()
{
}

DataLoader& DataLoader::instance()
{
    static DataLoader instance;
    return instance;
}

void DataLoader::setDataCallback(MvbDataCallback callback)
{
    QMutexLocker locker(&m_mutex);
    m_callback = callback;
}

void DataLoader::clearCallback()
{
    QMutexLocker locker(&m_mutex);
    m_callback = nullptr;
}

bool DataLoader::hasCallback() const
{
    return m_callback != nullptr;
}

void DataLoader::logSentData(const QByteArray &usartPppFrame)
{
    QMutexLocker locker(&m_mutex);
    
    // 如果没有设置回调，直接返回
    if (!m_callback) {
        return;
    }

    QByteArray frameContent = usartPppFrame.mid(0,-1);
    QByteArray unescapedFrameContent = unescapeData(frameContent);
    
    // USART-PPP帧格式: 7E + (21 + size(1) + PD(2) + data) + CRC(2) + 7E
    // 检查帧的有效性
    if (unescapedFrameContent.size() < 8) {
        qWarning() << "DataLoader: 帧长度不足，无法提取数据";
        return;
    }
    
    // 检查起始标志
    if (static_cast<quint8>(unescapedFrameContent[0]) != 0x7E) {
        qWarning() << "DataLoader: 无效的帧起始标志";
        return;
    }
    
    // 查找最后一个0x7E的位置（结束标志）
    int endPos = -1;
    for (int i = unescapedFrameContent.size() - 1; i > 0; --i) {
        if (static_cast<quint8>(unescapedFrameContent[i]) == 0x7E) {
            endPos = i;
            break;
        }
    }
    
    if (endPos <= 0) {
        qWarning() << "DataLoader: 未找到帧结束标志";
        return;
    }

    
    // 提取帧内容（去掉头尾的0x7E和最后的CRC(2字节)）
    // 内容格式: 21 + size(1) + PD(2) + data
    int contentStart = 1;  // 跳过起始0x7E
    int contentEnd = endPos - 3;  // 去掉CRC(2字节)和结束0x7E
    
    if (contentEnd - contentStart < 4) {
        qWarning() << "DataLoader: 帧内容长度不足";
        return;
    }
    
    // 检查是否是MVB数据包（第一个字节应该是0x21）
    if (static_cast<quint8>(unescapedFrameContent[contentStart]) != 0x21) {
        qWarning() << "DataLoader: 不是有效的MVB数据包";
        return;
    }

    
    // 提取PD号（2字节，大端序）
    quint8 pdHigh = static_cast<quint8>(unescapedFrameContent[contentStart + 2]);
    quint8 pdLow = static_cast<quint8>(unescapedFrameContent[contentStart + 3]);
    quint16 pdAddress = (static_cast<quint16>(pdHigh) << 8) | pdLow;
    
    // 提取data部分（从PD之后到CRC之前）
    int dataStart = contentStart + 4;
    int dataEnd = contentEnd;
    
    if (dataEnd <= dataStart) {
        qWarning() << "DataLoader: 没有数据部分";
        return;
    }
    
    QByteArray dataBytes = unescapedFrameContent.mid(dataStart, dataEnd - dataStart + 1);
    
    // 调用回调函数
    m_callback(pdAddress, dataBytes);
    
    // 调试输出
    QString pdStr = QString("0x%1").arg(pdAddress, 4, 16, QChar('0')).toUpper();
    QString dataStr = dataBytes.toHex(' ').toUpper();
    qDebug() << "DataLoader: PD=" << pdStr << "Data=" << dataStr;
}
