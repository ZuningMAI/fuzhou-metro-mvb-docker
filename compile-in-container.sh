#!/bin/bash
# 在容器中编译 libmvbTest 和 MetroDas 的脚本

echo "=================================="
echo "编译 libmvbTest 和 MetroDas"
echo "=================================="
echo ""

# 检查容器是否运行
containerRunning=$(docker ps --filter "name=metrodas-dev" --format "{{.ID}}")

if [ -z "$containerRunning" ]; then
    echo "错误: 容器 metrodas-dev 未运行"
    echo "请先运行: ./run-container.sh"
    exit 1
fi

echo "容器 ID: $containerRunning"
echo ""

# 询问是否要编译
read -p "是否开始编译? (y/n) " response

if [ "$response" != "y" ] && [ "$response" != "Y" ]; then
    echo "已取消"
    exit 0
fi

echo ""
echo "开始编译..."
echo "这可能需要几分钟时间..."
echo ""

# 执行编译脚本
docker exec -it metrodas-dev /workspace/build-all.sh

if [ $? -eq 0 ]; then
    echo ""
    echo "=================================="
    echo "✓ 编译完成"
    echo "=================================="
    echo ""
    echo "下一步:"
    echo "  1. 进入容器: docker exec -it metrodas-dev bash"
    echo "  2. 启动服务: /workspace/start-metrodas.sh"
    echo "  3. 访问: http://localhost:24000"
    echo ""
else
    echo ""
    echo "=================================="
    echo "✗ 编译失败"
    echo "=================================="
    echo ""
    echo "请检查错误信息并修复问题"
    echo ""
    echo "进入容器查看详细信息:"
    echo "  docker exec -it metrodas-dev bash"
    echo ""
fi
