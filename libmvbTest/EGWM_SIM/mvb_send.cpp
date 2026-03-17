#include "mvb_send.h"



mvb_send::mvb_send()
{

}
QByteArray mvb_send::structToByteArray(const DateTimeStruct& data)
{

    QByteArray array(reinterpret_cast<const char*>(&data), sizeof(DateTimeStruct));
    return array;
}


// 直接计算CRC-16/X25，不使用查找表
quint16 mvb_send::crc16_direct(quint16 crc, quint8 data) {
    crc ^= data;
    for (int i = 0; i < 8; i++) {
        if (crc & 1) {
            crc = (crc >> 1) ^ 0x8408;  // 0x8408是0x1021的位反转
        } else {
            crc >>= 1;
        }
    }
    return crc;
}

quint16 mvb_send::calculate_crc16_direct(const quint8 *data, size_t len) {
    quint16 crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc = crc16_direct(crc, data[i]);
    }
    return ~crc;  // 最终异或0xFFFF
}

QByteArray mvb_send::structToByteArray(const TrainStruct& data)
{
    QByteArray array;
    array.resize(8);
    
    // // quint16 trainNum -> 2 bytes (big endian)
    // array[0] = (data.trainNum >> 8) & 0xFF;
    // array[1] = data.trainNum & 0xFF;
    
    // // quint32 trainInfo -> 4 bytes (big endian)
    // array[2] = (data.trainInfo >> 24) & 0xFF;
    // array[3] = (data.trainInfo >> 16) & 0xFF;
    // array[4] = (data.trainInfo >> 8) & 0xFF;
    // array[5] = data.trainInfo & 0xFF;
    
    // // quint8 resert_8 -> 1 byte
    // array[6] = data.resert_8;
    
    // // quint8 trainSet_flag -> 1 byte
    // array[7] = data.trainSet_flag;

    // 使用指针操作将数据按大端序写入数组
    quint16 trainNumBE = qToBigEndian(data.trainNum);
    quint32 trainInfoBE = qToBigEndian(data.trainInfo);
    
    memcpy(array.data(), &trainNumBE, sizeof(trainNumBE));
    memcpy(array.data() + sizeof(trainNumBE), &trainInfoBE, sizeof(trainInfoBE));
    array[6] = data.resert_8;
    array[7] = data.trainSet_flag;
    
    return array;
}
QByteArray mvb_send::structToByteArray(const RunInfoStruct& data)
{
    QByteArray array;
    array.resize(32);

    // array[0] = (data.lifeSignal >> 8) & 0xFF;
    // array[1] = data.lifeSignal & 0xFF;

    // array[2] = (data.flags >> 8) & 0xFF;
    // array[3] = data.flags & 0xFF;

    // array[4] = (data.railwayID >> 8) & 0xFF;
    // array[5] = data.railwayID & 0xFF;

    // array[6] = (data.endStationID >> 8) & 0xFF;
    // array[7] = data.endStationID & 0xFF;

    // array[8] = (data.nextStationID >> 8) & 0xFF;
    // array[9] = data.nextStationID & 0xFF;

    // array[10] = (data.currentStationID >> 8) & 0xFF;
    // array[11] = data.currentStationID & 0xFF;

    // array[12] = (data.targetDistance >> 8) & 0xFF;
    // array[13] = data.targetDistance & 0xFF;

    // array[14] = (data.startDistance >> 8) & 0xFF;
    // array[15] = data.startDistance & 0xFF;

    // array[16] = (data.trainLoad >> 8) & 0xFF;
    // array[17] = data.trainLoad & 0xFF;

    // array[18] = (data.limitSpeed >> 8) & 0xFF;
    // array[19] = data.limitSpeed & 0xFF;

    // array[20] = (data.netElectric >> 8) & 0xFF;
    // array[21] = data.netElectric & 0xFF;

    // array[22] = (data.netVoltage >> 8) & 0xFF;
    // array[23] = data.netVoltage & 0xFF;

    // array[24] = (data.Speed >> 8) & 0xFF;
    // array[25] = data.Speed & 0xFF;

    // array[26] = (data.tractionForce >> 8) & 0xFF;
    // array[27] = data.tractionForce & 0xFF;

    // array[28] = (data.ebrakeForce >> 8) & 0xFF;
    // array[29] = data.ebrakeForce & 0xFF;

    // array[30] = (data.airbrakeForce >> 8) & 0xFF;
    // array[31] = data.airbrakeForce & 0xFF;


    // 创建一个指向RunInfoStruct的指针，用于遍历所有字段
    const quint16* fields = reinterpret_cast<const quint16*>(&data);
    
    // 遍历所有字段，将其转换为大端序并写入数组
    for (int i = 0; i < 16; ++i) {
        quint16 fieldBE = qToBigEndian(fields[i]);
        memcpy(array.data() + i * 2, &fieldBE, sizeof(fieldBE));
    }

    return array;
}

QByteArray mvb_send::structToByteArray(const PosInfoStruct& data)
{
    QByteArray array;
    array.resize(32);

    // 使用指针操作将数据按大端序写入数组
    quint16* dest16 = reinterpret_cast<quint16*>(array.data());
    
    // 处理quint16类型的字段
    dest16[0] = qToBigEndian(data.lifeSignal);
    dest16[1] = qToBigEndian(data.flags);
    dest16[2] = qToBigEndian(data.resert_16_1);
    
    // 处理quint8类型的字段
    array[6] = data.railwayID;
    array[7] = data.resert_8;
    
    // 处理quint64类型的字段 (使用memcpy确保对齐安全)
    quint64 resert64_1BE = qToBigEndian(data.resert_64_1);
    memcpy(&array.data()[8], &resert64_1BE, sizeof(resert64_1BE));
    
    // 继续处理quint16类型的字段
    dest16[8] = qToBigEndian(data.targetDistance);
    dest16[9] = qToBigEndian(data.startDistance);
    dest16[10] = qToBigEndian(data.speed);
    
    // 处理第二个quint64类型的字段
    quint64 resert64_2BE = qToBigEndian(data.resert_64_2);
    memcpy(&array.data()[22], &resert64_2BE, sizeof(resert64_2BE));
    
    // 处理最后一个quint16类型的字段
    dest16[15] = qToBigEndian(data.resert_16_2);
    
    
    return array;
}


QByteArray mvb_send::structToByteArray(const RailwayInfoStruct& data)
{
    QByteArray array;
    array.resize(32);

    // array[0] = (data.resert_16_1 >> 8) & 0xFF;
    // array[1] = data.resert_16_1 & 0xFF;

    // array[2] = (data.netVoltage >> 8) & 0xFF;
    // array[3] = data.netVoltage & 0xFF;

    // array[4] = (data.netElectric >> 8) & 0xFF;
    // array[5] = data.netElectric & 0xFF;

    // array[6] = (data.resert_16_2 >> 8) & 0xFF;
    // array[7] = data.resert_16_2 & 0xFF;

    // array[8] = (data.endStationId >> 8) & 0xFF;
    // array[9] = data.endStationId & 0xFF;

    // array[10] = (data.currentStationId >> 8) & 0xFF;
    // array[11] = data.currentStationId & 0xFF;

    // array[12] = (data.nextStationId >> 8) & 0xFF;
    // array[13] = data.nextStationId & 0xFF;

    // array[14] = (data.speed >> 8) & 0xFF;
    // array[15] = data.speed & 0xFF;

    // array[16] = (data.limitSpeed >> 8) & 0xFF;
    // array[17] = data.limitSpeed & 0xFF;

    // array[18] = (data.resert_16_3 >> 8) & 0xFF;
    // array[19] = data.resert_16_3 & 0xFF;

    // array[20] = data.resert_8_1;
    // array[21] = data.flags1;
    // array[22] = data.resert_8_2;
    // array[23] = data.flags2;

    // array[24] = (data.resert_32 >> 24) & 0xFF;
    // array[25] = (data.resert_32 >> 16) & 0xFF;
    // array[26] = (data.resert_32 >> 8) & 0xFF;
    // array[27] = data.resert_32 & 0xFF;

    // array[28] = (data.tractionForce >> 8) & 0xFF;
    // array[29] = data.tractionForce & 0xFF;

    // array[30] = (data.ebrakeForce >> 8) & 0xFF;
    // array[31] = data.ebrakeForce & 0xFF;

    // 使用指针操作将数据按大端序写入数组
    quint16* dest16 = reinterpret_cast<quint16*>(array.data());
    
    // 处理quint16类型的字段
    dest16[0] = qToBigEndian(data.resert_16_1);
    dest16[1] = qToBigEndian(data.netVoltage);
    dest16[2] = qToBigEndian(data.netElectric);
    dest16[3] = qToBigEndian(data.resert_16_2);
    dest16[4] = qToBigEndian(data.endStationId);
    dest16[5] = qToBigEndian(data.currentStationId);
    dest16[6] = qToBigEndian(data.nextStationId);
    dest16[7] = qToBigEndian(data.speed);
    dest16[8] = qToBigEndian(data.limitSpeed);
    dest16[9] = qToBigEndian(data.resert_16_3);
    
    // 处理quint8类型的字段
    array[20] = data.resert_8_1;
    array[21] = data.flags1;
    array[22] = data.resert_8_2;
    array[23] = data.flags2;
    
    // 处理quint32类型的字段
    quint32 resert32BE = qToBigEndian(data.resert_32);
    memcpy(&array.data()[24], &resert32BE, sizeof(resert32BE));
    
    // 处理最后两个quint16类型的字段
    dest16[14] = qToBigEndian(data.tractionForce);
    dest16[15] = qToBigEndian(data.ebrakeForce);
    
    return array;
}

// QByteArray mvb_send::structToByteArray(const CarriageInfoStruct& data)
// {
//     QByteArray array;
//     array.resize(32);

//     array[0] = (data.resert_64_1 >> 56) & 0xFF;
//     array[1] = (data.resert_64_1 >> 48) & 0xFF;
//     array[2] = (data.resert_64_1 >> 40) & 0xFF;
//     array[3] = (data.resert_64_1 >> 32) & 0xFF;
//     array[4] = (data.resert_64_1 >> 24) & 0xFF;
//     array[5] = (data.resert_64_1 >> 16) & 0xFF;
//     array[6] = (data.resert_64_1 >> 8) & 0xFF;
//     array[7] = (data.resert_64_1) & 0xFF;

//     array[8] = data.flag1;
//     array[9] = data.resert_8_1;

//     array[10] = (data.load_A1 >> 8)&0xFF;
//     array[11] = (data.load_A1) & 0xFF;

//     array[12] = data.flag2;
//     array[13] = data.resert_8_2;

//     array[14] = (data.load_A2 >> 8)&0xFF;
//     array[15] = (data.load_A2) & 0xFF;

//     array[16] = data.flag3;
//     array[17] = data.resert_8_3;

//     array[18] = (data.load_B1 >> 8)&0xFF;
//     array[19] = (data.load_B1) & 0xFF;

//     array[20] = data.flag4;
//     array[21] = data.resert_8_4;

//     array[22] = (data.load_B2 >> 8)&0xFF;
//     array[23] = (data.load_B2) & 0xFF;

//     array[24] = (data.resert_64_2 >> 56) & 0xFF;
//     array[25] = (data.resert_64_2 >> 48) & 0xFF;
//     array[26] = (data.resert_64_2 >> 40) & 0xFF;
//     array[27] = (data.resert_64_2 >> 32) & 0xFF;
//     array[28] = (data.resert_64_2 >> 24) & 0xFF;
//     array[29] = (data.resert_64_2 >> 16) & 0xFF;
//     array[30] = (data.resert_64_2 >> 8) & 0xFF;
//     array[31] = (data.resert_64_2) & 0xFF;
    

//     return array;

// }

// QByteArray mvb_send::structToByteArray(const AirbrakePowerStruct& data)
// {
//     QByteArray array;
//     array.resize(32);

//     array[0] = (data.resert_64_1 >> 56) & 0xFF;
//     array[1] = (data.resert_64_1 >> 48) & 0xFF;
//     array[2] = (data.resert_64_1 >> 40) & 0xFF;
//     array[3] = (data.resert_64_1 >> 32) & 0xFF;
//     array[4] = (data.resert_64_1 >> 24) & 0xFF;
//     array[5] = (data.resert_64_1 >> 16) & 0xFF;
//     array[6] = (data.resert_64_1 >> 8) & 0xFF;
//     array[7] = (data.resert_64_1) & 0xFF;

//     array[8] = (data.resert_64_2 >> 56) & 0xFF;
//     array[9] = (data.resert_64_2 >> 48) & 0xFF;
//     array[10] = (data.resert_64_2 >> 40) & 0xFF;
//     array[11] = (data.resert_64_2 >> 32) & 0xFF;
//     array[12] = (data.resert_64_2 >> 24) & 0xFF;
//     array[13] = (data.resert_64_2 >> 16) & 0xFF;
//     array[14] = (data.resert_64_2 >> 8) & 0xFF;
//     array[15] = (data.resert_64_2) & 0xFF;

//     array[16] = (data.resert_32_1 >> 24) & 0xFF;
//     array[17] = (data.resert_32_1 >> 16) & 0xFF;
//     array[18] = (data.resert_32_1 >> 8) & 0xFF;
//     array[19] = data.resert_32_1 & 0xFF;

//     array[20] = (data.airbrakePower_A1 >> 8) & 0xFF;
//     array[21] = data.airbrakePower_A1 & 0xFF;
//     array[22] = (data.airbrakePower_A2 >> 8) & 0xFF;
//     array[23] = data.airbrakePower_A2 & 0xFF;
//     array[24] = (data.airbrakePower_B1 >> 8) & 0xFF;
//     array[25] = data.airbrakePower_B1 & 0xFF;
//     array[26] = (data.airbrakePower_B2 >> 8) & 0xFF;
//     array[27] = data.airbrakePower_B2 & 0xFF;

//     array[28] = (data.resert_32_2 >> 24) & 0xFF;
//     array[29] = (data.resert_32_2 >> 16) & 0xFF;
//     array[30] = (data.resert_32_2 >> 8) & 0xFF;
//     array[31] = data.resert_32_2 & 0xFF;

//     return array;
    
// }

QByteArray mvb_send::structToByteArray(const CarriageInfoStruct& data)
{
    QByteArray array;
    array.resize(32);
    
    // 使用指针操作将数据按大端序写入数组
    quint16* dest16 = reinterpret_cast<quint16*>(array.data());
    
    // 处理quint64类型的字段 (使用memcpy确保对齐安全)
    quint64 resert64_1BE = qToBigEndian(data.resert_64_1);
    memcpy(&array.data()[0], &resert64_1BE, sizeof(resert64_1BE));
    
    // 处理quint8类型的字段
    array[8] = data.flag1;
    array[9] = data.resert_8_1;
    
    // 处理quint16类型的字段
    dest16[5] = qToBigEndian(data.load_A1);
    
    // 继续处理quint8类型的字段
    array[12] = data.flag2;
    array[13] = data.resert_8_2;
    
    // 处理quint16类型的字段
    dest16[7] = qToBigEndian(data.load_A2);
    
    // 继续处理quint8类型的字段
    array[16] = data.flag3;
    array[17] = data.resert_8_3;
    
    // 处理quint16类型的字段
    dest16[9] = qToBigEndian(data.load_B1);
    
    // 继续处理quint8类型的字段
    array[20] = data.flag4;
    array[21] = data.resert_8_4;
    
    // 处理quint16类型的字段
    dest16[11] = qToBigEndian(data.load_B2);
    
    // 处理quint64类型的字段 (使用memcpy确保对齐安全)
    quint64 resert64_2BE = qToBigEndian(data.resert_64_2);
    memcpy(&array.data()[24], &resert64_2BE, sizeof(resert64_2BE));
    
    return array;
}

QByteArray mvb_send::structToByteArray(const AirbrakePowerStruct& data)
{
    QByteArray array;
    array.resize(32);
    
    // 使用指针操作将数据按大端序写入数组
    quint16* dest16 = reinterpret_cast<quint16*>(array.data());
    
    // 处理quint64类型的字段 (使用memcpy确保对齐安全)
    quint64 resert64_1BE = qToBigEndian(data.resert_64_1);
    memcpy(&array.data()[0], &resert64_1BE, sizeof(resert64_1BE));
    
    // 处理第二个quint64类型的字段
    quint64 resert64_2BE = qToBigEndian(data.resert_64_2);
    memcpy(&array.data()[8], &resert64_2BE, sizeof(resert64_2BE));
    
    // 处理quint32类型的字段
    quint32 resert32_1BE = qToBigEndian(data.resert_32_1);
    memcpy(&array.data()[16], &resert32_1BE, sizeof(resert32_1BE));
    
    // 处理quint16类型的字段
    dest16[10] = qToBigEndian(data.airbrakePower_A1);
    dest16[11] = qToBigEndian(data.airbrakePower_A2);
    dest16[12] = qToBigEndian(data.airbrakePower_B1);
    dest16[13] = qToBigEndian(data.airbrakePower_B2);
    
    // 处理最后一个quint32类型的字段
    quint32 resert32_2BE = qToBigEndian(data.resert_32_2);
    memcpy(&array.data()[28], &resert32_2BE, sizeof(resert32_2BE));
    
    return array;
}
QByteArray mvb_send::generateMvbPacket(QByteArray &data, PD_TYPE type)
{
    // 根据枚举值获取对应的端口地址
    quint16 portAddress = 0;
    switch (type) {
        case TIME_PORT_TYPE:
            portAddress = TIME_PORT;
            break;
        case TRAIN_PORT_1_TYPE:
            portAddress = TRAIN_PORT_1;
            break;
        case TRAIN_PORT_2_TYPE:
            portAddress = TRAIN_PORT_2;
            break;
        case TRAIN_PORT_3_TYPE:
            portAddress = TRAIN_PORT_3;
            break;
        case TRAIN_PORT_4_TYPE:
            portAddress = TRAIN_PORT_4;
            break;
        case RUNINFO_PORT_TYPE:
            portAddress = RUNINFO_PORT;
            break;
        case POSINFO_PORT_TYPE:
            portAddress = POSINFO_PORT;
            break;
        case RAILWAYINFO_PORT_TYPE:
            portAddress = RAILWAYINFO_PORT;
            break;
        case CARRIAGEINFO_PORT_1_TYPE:
            portAddress = CARRIAGEINFO_PORT_1;
            break;
        case CARRIAGEINFO_PORT_2_TYPE:
            portAddress = CARRIAGEINFO_PORT_2;
            break;
        case AIRBRAKEPOWER_PORT_1_TYPE:
            portAddress = AIRBRAKEPOWER_PORT_1;
            break;
        case AIRBRAKEPOWER_PORT_2_TYPE:
            portAddress = AIRBRAKEPOWER_PORT_2;
            break;
        default:
            // 如果类型不匹配，返回空数组
            return QByteArray();  
    }
    // 将端口地址转换为两个字节的数组（大端序）
    QByteArray portArray(2, 0);
    portArray[0] = (portAddress >> 8) & 0xFF;
    portArray[1] = portAddress & 0xFF;
    
    // 根据数据大小获取对应的FramSizeType枚举值作为dataSize
    quint8 dataSize = 0;
    switch (data.size()) {
        case 2:
            dataSize = BYTE_2;   // 0
            break;
        case 4:
            dataSize = BYTE_4;   // 1
            break;
        case 8:
            dataSize = BYTE_8;   // 2
            break;
        case 16:
            dataSize = BYTE_16;  // 3
            break;
        case 32:
            dataSize = BYTE_32;  // 4
            break;
        default:
            // 如果数据大小不匹配任何已知的帧大小，返回空数组
            return QByteArray();
    }
    QByteArray result;
    result.reserve(4 + data.size());
    
    result.append(0x21);
    result.append(dataSize);
    result.append(portArray);
    result.append(data);
    
    return result;
}

QByteArray mvb_send::appendCrc16(QByteArray &data)
{
    // 计算CRC16校验值
    quint16 crc = calculate_crc16_direct(reinterpret_cast<const quint8*>(data.data()), data.size());
    
    // 创建结果数组
    QByteArray result = data;
    
    // 将CRC值以小端序方式追加到数据末尾
    result.append(static_cast<char>(crc & 0xFF));        // 低字节
    result.append(static_cast<char>((crc >> 8) & 0xFF)); // 高字节
    
    return result;
}

QByteArray mvb_send::unescapeReceivedData(QByteArray &data)
{
    // 创建结果数组，预留足够的空间
    QByteArray result;
    result.reserve(data.size() * 2 + 2); // 最坏情况下大小翻倍，再加上首尾的0x7E
    
    // 添加起始字节0x7E
    result.append(0x7E);
    
    // 对数据进行转义处理
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data.at(i));
        switch (byte) {
            case 0x7E: // 0x7E转义为0x7D 0x5E
                result.append(0x7D);
                result.append(0x5E);
                break;
            case 0x7D: // 0x7D转义为0x7D 0x5D
                result.append(0x7D);
                result.append(0x5D);
                break;
            default: // 其他字符不转义
                result.append(byte);
                break;
        }
    }
    
    // 添加结束字节0x7E
    result.append(0x7E);
    
    return result;
}



