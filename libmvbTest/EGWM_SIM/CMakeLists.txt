# EGWM_SIM 子模块 CMakeLists.txt
# 作为静态库编译，供 libmvbTest 使用

cmake_minimum_required(VERSION 3.16)

# 查找Qt6包
find_package(Qt6 REQUIRED COMPONENTS Core SerialPort)

# 定义源文件
set(EGWM_SIM_SOURCES
    csv_reader.cpp
    data_generator.cpp
    data_interpolator.cpp
    data_loader.cpp
    logger.cpp
    mvb_send.cpp
    runinfo_logger.cpp
    serial_sender.cpp
    simulation_controller.cpp
    simulator_config.cpp
    speed_limiter.cpp
    egwm_wrapper.cpp
)

# 定义头文件
set(EGWM_SIM_HEADERS
    csv_reader.h
    data_generator.h
    data_interpolator.h
    data_loader.h
    dataDef.h
    logger.h
    mvb_send.h
    runinfo_logger.h
    serial_sender.h
    simulation_controller.h
    simulator_config.h
    speed_limiter.h
    egwm_wrapper.h
)

# 创建静态库
add_library(egwm_sim STATIC ${EGWM_SIM_SOURCES} ${EGWM_SIM_HEADERS})

# 设置包含目录
target_include_directories(egwm_sim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 链接Qt库
target_link_libraries(egwm_sim PUBLIC
    Qt6::Core
    Qt6::SerialPort
)

# 自动处理moc
set_target_properties(egwm_sim PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# C++17标准
target_compile_features(egwm_sim PUBLIC cxx_std_17)
