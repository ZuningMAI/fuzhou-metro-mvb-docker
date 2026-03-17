# MetroDas Docker 构建准备脚本
# 此脚本检查并准备构建所需的所有文件

Write-Host "==================================" -ForegroundColor Cyan
Write-Host "MetroDas Docker 构建准备" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

$ErrorActionPreference = "Stop"

# 检查当前目录
$currentDir = Get-Location
Write-Host "当前目录: $currentDir" -ForegroundColor Gray
Write-Host ""

# 检查必需文件
Write-Host "检查必需文件..." -ForegroundColor Yellow

$requiredFiles = @(
    "Dockerfile",
    "build-image.ps1",
    "run-container.ps1",
    "compile-in-container.ps1",
    "build-all.sh",
    "start-metrodas.sh",
    "nginx.conf",
    "metrodas.tar.gz"
)

$missingFiles = @()
foreach ($file in $requiredFiles) {
    if (Test-Path $file) {
        Write-Host "  ✓ $file" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $file (缺失)" -ForegroundColor Red
        $missingFiles += $file
    }
}

# 检查 web 目录
if (Test-Path "web") {
    Write-Host "  ✓ web/" -ForegroundColor Green
} else {
    Write-Host "  ✗ web/ (缺失)" -ForegroundColor Red
    $missingFiles += "web/"
}

Write-Host ""

# 检查 libmvbTest
Write-Host "检查 libmvbTest..." -ForegroundColor Yellow

if (Test-Path "libmvbTest") {
    Write-Host "  ✓ libmvbTest/ 已存在" -ForegroundColor Green
    $libmvbTestReady = $true
} else {
    Write-Host "  ✗ libmvbTest/ 不存在" -ForegroundColor Red
    
    # 尝试从上级目录查找
    $parentLibmvbTest = Join-Path (Split-Path $currentDir -Parent) "libmvbTest"
    
    if (Test-Path $parentLibmvbTest) {
        Write-Host ""
        Write-Host "在上级目录找到 libmvbTest: $parentLibmvbTest" -ForegroundColor Yellow
        Write-Host ""
        
        $response = Read-Host "是否复制到当前目录? (y/n)"
        
        if ($response -eq "y" -or $response -eq "Y") {
            Write-Host ""
            Write-Host "正在复制 libmvbTest..." -ForegroundColor Yellow
            
            try {
                Copy-Item -Recurse -Force $parentLibmvbTest .
                Write-Host "  ✓ libmvbTest 复制成功" -ForegroundColor Green
                $libmvbTestReady = $true
            } catch {
                Write-Host "  ✗ 复制失败: $_" -ForegroundColor Red
                $libmvbTestReady = $false
            }
        } else {
            Write-Host "  跳过复制" -ForegroundColor Gray
            $libmvbTestReady = $false
        }
    } else {
        Write-Host ""
        Write-Host "未找到 libmvbTest 目录" -ForegroundColor Red
        Write-Host "请手动将 libmvbTest 目录复制到当前目录" -ForegroundColor Yellow
        $libmvbTestReady = $false
    }
}

Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "检查结果" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

if ($missingFiles.Count -gt 0) {
    Write-Host "缺失文件:" -ForegroundColor Red
    foreach ($file in $missingFiles) {
        Write-Host "  - $file" -ForegroundColor Red
    }
    Write-Host ""
    Write-Host "✗ 准备失败：缺少必需文件" -ForegroundColor Red
    Write-Host ""
    exit 1
}

if (-not $libmvbTestReady) {
    Write-Host "✗ 准备失败：libmvbTest 目录缺失" -ForegroundColor Red
    Write-Host ""
    Write-Host "请执行以下命令复制 libmvbTest:" -ForegroundColor Yellow
    Write-Host "  Copy-Item -Recurse ..\libmvbTest ." -ForegroundColor Cyan
    Write-Host ""
    exit 1
}

Write-Host "✓ 所有文件准备就绪！" -ForegroundColor Green
Write-Host ""
Write-Host "下一步：构建 Docker 镜像" -ForegroundColor Yellow
Write-Host "  .\build-image.ps1" -ForegroundColor Cyan
Write-Host ""
