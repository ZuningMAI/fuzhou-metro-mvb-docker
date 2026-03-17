# MetroDas 容器启动脚本
# 启动包含所有服务的开发容器

Write-Host "==================================" -ForegroundColor Cyan
Write-Host "启动 MetroDas 开发容器" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# 检查镜像是否存在
$imageExists = docker images metrodas-env:latest -q

if (-not $imageExists) {
    Write-Host "错误: 镜像 metrodas-env:latest 不存在" -ForegroundColor Red
    Write-Host "请先运行: .\build-image.ps1" -ForegroundColor Yellow
    exit 1
}

# 检查是否已有同名容器
$existingContainer = docker ps -a --filter "name=metrodas-dev" --format "{{.ID}}"

if ($existingContainer) {
    Write-Host "发现已存在的容器: $existingContainer" -ForegroundColor Yellow
    $response = Read-Host "是否删除并重新创建? (y/n)"
    
    if ($response -eq 'y' -or $response -eq 'Y') {
        Write-Host "停止并删除旧容器..." -ForegroundColor Yellow
        docker stop metrodas-dev 2>$null
        docker rm metrodas-dev 2>$null
        Write-Host "  ✓ 旧容器已删除" -ForegroundColor Green
    } else {
        Write-Host "启动现有容器..." -ForegroundColor Yellow
        docker start metrodas-dev
        docker exec -it metrodas-dev bash
        exit 0
    }
}

Write-Host ""

# 获取 libmvbTest 路径
$libmvbTestPath = Resolve-Path ".\libmvbTest" -ErrorAction SilentlyContinue

if (-not $libmvbTestPath) {
    Write-Host "警告: 未找到 libmvbTest 目录" -ForegroundColor Yellow
    Write-Host "将不挂载 libmvbTest 目录" -ForegroundColor Yellow
    $mountLibmvb = $false
} else {
    Write-Host "找到 libmvbTest 目录: $libmvbTestPath" -ForegroundColor Green
    $mountLibmvb = $true
}

Write-Host ""
Write-Host "创建新容器..." -ForegroundColor Yellow
Write-Host "  容器名称: metrodas-dev" -ForegroundColor White
Write-Host "  端口映射:" -ForegroundColor White
Write-Host "    - 30000:24000 (Web 前端)" -ForegroundColor White
Write-Host "    - 30001:8000 (后端 API)" -ForegroundColor White
Write-Host "    - 30002:24001 (UDP MVB)" -ForegroundColor White

if ($mountLibmvb) {
    Write-Host "  挂载目录:" -ForegroundColor White
    Write-Host "    - $libmvbTestPath -> /workspace/libmvbTest" -ForegroundColor White
}

Write-Host ""

# 创建容器
if ($mountLibmvb) {
    docker run -it --platform linux/arm64 `
        --name metrodas-dev `
        --cap-add=SYS_TIME `
        -p 30000:24000 `
        -p 30001:8000 `
        -p 30002:24001 `
        -v "${libmvbTestPath}:/workspace/libmvbTest" `
        metrodas-env:latest
} else {
    docker run -it --platform linux/arm64 `
        --name metrodas-dev `
        --cap-add=SYS_TIME `
        -p 30000:24000 `
        -p 30001:8000 `
        -p 30002:24001 `
        metrodas-env:latest
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "`n错误: 容器启动失败" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "容器已退出" -ForegroundColor Yellow
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "重新进入容器:" -ForegroundColor Yellow
Write-Host "  docker start metrodas-dev" -ForegroundColor White
Write-Host "  docker exec -it metrodas-dev bash" -ForegroundColor White
Write-Host ""
Write-Host "编译项目:" -ForegroundColor Yellow
Write-Host "  docker exec -it metrodas-dev /workspace/build-all.sh" -ForegroundColor White
Write-Host ""
Write-Host "删除容器:" -ForegroundColor Yellow
Write-Host "  docker rm metrodas-dev" -ForegroundColor White
Write-Host ""
