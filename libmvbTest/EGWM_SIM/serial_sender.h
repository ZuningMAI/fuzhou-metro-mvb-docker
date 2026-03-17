#ifndef SERIAL_SENDER_H
#define SERIAL_SENDER_H

#include "dataDef.h"
#include "mvb_send.h"
#include <QSerialPort>
#include <QByteArray>
#include <QVector>

class SerialSender {
public:
    SerialSender(QSerialPort *serial, bool enableSplit = false);
    
    // 发送数据（自动处理截断）
    bool sendData(const QByteArray &usartPppFrame);
    
    // 发送MVB数据结构
    bool sendDateTimeStruct(const DateTimeStruct &data);
    bool sendTrainStruct(const TrainStruct &data, PD_TYPE portType = TRAIN_PORT_1_TYPE);
    bool sendRunInfoStruct(const RunInfoStruct &data);
    bool sendPosInfoStruct(const PosInfoStruct &data);
    bool sendRailwayInfoStruct(const RailwayInfoStruct &data);
    bool sendCarriageInfoStruct(const CarriageInfoStruct &data, PD_TYPE portType);
    bool sendAirbrakePowerStruct(const AirbrakePowerStruct &data, PD_TYPE portType);
    
    // 设置是否启用帧截断
    void setEnableFrameSplit(bool enable) { m_enableFrameSplit = enable; }
    bool isFrameSplitEnabled() const { return m_enableFrameSplit; }
    
private:
    QSerialPort *m_serial;
    bool m_enableFrameSplit;
    
    // 将帧分割成多个部分发送
    QVector<QByteArray> splitFrame(const QByteArray &frame);
    
    // 实际发送数据到串口
    bool sendToSerial(const QByteArray &data);
};

#endif // SERIAL_SENDER_H

