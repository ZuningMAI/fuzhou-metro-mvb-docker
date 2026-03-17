#-------------------------------------------------
# Qt 控制台应用（无 GUI）
# 集成 EGWM_SIM 仿真器
#-------------------------------------------------
QT       -= gui
QT       += core network serialport
CONFIG   += console c++17
CONFIG   -= app_bundle

TARGET = mvbtest
TEMPLATE = app

# 输出目录配置
DESTDIR = $$PWD/bin              # 可执行文件输出到 bin 目录
OBJECTS_DIR = $$PWD/bin/obj      # 对象文件
MOC_DIR = $$PWD/bin/moc          # MOC 文件
RCC_DIR = $$PWD/bin/rcc          # 资源文件
UI_DIR = $$PWD/bin/ui            # UI 文件

# 源文件
SOURCES += \
    main.cpp \
    mvb_pressure.cpp \
    mvbapp.cpp

# 头文件路径
INCLUDEPATH += $$PWD/include

# 包含 EGWM_SIM 子模块
include(EGWM_SIM/mvb_send_demo.pri)

# 库路径和链接库（直接链接 .a 文件）
LIBS += \
    $$PWD/lib/libmvbapi.a \
    -lpthread \
    -lrt

# 宏定义
DEFINES += __linux

# 链接器选项
QMAKE_LFLAGS += -rdynamic

# 禁用 PIE（关键修复：解决 aarch64 + 非-PIC 静态库的链接错误）
QMAKE_LFLAGS += -no-pie

# 编译器警告
QMAKE_CXXFLAGS += -Wall
