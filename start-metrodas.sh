#!/bin/bash
# MetroDas 服务启动脚本
# 在容器内运行此脚本以启动所有服务

set -e

echo "=================================="
echo "启动 MetroDas 服务"
echo "=================================="
echo ""

# 检查是否在容器内
if [ ! -f /.dockerenv ]; then
    echo "错误: 此脚本必须在 Docker 容器内运行"
    exit 1
fi

# 1. 检查 nginx 配置
echo "1. 检查 nginx 配置..."
if nginx -t 2>&1 | grep -q "successful"; then
    echo "  ✓ nginx 配置正确"
else
    echo "  ✗ nginx 配置错误"
    nginx -t
    exit 1
fi

# 2. 启动 nginx
echo ""
echo "2. 启动 nginx..."
if pgrep nginx > /dev/null; then
    echo "  ⚠ nginx 已在运行"
else
    nginx
    sleep 1
    if pgrep nginx > /dev/null; then
        echo "  ✓ nginx 已启动"
    else
        echo "  ✗ nginx 启动失败"
        exit 1
    fi
fi

# 3. 检查 MetroDas 配置
echo ""
echo "3. 检查 MetroDas 配置..."
CONFIG_FILE="/workspace/metrodas/bin/config/MetroDas.ini"

if [ ! -f "$CONFIG_FILE" ]; then
    echo "  ✗ 配置文件不存在: $CONFIG_FILE"
    exit 1
fi

echo "  ✓ 配置文件存在"

# 显示当前配置的 IP
CURRENT_IP=$(grep "ServerIP=" "$CONFIG_FILE" | cut -d'=' -f2)
echo "  当前 ServerIP: $CURRENT_IP"

# 4. 提示用户更新 IP
echo ""
echo "4. 更新 ServerIP (可选)"
echo "  当前 IP: $CURRENT_IP"
echo "  如需更新，请运行:"
echo "    sed -i 's/ServerIP=.*/ServerIP=YOUR_IP/' $CONFIG_FILE"
echo ""
read -p "是否现在更新 IP? (y/n): " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    read -p "请输入新的 IP 地址: " NEW_IP
    sed -i "s/ServerIP=.*/ServerIP=$NEW_IP/" "$CONFIG_FILE"
    echo "  ✓ IP 已更新为: $NEW_IP"
fi

# 5. 启动 MetroDas
echo ""
echo "5. 启动 MetroDas..."

cd /workspace/metrodas/bin

if pgrep -f "MetroDas" > /dev/null; then
    echo "  ⚠ MetroDas 已在运行"
    echo "  PID: $(pgrep -f MetroDas)"
else
    echo "  启动中..."
    QT_QPA_PLATFORM=offscreen nohup ./MetroDas > metrodas.log 2>&1 &
    METRODAS_PID=$!
    
    sleep 3
    
    if ps -p $METRODAS_PID > /dev/null; then
        echo "  ✓ MetroDas 已启动"
        echo "  PID: $METRODAS_PID"
    else
        echo "  ✗ MetroDas 启动失败"
        echo "  查看日志: tail -f /workspace/metrodas/bin/metrodas.log"
        exit 1
    fi
fi

# 6. 验证服务
echo ""
echo "6. 验证服务状态..."

# 检查端口
echo "  检查端口监听..."
sleep 2

if ss -tulpn 2>/dev/null | grep -q ":24000"; then
    echo "    ✓ 端口 24000 (nginx) 正在监听"
else
    echo "    ✗ 端口 24000 未监听"
fi

if ss -tulpn 2>/dev/null | grep -q ":8000"; then
    echo "    ✓ 端口 8000 (MetroDas API) 正在监听"
else
    echo "    ⚠ 端口 8000 未监听 (可能需要等待)"
fi

if ss -tulpn 2>/dev/null | grep -q ":24001"; then
    echo "    ✓ 端口 24001 (UDP MVB) 正在监听"
else
    echo "    ⚠ 端口 24001 未监听"
fi

# 7. 完成
echo ""
echo "=================================="
echo "✓ 服务启动完成"
echo "=================================="
echo ""
echo "访问地址:"
echo "  Web 前端: http://localhost:30000"
echo "  后端 API: http://localhost:30001"
echo ""
echo "查看日志:"
echo "  MetroDas: tail -f /workspace/metrodas/bin/metrodas.log"
echo "  nginx: tail -f /var/log/nginx/error.log"
echo ""
echo "停止服务:"
echo "  pkill MetroDas"
echo "  nginx -s stop"
echo ""
