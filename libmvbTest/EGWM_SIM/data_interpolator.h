#ifndef DATA_INTERPOLATOR_H
#define DATA_INTERPOLATOR_H

#include "simulator_config.h"
#include <QVector>

class DataInterpolator {
public:
    // 获取指定时间的状态（插值模式）
    // 对速度、位置进行线性插值，力和操纵工况维持前一个点的值
    static TrajectoryPoint interpolate(const QVector<TrajectoryPoint> &trajectory,
                                      double currentTime);
    
    // 获取指定时间的状态（维持模式）
    // 返回时间最接近且不大于currentTime的点
    static TrajectoryPoint maintain(const QVector<TrajectoryPoint> &trajectory,
                                   double currentTime);
    
private:
    // 查找时间区间：找到trajectory[i].time <= currentTime < trajectory[i+1].time
    // 返回索引i，如果找不到则返回-1
    static int findTimeInterval(const QVector<TrajectoryPoint> &trajectory,
                               double currentTime);
    
    // 线性插值
    static double linearInterpolate(double t, double t0, double t1, double v0, double v1);
};

#endif // DATA_INTERPOLATOR_H

