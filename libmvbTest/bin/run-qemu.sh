#!/bin/bash
# Qt Creator QEMU 运行脚本 - 通用版本

# 获取脚本所在目录（项目根目录）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 程序名称（修改为你的程序名）
APP_NAME="mvbtest"

# 查找可执行文件的可能位置
SEARCH_PATHS=(
    "$SCRIPT_DIR/bin"                           # 自定义 bin 目录
    "$SCRIPT_DIR/build"                         # build 根目录
    "$SCRIPT_DIR/build/Qt6_8_1ARM64-Debug"     # Qt Creator 默认目录
    "$SCRIPT_DIR/build/Qt6_8_1ARM64-Release"   # Release 目录
    "$SCRIPT_DIR"                               # 项目根目录
)

# 查找可执行文件
APP=""
for path in "${SEARCH_PATHS[@]}"; do
    if [ -f "$path/$APP_NAME" ]; then
        APP="$path/$APP_NAME"
        break
    fi
done

# 如果还没找到，尝试递归查找
if [ -z "$APP" ]; then
    APP=$(find "$SCRIPT_DIR" -type f -executable -name "$APP_NAME" 2>/dev/null | head -1)
fi

# 检查是否找到
if [ -z "$APP" ]; then
    echo "错误: 找不到可执行文件 '$APP_NAME'"
    echo ""
    echo "查找路径:"
    for path in "${SEARCH_PATHS[@]}"; do
        echo "  - $path"
    done
    echo ""
    echo "请检查:"
    echo "  1. 程序是否已编译"
    echo "  2. 程序名称是否正确（当前: $APP_NAME）"
    echo "  3. 输出目录是否正确"
    exit 1
fi

# 检查是否是 ARM64 程序
if ! file "$APP" | grep -q "ARM aarch64"; then
    echo "警告: $APP 可能不是 ARM64 程序"
    file "$APP"
    echo ""
fi

echo "找到程序: $APP"
echo ""

# 设置环境变量
export LD_LIBRARY_PATH=/opt/qt6-arm64/lib:/home/ubuntu/qt-arm64-sysroot/usr/lib/aarch64-linux-gnu
export QT_PLUGIN_PATH=/opt/qt6-arm64/plugins
export QT_QPA_PLATFORM=offscreen

# 使用 QEMU 运行
exec qemu-aarch64-static -L /home/ubuntu/qt-arm64-sysroot "$APP" "$@"
