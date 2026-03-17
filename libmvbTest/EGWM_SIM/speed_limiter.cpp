#include "speed_limiter.h"
#include <QDebug>
#include <cmath>

SpeedLimiter::SpeedLimiter()
{
}

SpeedLimiter::SpeedLimiter(const QVector<SpeedLimitEntry> &limits)
    : m_speedLimits(limits)
{
}

int SpeedLimiter::getSpeedLimit(double position) const
{
    if (m_speedLimits.isEmpty()) {
        qWarning() << "限速配置为空";
        return 80;  // 默认限速
    }
    
    // 遍历限速配置，找到包含当前位置的区间
    for (const SpeedLimitEntry &entry : m_speedLimits) {
        // 处理正向区间 (start < end)
        if (entry.start < entry.end) {
            if (position >= entry.start && position <= entry.end) {
                return entry.limit;
            }
        }
        // 处理反向区间 (start > end)，如FZ601
        else if (entry.start > entry.end) {
            if (position <= entry.start && position >= entry.end) {
                return entry.limit;
            }
        }
    }
    
    // 如果没有找到，返回最后一个限速值或默认值
    if (!m_speedLimits.isEmpty()) {
        return m_speedLimits.last().limit;
    }
    
    qWarning() << "未找到位置" << position << "的限速配置";
    return 80;  // 默认限速
}

QVector<SpeedLimitEntry> SpeedLimiter::loadFZ602Config()
{
    // FZ602限速配置
    QVector<SpeedLimitEntry> limits;
    limits.append(SpeedLimitEntry(0, 8182, 95, 0));
    limits.append(SpeedLimitEntry(8182, 12441, 77, 0));
    limits.append(SpeedLimitEntry(12441, 27120, 95, 0));
    limits.append(SpeedLimitEntry(27120, 27169, 45, 0));
    limits.append(SpeedLimitEntry(27169, 28560, 70, 0));
    limits.append(SpeedLimitEntry(28560, 29694, 95, 0));
    limits.append(SpeedLimitEntry(29694, 29921, 47, 0));
    limits.append(SpeedLimitEntry(29921, 30977, 68, 0));
    limits.append(SpeedLimitEntry(30977, 36600, 95, 0));
    
    qDebug() << "加载FZ602限速配置，共" << limits.size() << "个区间";
    return limits;
}

QVector<SpeedLimitEntry> SpeedLimiter::loadFZ601Config()
{
    // FZ601限速配置（注意：FZ601是反向的，start > end）
    QVector<SpeedLimitEntry> limits;
    limits.append(SpeedLimitEntry(36520, 30897, 95, 0));
    limits.append(SpeedLimitEntry(30897, 29883, 95, 0));
    limits.append(SpeedLimitEntry(29883, 29764, 45, 0));
    limits.append(SpeedLimitEntry(29764, 28480, 60, 0));
    limits.append(SpeedLimitEntry(28480, 27210, 95, 0));
    limits.append(SpeedLimitEntry(27210, 27090, 45, 0));
    limits.append(SpeedLimitEntry(27090, 24279, 75, 0));
    limits.append(SpeedLimitEntry(24279, 14912, 95, 0));
    limits.append(SpeedLimitEntry(14912, 10902, 75, 0));
    limits.append(SpeedLimitEntry(10902, 0, 95, 0));
    
    qDebug() << "加载FZ601限速配置，共" << limits.size() << "个区间";
    return limits;
}

void SpeedLimiter::setSpeedLimits(const QVector<SpeedLimitEntry> &limits)
{
    m_speedLimits = limits;
}

