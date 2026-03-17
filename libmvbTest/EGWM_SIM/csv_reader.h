#ifndef CSV_READER_H
#define CSV_READER_H

#include "simulator_config.h"
#include <QString>
#include <QVector>

class CSVReader {
public:
    // 读取单个CSV文件
    static bool readCSVFile(const QString &filePath, SectionInfo &sectionInfo);
    
    // 读取整条线路的数据（自动选择每个区间的一个CSV文件）
    static bool readLineData(const QString &railwayLine, QVector<SectionInfo> &sections);
    
    // 解析文件名获取区间信息
    // 例如: "OptReslog.2025-10-26_18-00-30-FZ602-1-2-85.csv"
    // 提取: line="FZ602", startStn=1, endStn=2, time=85
    static bool parseFileName(const QString &fileName, 
                             QString &line, int &startStn, int &endStn, int &time);
    
private:
    // 解析CSV文件的头信息行
    static bool parseHeaderLine(const QString &line, SectionInfo &info);
    
    // 解析CSV数据行
    static bool parseDataLine(const QString &line, TrajectoryPoint &point);
    
    // 查找指定区间的CSV文件
    static QString findSectionCSV(const QString &railwayLine, int startStn, int endStn);
    
    // 获取线路的区间数量（FZ601和FZ602都是18个区间：1->2, 2->3, ..., 18->19）
    static int getLineStationCount(const QString &railwayLine);
    
    // 从JSON文件读取计划运行时间配置
    static QVector<int> loadPlannedTimesFromJson(const QString &railwayLine);
};

#endif // CSV_READER_H

