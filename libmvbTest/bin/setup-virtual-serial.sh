#!/bin/bash

echo "=== 虚拟串口配置脚本 ==="

# 1. 检查 socat 是否安装
if ! command -v socat &> /dev/null; then
    echo "安装 socat..."
    sudo apt-get update
    sudo apt-get install -y socat
fi

# 2. 停止旧的 socat 进程
echo "清理旧的虚拟串口..."
pkill -f "socat.*vserial" 2>/dev/null
rm -f /tmp/vserial1 /tmp/vserial2

# 3. 创建虚拟串口对
echo "创建虚拟串口对..."
socat -d -d pty,raw,echo=0,link=/tmp/vserial1 pty,raw,echo=0,link=/tmp/vserial2 &
SOCAT_PID=$!

# 等待创建完成
sleep 2

# 4. 检查是否创建成功
if [ ! -e /tmp/vserial1 ] || [ ! -e /tmp/vserial2 ]; then
    echo "错误：虚拟串口创建失败"
    exit 1
fi

# 5. 设置权限
sudo chmod 666 /tmp/vserial1 /tmp/vserial2

# 6. 显示信息
echo ""
echo "✓ 虚拟串口创建成功："
ls -l /tmp/vserial*
echo ""
echo "socat 进程 PID: $SOCAT_PID"
echo ""

echo ""
echo "=== 配置完成 ==="
echo ""
echo "现在可以运行程序："
echo "  cd ~/QtProj/mvb_send_demo/bin"
echo "  ./mvb_send_demo"
echo ""
echo "监控串口数据（另一个终端）："
echo "  hexdump -C /tmp/vserial2"
echo ""
echo "停止虚拟串口："
echo "  kill $SOCAT_PID"
echo ""
