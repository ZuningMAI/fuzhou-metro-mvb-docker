#!/bin/bash
# MetroDas 容器启动脚本
# 启动包含所有服务的开发容器

echo "=================================="
echo "启动 MetroDas 开发容器"
echo "=================================="
echo ""

# 检查镜像是否存在
imageExists=$(docker images metrodas-env:latest -q)

if [ -z "$imageExists" ]; then
    echo "错误: 镜像 metrodas-env:latest 不存在"
    echo "请先运行: ./build-image.sh"
    exit 1
fi

# 检查是否已有同名容器
existingContainer=$(docker ps -a --filter "name=metrodas-dev" --format "{{.ID}}")

if [ "$existingContainer" ]; then
    echo "发现已存在的容器: $existingContainer"
    read -p "是否删除并重新创建? (y/n) " response
    
    if [ "$response" = "y" ] || [ "$response" = "Y" ]; then
        echo "停止并删除旧容器..."
        docker stop metrodas-dev 2>/dev/null
        docker rm metrodas-dev 2>/dev/null
        echo "  ✓ 旧容器已删除"
    else
        echo "启动现有容器..."
        docker start metrodas-dev
        docker exec -it metrodas-dev bash
        exit 0
    fi
fi

echo ""

# 获取 libmvbTest 路径
libmvbTestPath="./libmvbTest"

if [ -d "$libmvbTestPath" ]; then
    libmvbTestPath=$(realpath "$libmvbTestPath")
    echo "找到 libmvbTest 目录: $libmvbTestPath"
    mountLibmvb=true
else
    echo "警告: 未找到 libmvbTest 目录"
    echo "将不挂载 libmvbTest 目录"
    mountLibmvb=false
fi

echo ""
echo "创建新容器..."
echo "  容器名称: metrodas-dev"
echo "  端口映射:"
echo "    - 30000:24000 (Web 前端)"
echo "    - 30001:8000 (后端 API)"
echo "    - 30002:24001 (UDP MVB)"

if [ "$mountLibmvb" = true ]; then
    echo "  挂载目录:"
    echo "    - $libmvbTestPath -> /workspace/libmvbTest"
fi

echo ""

# 创建容器
if [ "$mountLibmvb" = true ]; then
    docker run -it --platform linux/arm64 \
        --name metrodas-dev \
        --cap-add=SYS_TIME \
        -p 30000:24000 \
        -p 30001:8000 \
        -p 30002:24001 \
        -v "$libmvbTestPath:/workspace/libmvbTest" \
        metrodas-env:latest
else
    docker run -it --platform linux/arm64 \
        --name metrodas-dev \
        --cap-add=SYS_TIME \
        -p 30000:24000 \
        -p 30001:8000 \
        -p 30002:24001 \
        metrodas-env:latest
fi

if [ $? -ne 0 ]; then
    echo "\n错误: 容器启动失败"
    exit 1
fi

echo ""
echo "=================================="
echo "容器已退出"
echo "=================================="
echo ""
echo "重新进入容器:"
echo "  docker start metrodas-dev"
echo "  docker exec -it metrodas-dev bash"
echo ""
echo "编译项目:"
echo "  docker exec -it metrodas-dev /workspace/build-all.sh"
echo ""
echo "删除容器:"
echo "  docker rm metrodas-dev"
echo ""
