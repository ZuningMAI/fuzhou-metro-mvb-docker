# MetroDas 项目 Docker 镜像构建脚本
# 用于构建包含 MetroDas 和 libmvbTest 的完整开发环境

# 步骤 1: 设置 QEMU
Write-Host "步骤 1/3: 设置 QEMU ARM64 支持..." -ForegroundColor Yellow
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

if ($LASTEXITCODE -ne 0) {
    Write-Host "错误: QEMU 设置失败" -ForegroundColor Red
    exit 1
}
Write-Host "  ✓ QEMU 设置完成" -ForegroundColor Green
Write-Host ""

# 步骤 2: 构建镜像
Write-Host "步骤 2/3: 构建 Docker 镜像..." -ForegroundColor Yellow
Write-Host "  镜像名称: metrodas-env:latest" -ForegroundColor White
Write-Host "  平台: linux/arm64" -ForegroundColor White
Write-Host "  这可能需要几分钟时间..." -ForegroundColor White
Write-Host ""

docker build --platform linux/arm64 -t metrodas-env:latest .

if ($LASTEXITCODE -ne 0) {
    Write-Host "`n错误: 镜像构建失败" -ForegroundColor Red
    exit 1
}

Write-Host "`n  ✓ 镜像构建完成" -ForegroundColor Green
Write-Host ""

# 步骤 3: 验证镜像
Write-Host "步骤 3/3: 验证镜像..." -ForegroundColor Yellow
$imageInfo = docker images metrodas-env:latest --format "{{.Size}}"

if ($imageInfo) {
    Write-Host "  ✓ 镜像验证成功" -ForegroundColor Green
    Write-Host "  镜像大小: $imageInfo" -ForegroundColor White
} else {
    Write-Host "  ✗ 镜像验证失败" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "✓ 构建完成！" -ForegroundColor Green
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "下一步:" -ForegroundColor Yellow
Write-Host "  运行容器: .\run-container.ps1" -ForegroundColor White
Write-Host "  或手动运行:" -ForegroundColor White
Write-Host "    docker run -it --platform linux/arm64 --name metrodas-dev ``" -ForegroundColor Gray
Write-Host "      -p 24000:24000 -p 8000:8000 -p 24001:24001 ``" -ForegroundColor Gray
Write-Host "      -v `${PWD}/../libmvbTest:/workspace/libmvbTest ``" -ForegroundColor Gray
Write-Host "      metrodas-env:latest" -ForegroundColor Gray
Write-Host ""
