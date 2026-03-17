#ifndef SPEED_LIMITER_H
#define SPEED_LIMITER_H

#include "simulator_config.h"
#include <QVector>

class SpeedLimiter {
public:
    SpeedLimiter();
    SpeedLimiter(const QVector<SpeedLimitEntry> &limits);
    
    // 根据位置获取限速值
    int getSpeedLimit(double position) const;
    
    // 加载FZ601限速配置
    static QVector<SpeedLimitEntry> loadFZ601Config();
    
    // 加载FZ602限速配置
    static QVector<SpeedLimitEntry> loadFZ602Config();
    
    // 设置限速配置
    void setSpeedLimits(const QVector<SpeedLimitEntry> &limits);
    
private:
    QVector<SpeedLimitEntry> m_speedLimits;
};

#endif // SPEED_LIMITER_H

