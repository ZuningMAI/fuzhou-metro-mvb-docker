#ifndef MVB_SEND_H
#define MVB_SEND_H

#include <QByteArray>
#include "dataDef.h"


class mvb_send
{
public:
    mvb_send();
    
    static QByteArray structToByteArray(const DateTimeStruct& data);
    static QByteArray structToByteArray(const TrainStruct& data);
    static QByteArray structToByteArray(const RunInfoStruct& data);
    static QByteArray structToByteArray(const PosInfoStruct& data);
    static QByteArray structToByteArray(const RailwayInfoStruct& data);
    static QByteArray structToByteArray(const CarriageInfoStruct& data);
    static QByteArray structToByteArray(const AirbrakePowerStruct& data);
    


    // 根据PD_TYPE枚举值和数据生成CRC检验前的MVB数据包
    static QByteArray generateMvbPacket(QByteArray &data, PD_TYPE type);

    // CRC-16/X25计算相关函数
    static quint16 crc16_direct(quint16 crc, quint8 data);
    static quint16 calculate_crc16_direct(const quint8 *data, size_t len);
    // 对数据进行CRC16校验并拼接结果
    static QByteArray appendCrc16(QByteArray &data); 

    // 对数据进行转义处理，形成UART-PPP 帧
    static QByteArray unescapeReceivedData(QByteArray &data);

};

#endif // MVB_SEND_H
