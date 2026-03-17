/********************************
* @note： 集成 EGWM_SIM 仿真器的 MVB 测试程序
* 仿真器产生的数据直接用作 MVB 端口的 val 值
* 不再通过中间文件 dataloader.txt
********************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mvbapp.h"
#include "mvb_pressure.h"
#include "mvbapi.h"
#include <sys/select.h>
#include <QCoreApplication>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>
#include <QMap>
#include <QMutex>
#include <QThread>

// EGWM_SIM 封装接口
#include "EGWM_SIM/egwm_wrapper.h"
#include "EGWM_SIM/dataDef.h"

// 全局数据存储：PD地址 -> 数据
static QMap<quint16, QByteArray> g_pdDataMap;
static QMutex g_dataMutex;

// MVB数据回调函数：当仿真器发送数据时调用
void onMvbDataReceived(quint16 pdAddress, const QByteArray &data)
{
    QMutexLocker locker(&g_dataMutex);
    g_pdDataMap[pdAddress] = data;
    
    // 调试输出
    QString pdStr = QString("0x%1").arg(pdAddress, 4, 16, QChar('0')).toUpper();
    qDebug() << "[MVB回调] PD=" << pdStr << "数据长度=" << data.size();
}

// 获取指定PD端口的数据
static QByteArray getPdData(quint16 pdAddress)
{
    QMutexLocker locker(&g_dataMutex);
    return g_pdDataMap.value(pdAddress, QByteArray());
}

static void print_message_hex(unsigned char* data, int dataLen)
{
    int i = 0;
    if (data == NULL) { return; }

    for (i = 0; i < dataLen; i++)
    {
        printf("%02X ", *(data + i));

        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}

// PD端口地址到数据长度的映射
static QMap<int, int> PD_list_init()
{
    QMap<int, int> pdMap;
    pdMap.insert(TIME_PORT, 8);
    pdMap.insert(TRAIN_PORT_1, 8);
    pdMap.insert(TRAIN_PORT_2, 8);
    pdMap.insert(TRAIN_PORT_3, 8);
    pdMap.insert(TRAIN_PORT_4, 8);
    pdMap.insert(RUNINFO_PORT, 32);
    pdMap.insert(POSINFO_PORT, 32);
    pdMap.insert(RAILWAYINFO_PORT, 32);
    pdMap.insert(CARRIAGEINFO_PORT_1, 32);
    pdMap.insert(CARRIAGEINFO_PORT_2, 32);
    pdMap.insert(AIRBRAKEPOWER_PORT_1, 32);
    pdMap.insert(AIRBRAKEPOWER_PORT_2, 32);

    return pdMap;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    qDebug() << "====================================";
    qDebug() << "    MVB测试程序 + EGWM仿真器";
    qDebug() << "====================================";

    // 解析命令行参数获取配置文件
    QString configFile = "config_line_mode.json";
    if (argc > 1) {
        configFile = argv[1];
    }
    
    qDebug() << "使用配置文件:" << configFile;
    qDebug() << "";

    // 初始化EGWM仿真器
    if (!EGWMWrapper::initialize(configFile, "Log")) {
        qWarning() << "EGWM仿真器初始化失败";
        return -1;
    }

    // 设置EGWM数据回调函数
    EGWMWrapper::setDataCallback(onMvbDataReceived);

    // 启动EGWM仿真器
    if (!EGWMWrapper::start()) {
        qWarning() << "EGWM仿真器启动失败";
        EGWMWrapper::cleanup();
        return -1;
    }

    qDebug() << "";
    qDebug() << "========== MVB初始化 ==========";
    
    // 初始化MVB
    MVBAPP_Init();
    printf("MVB应用已初始化\n");

    // UDP 目标地址
    // QHostAddress targetAddr("192.168.2.83");
    // quint16 targetPort = 6000;
    // QUdpSocket udpSocket;

    QHostAddress targetAddr("127.0.0.1");
    quint16 targetPort = 24001;
    QUdpSocket udpSocket;

    // 初始化 PD 列表
    QMap<int, int> pdMap = PD_list_init();

    qDebug() << "";
    qDebug() << "仿真正在运行...";
    qDebug() << "按 Ctrl+C 停止";
    qDebug() << "";

    char buffer[32] = {0};

    // 主循环：读取仿真器产生的数据并设置到MVB端口
    while (EGWMWrapper::isRunning())
    {
        // 处理Qt事件（让仿真器定时器工作）
        EGWMWrapper::processEvents();

        // 遍历所有PD端口
        for (auto it = pdMap.constBegin(); it != pdMap.constEnd(); ++it)
        {
            int pdAddr = it.key();
            int dataLen = it.value();

            // 从全局数据存储获取仿真器产生的数据
            QByteArray simData = getPdData(static_cast<quint16>(pdAddr));
            
            if (!simData.isEmpty() && simData.size() >= dataLen)
            {
                // 使用仿真器产生的数据
                memcpy(buffer, simData.constData(), dataLen);
            }
            else
            {
                // 如果没有仿真数据，清空缓冲区
                memset(buffer, 0, sizeof(buffer));
            }

            // 设置MVB数据
            MVBAPP_SetMvbData(pdAddr, buffer, dataLen);

            // 读取MVB数据
            memset(buffer, 0, sizeof(buffer));
            if (MVBAPP_GetMvbData(pdAddr, buffer, dataLen) == 0)
            {
                // 打印调试信息
                printf("PD: 0x%02X, Data (%d bytes):\n", pdAddr, dataLen);
                print_message_hex(reinterpret_cast<unsigned char*>(buffer), dataLen);

                QByteArray data = QByteArray(buffer, dataLen);
                QString hexData = data.toHex().toUpper();
                
                // 构造 JSON
                QJsonObject json;
                json["pd"] = pdAddr;
                json["data"] = hexData;

                QJsonDocument doc(json);
                QByteArray jsonBytes = doc.toJson(QJsonDocument::Compact);

                // 通过 UDP 发送
                qint64 sent = udpSocket.writeDatagram(jsonBytes, targetAddr, targetPort);
                if (sent != jsonBytes.size()) {
                    fprintf(stderr, "UDP send failed for PD 0x%02X: %s\n",
                            pdAddr, udpSocket.errorString().toLocal8Bit().constData());
                }
            }
            else
            {
                fprintf(stderr, "Failed to read MVB data from PD 0x%02X\n", pdAddr);
            }

            usleep(1000); // 1ms
        }

        // 主循环间隔
        usleep(128000); // 100ms
    }

    // 清理
    qDebug() << "";
    qDebug() << "程序退出，清理资源...";
    EGWMWrapper::cleanup();

    return 0;
}
