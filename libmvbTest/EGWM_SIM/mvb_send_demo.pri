# EGWM_SIM 子模块 pri 文件
# 用于 qmake 项目包含

QT += serialport

INCLUDEPATH += $PWD

HEADERS += \
    $PWD/dataDef.h \
    $PWD/mvb_send.h \
    $PWD/simulator_config.h \
    $PWD/csv_reader.h \
    $PWD/data_interpolator.h \
    $PWD/speed_limiter.h \
    $PWD/data_generator.h \
    $PWD/serial_sender.h \
    $PWD/simulation_controller.h \
    $PWD/logger.h \
    $PWD/runinfo_logger.h \
    $PWD/data_loader.h \
    $PWD/egwm_wrapper.h

SOURCES += \
    $PWD/mvb_send.cpp \
    $PWD/simulator_config.cpp \
    $PWD/csv_reader.cpp \
    $PWD/data_interpolator.cpp \
    $PWD/speed_limiter.cpp \
    $PWD/data_generator.cpp \
    $PWD/serial_sender.cpp \
    $PWD/simulation_controller.cpp \
    $PWD/logger.cpp \
    $PWD/runinfo_logger.cpp \
    $PWD/data_loader.cpp \
    $PWD/egwm_wrapper.cpp
