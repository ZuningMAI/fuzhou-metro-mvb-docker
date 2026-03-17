#ifndef DATADEF_H
#define DATADEF_H

#include <qendian.h>
#include <QDebug>

#define RESERT_8   0x00
#define RESERT_16  0x0000
#define RESERT_32  0x00000000
#define RESERT_64  0x0000000000000000

// pd端口地址
#define TIME_PORT               0xFF
#define TRAIN_PORT_1            0x0F1
#define TRAIN_PORT_2            0x0F2
#define TRAIN_PORT_3            0x0F3
#define TRAIN_PORT_4            0x0F4
#define RUNINFO_PORT            0xA0
#define POSINFO_PORT            0x80
#define RAILWAYINFO_PORT        0x01
#define CARRIAGEINFO_PORT_1     0x0D
#define CARRIAGEINFO_PORT_2     0x0E
#define AIRBRAKEPOWER_PORT_1    0x0F
#define AIRBRAKEPOWER_PORT_2    0x10

// PD端口类型枚举
enum PD_TYPE {
    TIME_PORT_TYPE = 1,
    TRAIN_PORT_1_TYPE,
    TRAIN_PORT_2_TYPE,
    TRAIN_PORT_3_TYPE,
    TRAIN_PORT_4_TYPE,
    RUNINFO_PORT_TYPE,
    POSINFO_PORT_TYPE,
    RAILWAYINFO_PORT_TYPE,
    CARRIAGEINFO_PORT_1_TYPE,
    CARRIAGEINFO_PORT_2_TYPE,
    AIRBRAKEPOWER_PORT_1_TYPE,
    AIRBRAKEPOWER_PORT_2_TYPE
};
// 数据大小类型
enum FramSizeType
{
    BYTE_2   = 0,
    BYTE_4   = 1,
    BYTE_8   = 2,
    BYTE_16  = 3,
    BYTE_32  = 4,
};

// 时间结构体
struct DateTimeStruct
{
    quint8  year;        // 年（例如：2025）
    quint8  month;       // 月（1~12）
    quint8  day;         // 日（1~31）
    quint8  hour;        // 小时（0~23）
    quint8  minute;      // 分钟（0~59）
    quint8  second;      // 秒（0~59）
    quint8 resert_8;
    quint8 timeset_avi_flag;


    // 无参构造
    DateTimeStruct()
    {
        memset(this, 0, sizeof(*this));
    }

    // 有参构造
    DateTimeStruct(quint8 y, quint8 mon, quint8 d, quint8 h, quint8 min, quint8 sec
    ,bool timeset,bool timeAvailable) 
        : year(y), month(mon), day(d), hour(h), minute(min), second(sec), resert_8(RESERT_8), 
        timeset_avi_flag((timeset?(1<<1):0)|
                         (timeAvailable?(1<<0):0)) {}

     void print()
    {
        qDebug() << "----------- 当前时间信息 -----------";

        QString time = QString("%1-%2-%3 %4:%5:%6").arg(year).arg(month).arg(day).arg(hour).arg(minute).arg(second);
        qDebug()<<"当前时间:"<<time;
        qDebug()<<"时间设置标志位:"<<((timeset_avi_flag & 0x02)?true:false);
        qDebug()<<"时间有效标志位:"<<((timeset_avi_flag & 0x01)?true:false);

        qDebug() << "----------------------------------";
    }
};

// 列车结构体
struct TrainStruct
{
    quint16   trainNum;
    quint32   trainInfo;
    quint8    resert_8;
    quint8    trainSet_flag;

    TrainStruct()
    {
        memset(this, 0, sizeof(*this));
    }
    
    TrainStruct(quint16 num, quint32 info,
    bool trainSet)
        :trainNum(num), trainInfo(info), resert_8(RESERT_8), 
        trainSet_flag((trainSet? (1<<0):0)){}
    void print()
    {
        qDebug() << "----------- 列车车辆信息 -----------";

        QString trainNumStr = QString("列车号:%1").arg(trainNum,4,10,QChar('0'));
        QString trainInfoStr = QString("列车信息:%2").arg(trainInfo,6,10,QChar('0'));
        qDebug() << trainNumStr << trainInfoStr;
        qDebug() << "列车号设置有效标志:"<<((trainSet_flag & 0x01)?true:false);

        qDebug() << "----------------------------------";
    }
};

// 运行时信息
struct RunInfoStruct
{
    quint16     lifeSignal;                     // 生命信号
    quint16     flags;
    quint16     railwayID;                       // 线路ID
    quint16     endStationID;                   // 终点站ID
    quint16     nextStationID;                  // 下一站ID
    quint16     currentStationID;               // 当前站ID
    quint16     targetDistance;                 // 目标距离
    quint16     startDistance;                  // 起始距离
    quint16     trainLoad;                      // 列车载荷(1=0.1t)
    quint16     limitSpeed;                     // 车辆限速值(1=1km/h)
    quint16     netElectric;                    // 网侧电流(1=0.1A)
    quint16     netVoltage;                     // 网侧电压(1=1V)
    quint16     Speed;                          // 列车速度(1=0.01km/h)
    quint16     tractionForce;                  // 列车牵引力(1=0.1kN)
    quint16     ebrakeForce;                    // 列车制动力(1=0.1kN)
    quint16     airbrakeForce;                  // 列车空气制动力(1=0.1kN)

    RunInfoStruct()
    {
        memset(this, 0, sizeof(*this));
    }
    RunInfoStruct(
        quint16 lifeSig,quint16 railID, quint16 endStation, quint16 nextStation, quint16 currentStation,
        quint16 targetDist, quint16 startDist, quint16 trainLoadVal, quint16 limitSpd,
        quint16 netElec, quint16 netVolt, quint16 spd, quint16 tracForce, quint16 ebrakeForce, quint16 airBrakeForce,
        bool endStationIDAvaliable,bool nextStationIDAvaliable,bool currentStationIDAvaliable,
        bool targetDistanceAvaliable,bool startDistanceAvaliable,bool ATOMode,
        bool TmcActivity_1,bool TmcActivity_2,bool coast,bool traction,bool ebrake,
        bool loadAW_0,bool loadAW_2,bool loadAW_3)
        : lifeSignal(lifeSig),
          flags((endStationIDAvaliable?(1<<15):0)|
                (nextStationIDAvaliable?(1<<14):0)|
                (currentStationIDAvaliable?(1<<13):0)|
                (targetDistanceAvaliable?(1<<12):0)|
                (startDistanceAvaliable?(1<<11):0)|
                (ATOMode?(1<<10):0)|
                (TmcActivity_1?(1<<9):0)|
                (TmcActivity_2?(1<<8):0)|
                (coast?(1<<7):0)|
                (traction?(1<<6):0)|
                (ebrake?(1<<5):0)|
                (loadAW_0?(1<<4):0)|
                (loadAW_2?(1<<3):0)|
                (loadAW_3?(1<<2):0)),
          railwayID(railID),
          endStationID(endStation),
          nextStationID(nextStation),
          currentStationID(currentStation),
          targetDistance(targetDist),
          startDistance(startDist),
          trainLoad(trainLoadVal),
          limitSpeed(limitSpd),
          netElectric(netElec),
          netVoltage(netVolt),
          Speed(spd),
          tractionForce(tracForce),
          ebrakeForce(ebrakeForce),
          airbrakeForce(airBrakeForce) {}

    void print()
    {
        qDebug() << "----------- 运行时信息 -----------";

        qDebug() << "生命信号:" << lifeSignal;
        // qDebug() << "flags:" << flags;
        qDebug() << "终点站ID有效标志:" << ((flags & 0x8000)?true:false);
        qDebug() << "下一站ID有效标志:" << ((flags & 0x4000)?true:false);
        qDebug() << "当前站ID有效标志:" << ((flags & 0x2000)?true:false);
        qDebug() << "目标距离有效标志:" << ((flags & 0x1000)?true:false);
        qDebug() << "起始距离有效标志:" << ((flags & 0x0800)?true:false);
        qDebug() << "ATO模式激活标志:" << ((flags & 0x0400)?true:false);
        qDebug() << "Tmc1司机室激活标志:" << ((flags & 0x0200)?true:false);
        qDebug() << "Tmc2司机室激活标志:" << ((flags & 0x0100)?true:false);
        qDebug() << "惰行标志:" << ((flags & 0x0080)?true:false);
        qDebug() << "牵引标志:" << ((flags & 0x0040)?true:false);
        qDebug() << "制动标志:" << ((flags & 0x0020)?true:false);
        qDebug() << "载荷AW0标志:" << ((flags & 0x0010)?true:false);
        qDebug() << "载荷AW2标志:" << ((flags & 0x0008)?true:false);
        qDebug() << "载荷AW3标志:" << ((flags & 0x0004)?true:false);
        qDebug() << "线路ID:" << railwayID;
        qDebug() << "终点站ID:" << endStationID;
        qDebug() << "下一站ID:" << nextStationID;
        qDebug() << "当前站ID:" << currentStationID;
        qDebug() << "目标距离:" << targetDistance;
        qDebug() << "起始距离:" << startDistance;
        qDebug() << "列车载荷:" << trainLoad*0.1 << " t";
        qDebug() << "车辆限速值:" << limitSpeed << " km/h";
        qDebug() << "网侧电流:" << netElectric*0.1 << " A";
        qDebug() << "网侧电压:" << netVoltage << " V";
        qDebug() << "列车速度:" << Speed*0.01 << " km/h";
        qDebug() << "牵引力:" << tractionForce*0.1 << " kN";
        qDebug() << "电制动力:" << ebrakeForce*0.1 << " kN";
        qDebug() << "空气制动力:" << airbrakeForce*0.1 << " kN";

        qDebug() << "----------------------------------";
    }
};

// 位置信息
struct PosInfoStruct
{
    quint16     lifeSignal;                     // 生命信号
    quint16     flags;
    quint16     resert_16_1;
    quint8      railwayID;                      // 线路ID
    quint8      resert_8;
    quint64     resert_64_1;
    quint16     targetDistance;                 // 目标距离
    quint16     startDistance;                  // 起始距离
    quint16     speed;                          // 当前速度
    quint64     resert_64_2;
    quint16     resert_16_2;

    PosInfoStruct()
    {
        memset(this, 0, sizeof(*this));
    }
    PosInfoStruct(
        quint16 lifeSig,
        quint8 railID, quint16 targetDist, 
        quint16 startDist, quint16 spd,
        bool endStationIdAvailable,bool nextStationIdAvailable,
        bool targetDistAvailable,bool startDistAvailable,
        bool currentStationIdAvailable)
        : lifeSignal(lifeSig),
          flags((endStationIdAvailable ? (1 << 13) : 0) | 
                (nextStationIdAvailable ? (1 << 12) : 0) | 
                (targetDistAvailable ? (1 << 9) : 0) | 
                (startDistAvailable ? (1 << 8) : 0) | 
                (currentStationIdAvailable ? (1 << 7) : 0)),
          resert_16_1(RESERT_16),
          railwayID(railID),
          resert_8(RESERT_8),
          resert_64_1(RESERT_64),
          targetDistance(targetDist),
          startDistance(startDist),
          speed(spd), 
          resert_64_2(RESERT_64),
          resert_16_2(RESERT_16)
          {}
    void print()
    {
        qDebug() << "------------- 位置信息 -------------";

        qDebug() << "生命信号:" << lifeSignal;
        // qDebug() << "flags:" << flags;
        qDebug() << "终点站ID有效标志:" << ((flags & 0x2000)?true:false);
        qDebug() << "下一站ID有效标志:" << ((flags & 0x1000)?true:false);
        qDebug() << "目标距离有效标志:" << ((flags & 0x0200)?true:false);
        qDebug() << "起始距离有效标志:" << ((flags & 0x0100)?true:false);
        qDebug() << "当前站ID有效标志:" << ((flags & 0x0080)?true:false);
        qDebug() << "线路ID" << railwayID;
        qDebug() << "目标距离" << targetDistance;
        qDebug() << "起始距离" << startDistance;
        qDebug() << "当前速度" << speed;

        qDebug() << "----------------------------------";
    }
};

// 线路信息
struct RailwayInfoStruct
{
    quint16     resert_16_1;
    quint16     netVoltage;                 // 网侧电压 (1=1V)
    quint16     netElectric;                // 网侧电流 (1=1A)
    quint16     resert_16_2;
    quint16     endStationId;               // 终点站ID
    quint16     currentStationId;           // 当前站ID
    quint16     nextStationId;              // 下一站ID
    quint16     speed;                      // 列车综合速度 (1=0.1km/h)
    quint16     limitSpeed;                 // 限速值 (1=1km/h)
    quint16     resert_16_3;
    quint8      resert_8_1;
    quint8      flags1;
    quint8      resert_8_2;
    quint8      flags2;
    quint32     resert_32;
    quint16     tractionForce;              // 列车总牵引力 (1=10N)
    quint16     ebrakeForce;                // 列车总电制动力 (1=10N)


    RailwayInfoStruct()
    {
        memset(this, 0, sizeof(*this));
    }
    RailwayInfoStruct(
        quint16 voltage, quint16 electric, quint16 endId, quint16 currentId, quint16 nextId,
        quint16 spd, quint16 limitSpd, quint16 tracForce, quint16 eBrakeForce,
        bool coast, bool eBrake, bool trac,
        bool driverRoomA2, bool driverRoomA1, bool atoMode)
        : resert_16_1(RESERT_16),
          netVoltage(voltage),
          netElectric(electric),
          resert_16_2(RESERT_16),
          endStationId(endId),
          currentStationId(currentId),
          nextStationId(nextId),
          speed(spd),
          limitSpeed(limitSpd),
          resert_16_3(RESERT_16),
          resert_8_1(RESERT_8),
          flags1((coast ? (1<<6) : 0) | 
                 (eBrake ? (1<<5) : 0) | 
                 (trac ? (1<<4) : 0) |
                 (driverRoomA2 ? (1<<1) : 0) |
                 (driverRoomA1 ? (1<<0) : 0)),
          resert_8_2(RESERT_8),
          flags2((atoMode ? (1<<1) : 0)),
          resert_32(RESERT_32),
          tractionForce(tracForce),
          ebrakeForce(eBrakeForce) {}

    void print()
    {
        qDebug() << "------------- 线路信息 -------------";

        qDebug() << "电网电流:" << netElectric << " A";
        qDebug() << "电网电压:" << netVoltage << " V";
        qDebug() << "终点站ID:" << endStationId;
        qDebug() << "当前站ID:" << currentStationId;
        qDebug() << "下一站ID:" << nextStationId;
        qDebug() << "列车综合速度:" << speed*0.1 << " km/h";
        qDebug() << "限速值:" << limitSpeed << " km/h";
        qDebug() << "惰行:" << ((flags1 & 0x40) ? true : false);
        qDebug() << "制动:" << ((flags1 & 0x20) ? true : false);
        qDebug() << "牵引:" << ((flags1 & 0x10) ? true : false);
        qDebug() << "A1司机室占有:" << ((flags1 & 0x01) ? true : false);
        qDebug() << "A2司机室占有:" << ((flags1 & 0x02) ? true : false);
        qDebug() << "ATO模式:" << ((flags2 & 0x02) ? true : false);
        qDebug() << "列车总牵引力:" << tractionForce/10 << " kN";
        qDebug() << "列车总制动力:" << ebrakeForce/10 << " kN";

        qDebug() << "----------------------------------";
    }
};

// 车架状态信息
struct CarriageInfoStruct
{

    quint64 resert_64_1; 
    quint8 flag1;
    quint8 resert_8_1;
    quint16 load_A1;                // 车架载荷 A1 (1=0.01t)
    quint8 flag2;
    quint8 resert_8_2;
    quint16 load_A2;                // 车架载荷 A2 (1=0.01t)
    quint8 flag3;
    quint8 resert_8_3;
    quint16 load_B1;                // 车架载荷 B1 (1=0.01t)
    quint8 flag4;
    quint8 resert_8_4;
    quint16 load_B2;                // 车架载荷 B2 (1=0.01t)
    quint64 resert_64_2; 

    CarriageInfoStruct()
    {
        memset(this, 0, sizeof(*this));
    }

    CarriageInfoStruct(
    quint16 loadVal_A1, quint16 loadVal_A2, quint16 loadVal_B1, quint16 loadVal_B2
    ,bool airbrake_A1,bool loadSig_A1,bool airbrake_A2,bool loadSig_A2
    ,bool airbrake_B1,bool loadSig_B1,bool airbrake_B2,bool loadSig_B2)
    : resert_64_1(RESERT_64),flag1((airbrake_A1?(1<<1):0)|(loadSig_A1?(1<<0):0)),
    resert_8_1(RESERT_8),load_A1(loadVal_A1),
    flag2((airbrake_A2?(1<<1):0)|(loadSig_A2?(1<<0):0)),
    resert_8_2(RESERT_8),load_A2(loadVal_A2),
    flag3((airbrake_B1?(1<<1):0)|(loadSig_B1?(1<<0):0)),
    resert_8_3(RESERT_8),load_B1(loadVal_B1),
    flag4((airbrake_B2?(1<<1):0)|(loadSig_B2?(1<<0):0)),
    resert_8_4(RESERT_8),load_B2(loadVal_B2),resert_64_2(RESERT_64){}

    void print()
    {
        qDebug() << "----------- 车架状态信息 -----------";

        qDebug() << "车架A1\t气制动状态:" << ((flag1 && 0x02)?true:false) << "\t载荷信号有效:" << ((flag1 && 0x01)?true:false) << "\t载荷:" << load_A1*0.01 << " t";
        qDebug() << "车架A2\t气制动状态:" << ((flag2 && 0x02)?true:false) << "\t载荷信号有效:" << ((flag2 && 0x01)?true:false) << "\t载荷:" << load_A2*0.01 << " t";
        qDebug() << "车架B1\t气制动状态:" << ((flag3 && 0x02)?true:false) << "\t载荷信号有效:" << ((flag3 && 0x01)?true:false) << "\t载荷:" << load_B1*0.01 << " t";
        qDebug() << "车架B2\t气制动状态:" << ((flag4 && 0x02)?true:false) << "\t载荷信号有效:" << ((flag4 && 0x01)?true:false) << "\t载荷:" << load_B2*0.01 << " t";

        qDebug() << "----------------------------------";
    }
};

// 气制动能力
struct AirbrakePowerStruct
{
    quint64 resert_64_1;
    quint64 resert_64_2;
    quint32 resert_32_1;
    quint16 airbrakePower_A1;       // 车架气制动能力_A1 (1=10N)
    quint16 airbrakePower_A2;       // 车架气制动能力_A2 (1=10N)
    quint16 airbrakePower_B1;       // 车架气制动能力_B1 (1=10N)
    quint16 airbrakePower_B2;       // 车架气制动能力_B2 (1=10N)
    quint32 resert_32_2;

    AirbrakePowerStruct()
    {
        memset(this, 0, sizeof(*this));
    }
    // AirbrakePowerStruct有参构造
    AirbrakePowerStruct(quint16 airbrakePower_a1, quint16 airbrakePower_a2, quint16 airbrakePower_b1, quint16 airbrakePower_b2)
        :resert_64_1(RESERT_64),resert_64_2(RESERT_64),resert_32_1(RESERT_32),
        airbrakePower_A1(airbrakePower_a1),
            airbrakePower_A2(airbrakePower_a2),
            airbrakePower_B1(airbrakePower_b1),
            airbrakePower_B2(airbrakePower_b2),resert_32_2(RESERT_32){}



    void print()
    {
        qDebug() << "---------- 气制动能力信息 ----------";

        qDebug() << "车架A1气制动能力:" << airbrakePower_A1*10 << " N";
        qDebug() << "车架A2气制动能力:" << airbrakePower_A2*10 << " N";
        qDebug() << "车架B1气制动能力:" << airbrakePower_B1*10 << " N";
        qDebug() << "车架B2气制动能力:" << airbrakePower_B2*10 << " N";

        qDebug() << "----------------------------------";
    }
};

#endif // DATADEF_H
