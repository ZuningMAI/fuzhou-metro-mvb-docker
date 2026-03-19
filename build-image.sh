#!/bin/bash
# MetroDas 项目 Docker 镜像构建脚本
# 用于构建包含 MetroDas 和 libmvbTest 的完整开发环境

# 步骤 1: 设置 QEMU
echo "步骤 1/3: 设置 QEMU ARM64 支持..."
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

if [ $? -ne 0 ]; then
    echo "错误: QEMU 设置失败"
    exit 1
fi
echo "  ✓ QEMU 设置完成"
echo ""

# 步骤 2: 构建镜像
echo "步骤 2/3: 构建 Docker 镜像..."
echo "  镜像名称: metrodas-env:latest"
echo "  平台: linux/arm64"
echo "  这可能需要几分钟时间..."
echo ""

docker build --platform linux/arm64 -t metrodas-env:latest .

if [ $? -ne 0 ]; then
    echo "\n错误: 镜像构建失败"
    exit 1
fi

echo "\n  ✓ 镜像构建完成"
echo ""

# 步骤 3: 验证镜像
echo "步骤 3/3: 验证镜像..."
imageInfo=$(docker images metrodas-env:latest --format "{{.Size}}")

if [ "$imageInfo" ]; then
    echo "  ✓ 镜像验证成功"
    echo "  镜像大小: $imageInfo"
else
    echo "  ✗ 镜像验证失败"
    exit 1
fi

echo ""
echo "=================================="
echo "✓ 构建完成！"
echo "=================================="
echo ""
echo "下一步:"
echo "  运行容器: ./run-container.sh"
echo "  或手动运行:"
echo "    docker run -it --platform linux/arm64 --name metrodas-dev \
      -p 24000:24000 -p 8000:8000 -p 24001:24001 \
      -v $(pwd)/../libmvbTest:/workspace/libmvbTest \
      metrodas-env:latest"
echo ""
