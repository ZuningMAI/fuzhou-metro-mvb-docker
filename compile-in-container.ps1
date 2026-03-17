# 在容器中编译 libmvbTest 和 MetroDas 的脚本

Write-Host "==================================" -ForegroundColor Cyan
Write-Host "编译 libmvbTest 和 MetroDas" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# 检查容器是否运行
$containerRunning = docker ps --filter "name=metrodas-dev" --format "{{.ID}}"

if (-not $containerRunning) {
    Write-Host "错误: 容器 metrodas-dev 未运行" -ForegroundColor Red
    Write-Host "请先运行: .\run-container.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "容器 ID: $containerRunning" -ForegroundColor Green
Write-Host ""

# 询问是否要编译
$response = Read-Host "是否开始编译? (y/n)"

if ($response -ne 'y' -and $response -ne 'Y') {
    Write-Host "已取消" -ForegroundColor Yellow
    exit 0
}

Write-Host ""
Write-Host "开始编译..." -ForegroundColor Yellow
Write-Host "这可能需要几分钟时间..." -ForegroundColor White
Write-Host ""

# 执行编译脚本
docker exec -it metrodas-dev /workspace/build-all.sh

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "==================================" -ForegroundColor Cyan
    Write-Host "✓ 编译完成" -ForegroundColor Green
    Write-Host "==================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "下一步:" -ForegroundColor Yellow
    Write-Host "  1. 进入容器: docker exec -it metrodas-dev bash" -ForegroundColor White
    Write-Host "  2. 启动服务: /workspace/start-metrodas.sh" -ForegroundColor White
    Write-Host "  3. 访问: http://localhost:24000" -ForegroundColor White
    Write-Host ""
} else {
    Write-Host ""
    Write-Host "==================================" -ForegroundColor Cyan
    Write-Host "✗ 编译失败" -ForegroundColor Red
    Write-Host "==================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "请检查错误信息并修复问题" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "进入容器查看详细信息:" -ForegroundColor Yellow
    Write-Host "  docker exec -it metrodas-dev bash" -ForegroundColor White
    Write-Host ""
}
