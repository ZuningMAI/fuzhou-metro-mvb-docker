#include "data_interpolator.h"
#include <QDebug>
#include <cmath>

TrajectoryPoint DataInterpolator::interpolate(const QVector<TrajectoryPoint> &trajectory,
                                              double currentTime)
{
    if (trajectory.isEmpty()) {
        qWarning() << "轨迹数据为空";
        return TrajectoryPoint();
    }
    
    // 如果时间在第一个点之前，返回第一个点
    if (currentTime <= trajectory.first().relativeTime) {
        return trajectory.first();
    }
    
    // 如果时间在最后一个点之后，返回最后一个点
    if (currentTime >= trajectory.last().relativeTime) {
        return trajectory.last();
    }
    
    // 查找时间区间
    int index = findTimeInterval(trajectory, currentTime);
    if (index < 0 || index >= trajectory.size() - 1) {
        qWarning() << "无法找到时间区间:" << currentTime;
        return trajectory.last();
    }
    
    const TrajectoryPoint &p0 = trajectory[index];
    const TrajectoryPoint &p1 = trajectory[index + 1];
    
    // 计算插值比例
    double t0 = p0.relativeTime;
    double t1 = p1.relativeTime;
    
    TrajectoryPoint result;
    result.relativeTime = currentTime;
    
    // 线性插值速度和位置
    result.speed = linearInterpolate(currentTime, t0, t1, p0.speed, p1.speed);
    result.position = linearInterpolate(currentTime, t0, t1, p0.position, p1.position);
    
    // 力和操纵工况维持前一个点的值
    result.force = p0.force;
    result.operationMode = p0.operationMode;
    
    return result;
}

TrajectoryPoint DataInterpolator::maintain(const QVector<TrajectoryPoint> &trajectory,
                                          double currentTime)
{
    if (trajectory.isEmpty()) {
        qWarning() << "轨迹数据为空";
        return TrajectoryPoint();
    }
    
    // 如果时间在第一个点之前，返回第一个点
    if (currentTime <= trajectory.first().relativeTime) {
        return trajectory.first();
    }
    
    // 如果时间在最后一个点之后，返回最后一个点
    if (currentTime >= trajectory.last().relativeTime) {
        return trajectory.last();
    }
    
    // 查找最接近且不大于currentTime的点
    for (int i = trajectory.size() - 1; i >= 0; --i) {
        if (trajectory[i].relativeTime <= currentTime) {
            return trajectory[i];
        }
    }
    
    // 理论上不会到达这里
    return trajectory.first();
}

int DataInterpolator::findTimeInterval(const QVector<TrajectoryPoint> &trajectory,
                                      double currentTime)
{
    // 二分查找(relativeTime是有序数据)
    int left = 0;
    int right = trajectory.size() - 1;
    
    while (left < right) {
        int mid = left + (right - left) / 2;
        
        if (trajectory[mid].relativeTime <= currentTime && 
            (mid == trajectory.size() - 1 || trajectory[mid + 1].relativeTime > currentTime)) {
            return mid;
        } else if (trajectory[mid].relativeTime > currentTime) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    
    return left;
}

double DataInterpolator::linearInterpolate(double t, double t0, double t1, double v0, double v1)
{
    if (std::abs(t1 - t0) < 1e-9) {
        return v0;
    }
    
    double ratio = (t - t0) / (t1 - t0);
    return v0 + ratio * (v1 - v0);
}

