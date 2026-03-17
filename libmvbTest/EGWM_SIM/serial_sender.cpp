#include "serial_sender.h"
#include "logger.h"
#include "data_loader.h"
#include <QDebug>
#include <cstdlib>
#include <ctime>

SerialSender::SerialSender(QSerialPort *serial, bool enableSplit)
    : m_serial(serial)
    , m_enableFrameSplit(enableSplit)
{
    // 初始化随机数生成器
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(nullptr)));
        seeded = true;
    }
}

bool SerialSender::sendData(const QByteArray &usartPppFrame)
{
    if (!m_serial || !m_serial->isOpen()) {
        qWarning() << "串口未打开";
        return false;
    }
    
    // 如果启用了帧截断且帧长度大于34字节（包含头尾0x7E）
    if (m_enableFrameSplit && usartPppFrame.size() > 34) {
        // 提取原始PD号
        quint8 pdType = 0x00;
        if (usartPppFrame.size() >= 5 && static_cast<quint8>(usartPppFrame[0]) == 0x7E) {
            pdType = static_cast<quint8>(usartPppFrame[4]);
        }
        
        QVector<QByteArray> frames = splitFrame(usartPppFrame);
        
        bool allSuccess = true;
        for (int i = 0; i < frames.size(); ++i) {
            const QByteArray &frame = frames[i];
            
            // 发送帧
            QString hexData = frame.toHex(' ');
            LOG_INFO(QString("发送的USART-PPP帧数据（分段%1/%2）：%3").arg(i+1).arg(frames.size()).arg(hexData));
            LOG_INFO(QString("尝试发送%1字节数据到串口").arg(frame.size()));
            
            qint64 bytesWritten = m_serial->write(frame);
            
            if (bytesWritten == -1) {
                qWarning() << "发送数据失败:" << m_serial->errorString();
                LOG_ERROR(QString("发送数据失败: %1").arg(m_serial->errorString()));
                allSuccess = false;
                continue;
            }
            
            if (!m_serial->waitForBytesWritten(1000)) {
                qWarning() << "等待发送超时";
                LOG_ERROR("等待发送超时");
                allSuccess = false;
                continue;
            }
            
            LOG_INFO(QString("成功发送%1字节数据到串口").arg(bytesWritten));
            
            // 记录分段帧到日志（只有第一个分段有PD号）
            if (i == 0) {
                LOG_FRAME(pdType, QString("分段1/%1: %2").arg(frames.size()).arg(hexData), bytesWritten, frame.size());
            } else {
                LOG_FRAME(0x00, QString("分段%1/%2(续): %3").arg(i+1).arg(frames.size()).arg(hexData), bytesWritten, frame.size());
            }
        }
        
        return allSuccess;
    } else {
        return sendToSerial(usartPppFrame);
    }
}

bool SerialSender::sendDateTimeStruct(const DateTimeStruct &data)
{
    QByteArray dataArray = mvb_send::structToByteArray(data);
    QByteArray packet = mvb_send::generateMvbPacket(dataArray, TIME_PORT_TYPE);
    QByteArray packetWithCrc = mvb_send::appendCrc16(packet);
    QByteArray usartPppFrame = mvb_send::unescapeReceivedData(packetWithCrc);
    
    return sendData(usartPppFrame);
}

bool SerialSender::sendTrainStruct(const TrainStruct &data, PD_TYPE portType)
{
    QByteArray dataArray = mvb_send::structToByteArray(data);
    QByteArray packet = mvb_send::generateMvbPacket(dataArray, portType);
    QByteArray packetWithCrc = mvb_send::appendCrc16(packet);
    QByteArray usartPppFrame = mvb_send::unescapeReceivedData(packetWithCrc);
    
    return sendData(usartPppFrame);
}

bool SerialSender::sendRunInfoStruct(const RunInfoStruct &data)
{
    QByteArray dataArray = mvb_send::structToByteArray(data);
    QByteArray packet = mvb_send::generateMvbPacket(dataArray, RUNINFO_PORT_TYPE);
    QByteArray packetWithCrc = mvb_send::appendCrc16(packet);
    QByteArray usartPppFrame = mvb_send::unescapeReceivedData(packetWithCrc);
    
    return sendData(usartPppFrame);
}

QVector<QByteArray> SerialSender::splitFrame(const QByteArray &frame)
{
    QVector<QByteArray> result;
    
    // 去掉头尾的0x7E
    if (frame.size() < 3 || frame[0] != 0x7E || frame[frame.size() - 1] != 0x7E) {
        qWarning() << "无效的UART-PPP帧格式";
        result.append(frame);
        return result;
    }
    
    QByteArray content = frame.mid(1, frame.size() - 2);
    
    // 随机选择分割点（16~25字节）
    int splitPoint = 16 + (rand() % 10);  // 16~25
    
    if (splitPoint >= content.size()) {
        // 如果分割点超过内容长度，不分割
        result.append(frame);
        return result;
    }
    
    // 分割成两部分
    QByteArray part1 = content.left(splitPoint);
    QByteArray part2 = content.mid(splitPoint);
    
    // 为每部分添加头尾0x7E
    QByteArray frame1;
    frame1.append(0x7E);
    frame1.append(part1);
    // frame1.append(0x7E);
    
    QByteArray frame2;
    // frame2.append(0x7E);
    frame2.append(part2);
    frame2.append(0x7E);
    
    result.append(frame1);
    result.append(frame2);
    
    // qDebug() << "帧已分割: 原始" << frame.size() << "字节 -> " 
    //          << frame1.size() << "字节 + " << frame2.size() << "字节";
    LOG_INFO(QString("帧已分割: 原始%1字节 -> %2字节 + %3字节").arg(frame.size()).arg(frame1.size()).arg(frame2.size()));
    return result;
}

bool SerialSender::sendToSerial(const QByteArray &data)
{
    if (!m_serial || !m_serial->isOpen()) {
        qWarning() << "串口未打开";
        LOG_ERROR("串口未打开，无法发送数据");
        return false;
    }
    
    // 提取PD号（如果可能）
    // 帧格式: 7E + (21 + size(2) + PD(1) + data) + CRC(2) + 7E
    quint8 pdType = 0x00;
    if (data.size() >= 5 && static_cast<quint8>(data[0]) == 0x7E) {
        pdType = static_cast<quint8>(data[4]);  // 第5个字节是PD号
    }
    
    // 显示16进制数据
    QString hexData = data.toHex(' ');
    // qDebug() << "发送的USART-PPP帧数据：" << "\"" + hexData + "\"";
    // qDebug() << "尝试发送" << data.size() << "字节数据到串口";
    // 用LOG_INFO记录发送的USART-PPP帧数据
    LOG_INFO(QString("发送的USART-PPP帧数据：%1").arg(hexData));
    LOG_INFO(QString("尝试发送%1字节数据到串口").arg(data.size()));
    
    qint64 bytesWritten = m_serial->write(data);
    
    if (bytesWritten == -1) {
        qWarning() << "发送数据失败:" << m_serial->errorString();
        LOG_ERROR(QString("发送数据失败: %1").arg(m_serial->errorString()));
        return false;
    }
    
    // 等待数据发送完成
    if (!m_serial->waitForBytesWritten(1000)) {
        qWarning() << "等待发送超时";
        LOG_ERROR("等待发送超时");
        return false;
    }
    
    // qDebug() << "成功发送" << bytesWritten << "字节数据到串口";
    LOG_INFO(QString("成功发送%1字节数据到串口").arg(bytesWritten));
    
    // 记录到日志文件
    LOG_FRAME(pdType, hexData, bytesWritten, data.size());
    
    // 记录到dataloader.txt
    DataLoader::instance().logSentData(data);
    
    return true;
}


bool SerialSender::sendPosInfoStruct(const PosInfoStruct &data)
{
    QByteArray dataArray = mvb_send::structToByteArray(data);
    QByteArray packet = mvb_send::generateMvbPacket(dataArray, POSINFO_PORT_TYPE);
    QByteArray packetWithCrc = mvb_send::appendCrc16(packet);
    QByteArray usartPppFrame = mvb_send::unescapeReceivedData(packetWithCrc);
    
    return sendData(usartPppFrame);
}

bool SerialSender::sendRailwayInfoStruct(const RailwayInfoStruct &data)
{
    QByteArray dataArray = mvb_send::structToByteArray(data);
    QByteArray packet = mvb_send::generateMvbPacket(dataArray, RAILWAYINFO_PORT_TYPE);
    QByteArray packetWithCrc = mvb_send::appendCrc16(packet);
    QByteArray usartPppFrame = mvb_send::unescapeReceivedData(packetWithCrc);
    
    return sendData(usartPppFrame);
}

bool SerialSender::sendCarriageInfoStruct(const CarriageInfoStruct &data, PD_TYPE portType)
{
    QByteArray dataArray = mvb_send::structToByteArray(data);
    QByteArray packet = mvb_send::generateMvbPacket(dataArray, portType);
    QByteArray packetWithCrc = mvb_send::appendCrc16(packet);
    QByteArray usartPppFrame = mvb_send::unescapeReceivedData(packetWithCrc);
    
    return sendData(usartPppFrame);
}

bool SerialSender::sendAirbrakePowerStruct(const AirbrakePowerStruct &data, PD_TYPE portType)
{
    QByteArray dataArray = mvb_send::structToByteArray(data);
    QByteArray packet = mvb_send::generateMvbPacket(dataArray, portType);
    QByteArray packetWithCrc = mvb_send::appendCrc16(packet);
    QByteArray usartPppFrame = mvb_send::unescapeReceivedData(packetWithCrc);
    
    return sendData(usartPppFrame);
}
