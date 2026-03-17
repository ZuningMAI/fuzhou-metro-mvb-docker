# MetroDas + libmvbTest 集成开发环境
# 基于 Ubuntu 24.04 ARM64，包含 Qt 6.4.2 和所有必要的依赖

FROM --platform=linux/arm64 ubuntu:24.04
# 设置环境变量
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai
ENV QT_QPA_PLATFORM=offscreen

# 设置中文环境
ENV LANG=zh_CN.UTF-8
ENV LC_ALL=zh_CN.UTF-8

# 安装基础工具和依赖

RUN apt update && apt upgrade -y && \
    apt install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    vim \
    nginx \
    socat \
    tmux \
    bsdextrautils 

RUN apt install -y zlib1g-dev

# 安装 Qt6 和图形库
RUN apt install -y \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-serialport-dev \
    qt6-httpserver-dev  \
    qt6-websockets-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev

# 注意: qt6-httpserver-dev 在 Ubuntu 24.04 中不可用，已在代码中处理

# 安装中文语言包并配置
RUN apt install -y locales language-pack-zh-hans && \
    locale-gen zh_CN.UTF-8 && \
    apt clean && rm -rf /var/lib/apt/lists/*

# 创建符号链接
RUN ln -sf /usr/bin/qmake6 /usr/bin/qmake || true

# 创建工作目录
WORKDIR /workspace

# 复制项目文件
COPY metrodas.tar.gz /workspace/
COPY nginx.conf /etc/nginx/nginx.conf
COPY web /workspace/MAIN/web

# 解压 MetroDas
RUN tar -xzf metrodas.tar.gz -C /workspace/ && \
    rm metrodas.tar.gz

COPY libmvbTest /workspace/libmvbTest

# 复制编译和启动脚本
COPY build-all.sh /workspace/
COPY start-metrodas.sh /workspace/
RUN chmod +x /workspace/build-all.sh /workspace/start-metrodas.sh

# 设置权限
RUN chmod +x /workspace/metrodas/bin/MetroDas && \
    chmod -R 755 /workspace/metrodas && \
    chmod -R 755 /workspace/MAIN/web


# 验证 nginx 配置
RUN nginx -t

# 暴露端口
# 24000: nginx web 前端
# 8000: MetroDas 后端 API
# 24001: UDP MVB 数据接收
EXPOSE 24000 8000 24001

# 创建启动脚本
RUN echo '#!/bin/bash\n\
echo "================================="\n\
echo "MetroDas 开发环境"\n\
echo "================================="\n\
echo ""\n\
echo "端口映射:"\n\
echo "  - 24000: Web 前端"\n\
echo "  - 8000: 后端 API"\n\
echo "  - 24001: UDP MVB 数据"\n\
echo ""\n\
echo "访问地址: http://localhost:24000"\n\
echo ""\n\
echo "启动服务..."\n\
nginx\n\
echo "✓ nginx 已启动"\n\
echo ""\n\
echo "========================================"\n\
echo "重要: 首次运行需要编译项目"\n\
echo "========================================"\n\
echo ""\n\
echo "运行以下命令编译 libmvbTest 和 MetroDas:"\n\
echo "  /workspace/build-all.sh"\n\
echo ""\n\
echo "或分别编译:"\n\
echo ""\n\
echo "1. 编译 libmvbTest:"\n\
echo "   cd /workspace/libmvbTest/build"\n\
echo "   cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake/Qt6 .."\n\
echo "   cmake --build . --parallel \$(nproc)"\n\
echo ""\n\
echo "2. 编译 MetroDas:"\n\
echo "   cd /workspace/metrodas/build"\n\
echo "   cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake/Qt6 .."\n\
echo "   cmake --build . --parallel \$(nproc)"\n\
echo ""\n\
echo "3. 启动服务:"\n\
echo "   /workspace/start-metrodas.sh"\n\
echo ""\n\
exec /bin/bash\n\
' > /entrypoint.sh && chmod +x /entrypoint.sh

# 设置入口点
ENTRYPOINT ["/entrypoint.sh"]
