#include "data_generator.h"
#include <QDateTime>
#include <QDebug>
#include <cmath>
#include <cstdlib>
#include <random>

DateTimeStruct DataGenerator::generateTimeData()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDate date = currentDateTime.date();
    QTime time = currentDateTime.time();
    
    DateTimeStruct dt(
        static_cast<quint8>(date.year() - 2000), // 年份偏移2000年
        static_cast<quint8>(date.month()),
        static_cast<quint8>(date.day()),
        static_cast<quint8>(time.hour()),
        static_cast<quint8>(time.minute()),
        static_cast<quint8>(time.second()),
        true,  // timeset
        true   // timeAvailable
    );
    
    return dt;
}

TrainStruct DataGenerator::generateTrainInfo(quint16 trainNum, quint32 trainInfo, bool trainSetFlag)
{
    TrainStruct train(trainNum, trainInfo, trainSetFlag);
    return train;
}

RunInfoStruct DataGenerator::generateRunInfo(
    const TrajectoryPoint &currentPoint,
    const SectionInfo &currentSection,
    int currentStationID,
    int nextStationID,
    int endStationID,
    double currentPosition,
    int limitSpeed,
    quint16 lifeSignal,
    bool isLastSection,
    double trainLoad)
{
    // 计算目标距离和起始距离
    quint16 targetDistance = 0;
    quint16 startDistance = 0;
    calculateDistances(currentPosition, currentSection.startPosition, 
                      currentSection.endPosition, targetDistance, startDistance);
    
    // 计算牵引力和制动力
    quint16 tractionForce = 0;
    quint16 ebrakeForce = 0;
    quint16 airbrakeForce = 0;
    calculateForces(currentPoint.force, currentPoint.operationMode,
                   tractionForce, ebrakeForce, airbrakeForce);
    
    // 生成随机网侧电压和电流
    quint16 netVoltage = 0;
    quint16 netCurrent = 0;
    generateRandomPower(netVoltage, netCurrent);
    
    // 转换单位
    quint16 trainLoadValue = static_cast<quint16>(trainLoad * 10);  // t -> 0.1t
    quint16 speedValue = static_cast<quint16>(currentPoint.speed * 100);  // km/h -> 0.01km/h
    quint16 limitSpeedValue = static_cast<quint16>(limitSpeed);  // km/h
    
    // 设置标志位
    bool endStationIDValid = !isLastSection || (currentPoint.speed > 0.1);
    bool nextStationIDValid = !isLastSection;
    bool currentStationIDValid = true;
    bool targetDistValid = currentPoint.speed > 0.1;
    bool startDistValid = currentPoint.speed > 0.1;
    bool atoMode = true;
    bool tmc1Active = true;
    bool tmc2Active = false;
    
    // 根据操纵工况设置标志
    bool traction = (currentPoint.operationMode == 0);
    bool coast = (currentPoint.operationMode == 2);
    bool brake = (currentPoint.operationMode == 3);
    
    // 载荷类型（当前只考虑AW0）
    bool loadAW0 = true;
    bool loadAW2 = false;
    bool loadAW3 = false;
    
    RunInfoStruct runInfo(
        lifeSignal,
        6,  // 线路ID
        endStationID,
        nextStationID,
        currentStationID,
        targetDistance,
        startDistance,
        trainLoadValue,
        limitSpeedValue,
        netCurrent,
        netVoltage,
        speedValue,
        tractionForce,
        ebrakeForce,
        airbrakeForce,
        endStationIDValid,
        nextStationIDValid,
        currentStationIDValid,
        targetDistValid,
        startDistValid,
        atoMode,
        tmc1Active,
        tmc2Active,
        coast,
        traction,
        brake,
        loadAW0,
        loadAW2,
        loadAW3
    );
    
    return runInfo;
}

void DataGenerator::calculateDistances(double currentPos, 
                                      double sectionStart, 
                                      double sectionEnd,
                                      quint16 &targetDist, 
                                      quint16 &startDist)
{
    // 目标距离 = 区间终点 - 当前位置
    double target = std::abs(sectionEnd - currentPos);
    targetDist = static_cast<quint16>(std::round(target));
    
    // 起始距离 = 当前位置 - 区间起点
    double start = std::abs(currentPos - sectionStart);
    startDist = static_cast<quint16>(std::round(start));
}

// void DataGenerator::calculateForces(double force, int mode,
//                                    quint16 &tractionForce,
//                                    quint16 &ebrakeForce,
//                                    quint16 &airbrakeForce)
// {
//     // 初始化
//     tractionForce = 0;
//     ebrakeForce = 0;
//     airbrakeForce = 0;
    
//     if (force > 0) {
//         // 牵引力
//         tractionForce = static_cast<quint16>(std::round(force * 10));  // kN -> 0.1kN
//     } else if (force < 0) {
//         // 制动力
//         double totalBrake = std::abs(force);
        
//         // 随机决定是否使用空气制动
//         // 50%概率只用电制动，50%概率电制动+空气制动
//         if (rand() % 2 == 0) {
//             // 只用电制动
//             ebrakeForce = static_cast<quint16>(std::round(totalBrake * 10));  // kN -> 0.1kN
//             airbrakeForce = 0;
//         } else {
//             // 电制动+空气制动
//             // 电制动占60%~80%
//             double ebrakeRatio = 0.6 + (rand() % 21) / 100.0;  // 0.6~0.8
//             double ebrakePart = totalBrake * ebrakeRatio;
//             double airbrakePart = totalBrake - ebrakePart;
            
//             ebrakeForce = static_cast<quint16>(std::round(ebrakePart * 10));
//             airbrakeForce = static_cast<quint16>(std::round(airbrakePart * 10));
//         }
//     }
//     // else force == 0，所有力都保持为0
// }

void DataGenerator::calculateForces(double force, int mode,
                                    quint16 &tractionForce,
                                    quint16 &ebrakeForce,
                                    quint16 &airbrakeForce)
{
    // 初始化
    tractionForce = 0;
    ebrakeForce = 0;
    airbrakeForce = 0;

    if (force > 0) {
        // 牵引力
        tractionForce = static_cast<quint16>(std::round(force * 10));  // kN -> 0.1kN
    } else if (force < 0) {
        // 制动力
        double totalBrake = std::abs(force);

        // 总是使用电制动+空气制动（100%概率）
        // 约束条件：b ≥ 0.8305a，即 airbrake ≥ 0.8305 * ebrake
        // 由于 a + b = totalBrake，可得：
        // b ≥ 0.8305a
        // totalBrake - a ≥ 0.8305a
        // totalBrake ≥ 1.8305a
        // a ≤ totalBrake / 1.8305

        // 生成0.7~1之间服从正态分布的随机数
        // 使用均值0.85，标准差0.1的正态分布，然后截断到[0.7, 1.0]范围
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::normal_distribution<double> dist(0.85, 0.1);

        double ratio = dist(gen);
        // 截断到[0.7, 1.0]范围
        if (ratio < 0.7) ratio = 0.7;
        if (ratio > 1.0) ratio = 1.0;

        // 选择电制动力 a 在合理范围内
        double a = totalBrake / 1.8305 * ratio;
        double b = totalBrake - a;

        // 验证约束条件
        if (b < 0.8305 * a) {
            // 如果不满足约束，调整为刚好满足约束的值
            a = totalBrake / (1.0 + 0.8305);
            b = totalBrake - a;
        }

        // 转换为协议单位
        ebrakeForce = static_cast<quint16>(std::round(a * 10));  // kN -> 0.1kN
        airbrakeForce = static_cast<quint16>(std::round(b * 10));  // kN -> 0.1kN
    }
    // else force == 0，所有力都保持为0
}

void DataGenerator::generateRandomPower(quint16 &voltage, quint16 &current)
{
    // 网侧电压：1000V ± 500V
    int voltageBase = 1000;
    int voltageVariation = (rand() % 1001) - 500;  // -50 ~ +50
    voltage = static_cast<quint16>(voltageBase + voltageVariation);
    
    // 网侧电流：100A ~ 500A（根据实际功率合理估算）
    int currentBase = 300;
    int currentVariation = (rand() % 201) - 100;  // -100 ~ +100
    int currentValue = currentBase + currentVariation;
    if (currentValue < 100) currentValue = 100;
    if (currentValue > 500) currentValue = 500;
    
    current = static_cast<quint16>(currentValue * 10);  // A -> 0.1A
}


PosInfoStruct DataGenerator::generatePosInfo(
    const TrajectoryPoint &currentPoint,
    const SectionInfo &currentSection,
    double currentPosition,
    quint16 lifeSignal,
    int railwayID)
{
    // 计算目标距离和起始距离
    quint16 targetDistance = 0;
    quint16 startDistance = 0;
    calculateDistances(currentPosition, currentSection.startPosition, 
                      currentSection.endPosition, targetDistance, startDistance);
    
    // 设置有效性标志
    bool endStationIdAvailable = (currentPoint.speed > 0.1);
    bool nextStationIdAvailable = (currentPoint.speed > 0.1);
    bool targetDistAvailable = (currentPoint.speed > 0.1);
    bool startDistAvailable = (currentPoint.speed > 0.1);
    bool currentStationIdAvailable = true;
    
    // 转换速度单位（km/h -> m/s，但协议要求的是km/h，这里保持原值）
    quint16 speed = static_cast<quint16>(currentPoint.speed);
    
    PosInfoStruct posInfo(
        lifeSignal,
        static_cast<quint8>(railwayID),
        targetDistance,
        startDistance,
        speed,
        endStationIdAvailable,
        nextStationIdAvailable,
        targetDistAvailable,
        startDistAvailable,
        currentStationIdAvailable
    );
    
    return posInfo;
}

RailwayInfoStruct DataGenerator::generateRailwayInfo(
    const RunInfoStruct &runInfo,
    const TrajectoryPoint &currentPoint)
{
    // 从RunInfoStruct提取数据
    quint16 netVoltage = runInfo.netVoltage;
    quint16 netElectric = runInfo.netElectric;
    quint16 endStationId = runInfo.endStationID;
    quint16 currentStationId = runInfo.currentStationID;
    quint16 nextStationId = runInfo.nextStationID;
    quint16 limitSpeed = runInfo.limitSpeed;
    
    // 计算列车总制动力（电制动力 + 空气制动力）
    // runInfo中的制动力单位是0.1kN，需要转换为10N单位
    // 0.1kN = 100N，所以需要乘以10
    quint16 totalBrakeForce = (runInfo.ebrakeForce + runInfo.airbrakeForce) * 10;
    
    // 牵引力也需要转换：0.1kN -> 10N
    quint16 tractionForce = runInfo.tractionForce * 10;
    
    // 速度转换：0.01km/h -> 0.1km/h
    quint16 speed = runInfo.Speed / 10;
    
    // 提取状态标志
    bool coast = (runInfo.flags & 0x0080) != 0;
    bool eBrake = (runInfo.flags & 0x0020) != 0;
    bool trac = (runInfo.flags & 0x0040) != 0;
    bool driverRoomA1 = (runInfo.flags & 0x0200) != 0;  // Tmc1司机室激活
    bool driverRoomA2 = (runInfo.flags & 0x0100) != 0;  // Tmc2司机室激活
    bool atoMode = (runInfo.flags & 0x0400) != 0;
    
    RailwayInfoStruct railwayInfo(
        netVoltage,
        netElectric,
        endStationId,
        currentStationId,
        nextStationId,
        speed,
        limitSpeed,
        tractionForce,
        totalBrakeForce,
        coast,
        eBrake,
        trac,
        driverRoomA2,
        driverRoomA1,
        atoMode
    );
    
    return railwayInfo;
}

CarriageInfoStruct DataGenerator::generateCarriageInfo(
    double trainLoad,
    bool isFirstUnit,
    bool airbrakeActive)
{
    // 列车总重 = 总AW0 + 总payload
    // 总AW0 = 50*2 (动车) + 40*2 (拖车) = 180吨
    const double TOTAL_AW0 = 180.0;
    double totalPayload = trainLoad - TOTAL_AW0;
    
    // 如果trainLoad小于AW0，使用默认值
    if (totalPayload < 0) {
        totalPayload = 36.0;  // 默认216吨 - 180吨 = 36吨
    }
    
    // 每个转向架的payload（均匀分配到8个转向架）
    double payloadPerBogie = totalPayload / 8.0;
    
    // 动车参数
    const double MOTOR_AW0_PER_CAR = 50.0;  // 每辆动车AW0
    const double MOTOR_INERTIA_RATIO = 0.10; // 动车转动惯量占10%
    
    // 拖车参数
    const double TRAILER_AW0_PER_CAR = 40.0; // 每辆拖车AW0
    const double TRAILER_INERTIA_RATIO = 0.05; // 拖车转动惯量占5%
    
    // 计算每个转向架的载荷
    // 动车转向架载荷 = AW0/2 + 转动惯量/2 + payload
    double motorBogieLoad = (MOTOR_AW0_PER_CAR / 2.0) + 
                           (MOTOR_AW0_PER_CAR * MOTOR_INERTIA_RATIO / 2.0) + 
                           payloadPerBogie;
    
    // 拖车转向架载荷 = AW0/2 + 转动惯量/2 + payload
    double trailerBogieLoad = (TRAILER_AW0_PER_CAR / 2.0) + 
                             (TRAILER_AW0_PER_CAR * TRAILER_INERTIA_RATIO / 2.0) + 
                             payloadPerBogie;
    
    // 转换为协议单位（1=0.01t）
    quint16 motorLoad = static_cast<quint16>(motorBogieLoad * 100);
    quint16 trailerLoad = static_cast<quint16>(trailerBogieLoad * 100);
    
    // 根据是第一还是第二动力单元设置载荷
    // 第一动力单元：A1(M) + B1(T)，即动车A1的两个转向架 + 拖车B1的两个转向架
    // 第二动力单元：B2(T) + A2(M)，即拖车B2的两个转向架 + 动车A2的两个转向架
    quint16 load_A, load_B;
    if (isFirstUnit) {
        // 第一动力单元：A1是动车，B1是拖车
        load_A = motorLoad;
        load_B = trailerLoad;
    } else {
        // 第二动力单元：B2是拖车，A2是动车
        load_A = trailerLoad;  // B2
        load_B = motorLoad;    // A2
    }
    
    // 设置标志位
    bool loadSigValid = true;
    bool airbrakeStatus = airbrakeActive;
    
    CarriageInfoStruct carriageInfo(
        load_A,  // A车架1载荷
        load_A,  // A车架2载荷
        load_B,  // B车架1载荷
        load_B,  // B车架2载荷
        airbrakeStatus, loadSigValid,  // A1标志
        airbrakeStatus, loadSigValid,  // A2标志
        airbrakeStatus, loadSigValid,  // B1标志
        airbrakeStatus, loadSigValid   // B2标志
    );
    
    return carriageInfo;
}

void DataGenerator::calculateBrakeDistribution(
    double totalBrakeForce,
    double ebrakeForce,
    double trainMass,
    quint16 &motorBogie1Air,
    quint16 &motorBogie2Air,
    quint16 &trailerBogie1Air,
    quint16 &trailerBogie2Air)
{
    // 制动力分配算法（基于载荷比例的电制动分配）

    // 列车参数（根据task.md）
    const double MOTOR_AW0 = 50.0;           // 动车AW0（吨）
    const double TRAILER_AW0 = 40.0;         // 拖车AW0（吨）
    const double MOTOR_INERTIA_RATIO = 0.10; // 动车转动惯量占10%
    const double TRAILER_INERTIA_RATIO = 0.05; // 拖车转动惯量占5%
    const double TOTAL_AW0 = 180.0;          // 总AW0 = 50*2 + 40*2

    // 计算总payload
    double totalPayload = trainMass - TOTAL_AW0;
    if (totalPayload < 0) {
        totalPayload = 36.0;  // 默认值
    }

    // 每个转向架的payload（均匀分配到8个转向架）
    double payloadPerBogie = totalPayload / 8.0;

    // 计算每个转向架的载荷（吨）
    // 动车转向架载荷 = AW0/2 + 转动惯量/2 + payload
    double m_i = (MOTOR_AW0 / 2.0) + (MOTOR_AW0 * MOTOR_INERTIA_RATIO / 2.0) + payloadPerBogie;

    // 拖车转向架载荷 = AW0/2 + 转动惯量/2 + payload
    double m_j = (TRAILER_AW0 / 2.0) + (TRAILER_AW0 * TRAILER_INERTIA_RATIO / 2.0) + payloadPerBogie;

    // 动车总质量（4个动车转向架）
    double M_M = 4.0 * m_i;

    // 拖车总质量（4个拖车转向架）
    double M_T = 4.0 * m_j;

    // 列车总质量
    double M_total = M_M + M_T;

    // 电制动力和空气制动力
    double a = ebrakeForce;  // 电制动力（kN）
    double b = totalBrakeForce - ebrakeForce;  // 空气制动力（kN）

    // 步骤1：计算列车总减速度（m/s²）
    double a_dec = (a + b) / M_total;

    // 步骤2：计算每个动车转向架的电制动力（按载荷比例分配）
    // F_e,i = a × (m_i / M_M)
    double F_e_motor = a * (m_i / M_M);

    // 步骤3：计算动车转向架的气制动力
    // F_air,i = max(0, m_i × a_dec - F_e,i)
    double F_air_motor = std::max(0.0, m_i * a_dec - F_e_motor);

    // 步骤4：计算拖车转向架的气制动力
    // F_air,j = m_j × a_dec
    double F_air_trailer = m_j * a_dec;

    // 转换为协议单位（1=10N，即kN*100）
    motorBogie1Air = static_cast<quint16>(std::round(F_air_motor * 100));
    motorBogie2Air = static_cast<quint16>(std::round(F_air_motor * 100));
    trailerBogie1Air = static_cast<quint16>(std::round(F_air_trailer * 100));
    trailerBogie2Air = static_cast<quint16>(std::round(F_air_trailer * 100));

    // 确保数值在有效范围内（0~200kN = 0~20000）
    motorBogie1Air = std::min(motorBogie1Air, static_cast<quint16>(20000));
    motorBogie2Air = std::min(motorBogie2Air, static_cast<quint16>(20000));
    trailerBogie1Air = std::min(trailerBogie1Air, static_cast<quint16>(20000));
    trailerBogie2Air = std::min(trailerBogie2Air, static_cast<quint16>(20000));
}

AirbrakePowerStruct DataGenerator::generateAirbrakePower(
    double totalBrakeForce,
    double ebrakeForce,
    double trainMass,
    bool isFirstUnit)
{
    quint16 motorBogie1Air, motorBogie2Air, trailerBogie1Air, trailerBogie2Air;

    // 调用制动力分配算法
    calculateBrakeDistribution(
        totalBrakeForce,
        ebrakeForce,
        trainMass,
        motorBogie1Air,
        motorBogie2Air,
        trailerBogie1Air,
        trailerBogie2Air
        );

    // 根据是第一还是第二动力单元设置气制动能力
    // 第一动力单元：A1(M) + B1(T)
    // 第二动力单元：B2(T) + A2(M)
    quint16 airPower_A1, airPower_A2, airPower_B1, airPower_B2;

    if (isFirstUnit) {
        // 第一动力单元：A1是动车，B1是拖车
        airPower_A1 = motorBogie1Air;
        airPower_A2 = motorBogie2Air;
        airPower_B1 = trailerBogie1Air;
        airPower_B2 = trailerBogie2Air;
    } else {
        // 第二动力单元：B2是拖车，A2是动车
        airPower_A1 = trailerBogie1Air;  // B2车架1
        airPower_A2 = trailerBogie2Air;  // B2车架2
        airPower_B1 = motorBogie1Air;    // A2车架1
        airPower_B2 = motorBogie2Air;    // A2车架2
    }

    AirbrakePowerStruct airbrakePower(
        airPower_A1,
        airPower_A2,
        airPower_B1,
        airPower_B2
        );

    return airbrakePower;
}

