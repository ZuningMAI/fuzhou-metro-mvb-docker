#!/bin/bash
# 编译 libmvbTest 和 MetroDas 的脚本

set -e

echo "=========================================="
echo "编译 libmvbTest 和 MetroDas"
echo "=========================================="
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 1. 编译 libmvbTest
echo -e "${YELLOW}步骤 1/2: 编译 libmvbTest${NC}"
echo "----------------------------------------"

if [ -d "/workspace/libmvbTest" ]; then
    cd /workspace/libmvbTest
    
    rm -rf build
    
    # 创建 build 目录
    if [ ! -d "build" ]; then
        echo "创建 build 目录..."
        mkdir -p build
    fi
    
    cd build
    
    echo "运行 CMake 配置..."
    cmake -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake/Qt6 \
        ..
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}✗ CMake 配置失败${NC}"
        exit 1
    fi
    
    echo ""
    echo "开始编译..."
    cmake --build . --parallel $(nproc)
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ libmvbTest 编译成功${NC}"
        
        # 检查生成的文件
        if [ -f "bin/mvbtest" ]; then
            echo "  可执行文件: $(pwd)/bin/mvbtest"
            ls -lh bin/mvbtest
        fi
    else
        echo -e "${RED}✗ libmvbTest 编译失败${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}⚠ libmvbTest 目录不存在，跳过编译${NC}"
fi

echo ""
echo ""

# 2. 重新编译 MetroDas
echo -e "${YELLOW}步骤 2/2: 重新编译 MetroDas${NC}"
echo "----------------------------------------"

if [ -d "/workspace/metrodas" ]; then
    cd /workspace/metrodas

    rm -rf build
    
    # 创建 build 目录
    if [ ! -d "build" ]; then
        echo "创建 build 目录..."
        mkdir -p build
    fi
    
    cd build
    
    echo "运行 CMake 配置..."
    cmake -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake/Qt6 \
        ..
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}✗ CMake 配置失败${NC}"
        exit 1
    fi
    
    echo ""
    echo "开始编译..."
    cmake --build . --parallel $(nproc)
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ MetroDas 编译成功${NC}"
        
        # 检查生成的文件
        if [ -f "/workspace/metrodas/bin/MetroDas" ]; then
            echo "  可执行文件: /workspace/metrodas/bin/MetroDas"
            ls -lh /workspace/metrodas/bin/MetroDas
        fi
        
        # 检查依赖库
        echo ""
        echo "检查依赖库..."
        if [ -f "/workspace/metrodas/bin/libdas_db_if.so" ]; then
            echo "  ✓ libdas_db_if.so"
        else
            echo -e "  ${YELLOW}⚠ libdas_db_if.so 不存在${NC}"
        fi
        
        if [ -f "/workspace/metrodas/bin/libdas_alg_if.so" ]; then
            echo "  ✓ libdas_alg_if.so"
        else
            echo -e "  ${YELLOW}⚠ libdas_alg_if.so 不存在${NC}"
        fi
    else
        echo -e "${RED}✗ MetroDas 编译失败${NC}"
        exit 1
    fi
else
    echo -e "${RED}✗ metrodas 目录不存在${NC}"
    exit 1
fi

echo ""
echo "=========================================="
echo -e "${GREEN}✓ 所有编译完成${NC}"
echo "=========================================="
echo ""
echo "下一步:"
echo "  1. 启动服务: /workspace/start-metrodas.sh"
echo "  2. 或手动启动:"
echo "     - nginx"
echo "     - cd /workspace/metrodas/bin && ./MetroDas"
echo "     - cd /workspace/libmvbTest/bin && ./mvbtest"
echo ""
