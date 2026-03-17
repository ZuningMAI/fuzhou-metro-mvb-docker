#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include "simulator_config.h"
#include "dataDef.h"
#include <QDateTime>

class DataGenerator {
public:
    // 生成时间数据 (PD 0xFF)
    static DateTimeStruct generateTimeData();
    
    // 生成车号数据 (PD 0xF1~0xF4)
    static TrainStruct generateTrainInfo(quint16 trainNum, quint32 trainInfo, bool trainSetFlag = true);
    
    // 生成运行状态数据 (PD 0xA0)
    static RunInfoStruct generateRunInfo(
        const TrajectoryPoint &currentPoint,
        const SectionInfo &currentSection,
        int currentStationID,
        int nextStationID,
        int endStationID,
        double currentPosition,
        int limitSpeed,
        quint16 lifeSignal,
        bool isLastSection,
        double trainLoad = 216.0  // 默认载荷216吨
    );
    
    // 生成位置信息数据 (PD 0x80)
    static PosInfoStruct generatePosInfo(
        const TrajectoryPoint &currentPoint,
        const SectionInfo &currentSection,
        double currentPosition,
        quint16 lifeSignal,
        int railwayID = 6
    );
    
    // 生成线路信息数据 (PD 0x01)
    static RailwayInfoStruct generateRailwayInfo(
        const RunInfoStruct &runInfo,
        const TrajectoryPoint &currentPoint
    );
    
    // 生成车架载荷数据 (PD 0x0D, 0x0E)
    static CarriageInfoStruct generateCarriageInfo(
        double trainLoad,           // 列车总载荷（吨）
        bool isFirstUnit,          // true=第一动力单元(M1+T1), false=第二动力单元(T2+M2)
        bool airbrakeActive = true // 气制动是否激活
    );
    
    // 生成气制动能力数据 (PD 0x0F, 0x10)
    static AirbrakePowerStruct generateAirbrakePower(
        double totalBrakeForce,    // 总制动力（kN）
        double ebrakeForce,        // 电制动力（kN）
        double trainMass,          // 列车总质量（吨）
        bool isFirstUnit           // true=第一动力单元, false=第二动力单元
    );
    
private:
    // 计算目标距离和起始距离
    static void calculateDistances(double currentPos, 
                                   double sectionStart, 
                                   double sectionEnd,
                                   quint16 &targetDist, 
                                   quint16 &startDist);
    
    // 根据力和工况计算牵引力和制动力
    static void calculateForces(double force, int mode,
                               quint16 &tractionForce,
                               quint16 &ebrakeForce,
                               quint16 &airbrakeForce);
    
    // 生成随机网侧电压和电流
    static void generateRandomPower(quint16 &voltage, quint16 &current);
    
    // 计算制动力分配
    static void calculateBrakeDistribution(
        double totalBrakeForce,    // 总制动力（kN）
        double ebrakeForce,        // 电制动力（kN）
        double trainMass,          // 列车总质量（吨）
        quint16 &motorBogie1Air,   // 动车转向架1气制动力（10N）
        quint16 &motorBogie2Air,   // 动车转向架2气制动力（10N）
        quint16 &trailerBogie1Air, // 拖车转向架1气制动力（10N）
        quint16 &trailerBogie2Air  // 拖车转向架2气制动力（10N）
    );
};

#endif // DATA_GENERATOR_H

