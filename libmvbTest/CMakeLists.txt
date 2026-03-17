cmake_minimum_required(VERSION 3.16)
project(mvbtest CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录到 bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 查找Qt6包（使用Core、Network和SerialPort模块）
find_package(Qt6 REQUIRED COMPONENTS Core Network SerialPort)

# 添加 EGWM_SIM 子模块
add_subdirectory(EGWM_SIM)

# 设置目标类型为可执行文件
add_executable(mvbtest)

# 添加源文件
target_sources(mvbtest PRIVATE
    main.cpp
    mvb_pressure.cpp
    mvbapp.cpp
)

# 添加头文件包含路径
target_include_directories(mvbtest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/EGWM_SIM
)

# 链接Qt库（Core、Network和SerialPort模块）
target_link_libraries(mvbtest PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::SerialPort
)

# 链接 EGWM_SIM 子模块
target_link_libraries(mvbtest PRIVATE egwm_sim)

# 添加其他库文件
target_link_libraries(mvbtest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libmvbapi.a
    pthread
    rt
)

# 添加宏定义
target_compile_definitions(mvbtest PRIVATE __linux)

# 添加链接器选项
target_link_options(mvbtest PRIVATE
    -rdynamic
    -no-pie
)

# 添加编译器警告
target_compile_options(mvbtest PRIVATE -Wall)

# 自动处理moc、uic、rcc等Qt工具
set_target_properties(mvbtest PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)
