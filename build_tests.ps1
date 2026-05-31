# build_tests.ps1 — Compiles and links the unit test executable
# Usage: ./build_tests.ps1
# Output: bin/catnet_tests.exe

$ErrorActionPreference = "Stop"

# Ensure bin/ exists
New-Item -ItemType Directory -Force -Path "bin" | Out-Null

$buildCmd = 'clang-cl /nologo /W3 /TC /D _CRT_SECURE_NO_WARNINGS /D WIN32_LEAN_AND_MEAN /I src /I tests/unity tests/test_main.c tests/test_utils.c tests/test_list.c tests/test_parse_range.c tests/test_export.c tests/test_scan.c tests/unity/unity.c src/utils.c src/list.c src/range_parser.c src/export.c src/scan.c /Fe"bin/catnet_tests.exe" /link Ws2_32.lib'

# Determine which compiler is available in PATH
$cc = $null
if (Get-Command clang-cl -ErrorAction SilentlyContinue) {
    $cc = "clang-cl"
} elseif (Get-Command cl -ErrorAction SilentlyContinue) {
    $cc = "cl"
    $buildCmd = $buildCmd -replace '^clang-cl', 'cl'
}

if ($cc) {
    # Compiler found in PATH — run directly
    Write-Host "Building tests with $cc..."
    Invoke-Expression $buildCmd
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Test build failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }
} else {
    # Neither compiler in PATH — locate VsDevCmd.bat via vswhere
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        Write-Error "No compiler found in PATH and 'vswhere.exe' is missing. Open a Developer Command Prompt for VS and re-run this script."
    }

    $installPath = & $vswhere -latest -products * -property installationPath
    if (-not $installPath) {
        Write-Error "Visual Studio installation not found by vswhere. Please install Visual Studio Build Tools."
    }

    $vsDevCmd = Join-Path $installPath "Common7\Tools\VsDevCmd.bat"
    if (-not (Test-Path $vsDevCmd)) {
        Write-Error "VsDevCmd.bat not found at '$vsDevCmd'. Check your Build Tools installation."
    }

    Write-Host "Activating VS environment and building tests..."
    # Use cl (MSVC) as fallback when clang-cl is not available
    $buildCmdVs = $buildCmd -replace '^clang-cl', 'cl'
    $fullCmd = "`"$vsDevCmd`" -arch=amd64 -host_arch=amd64 && $buildCmdVs"
    Write-Host $fullCmd
    & cmd /c $fullCmd
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Test build failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }
}

Write-Host "Tests built successfully: bin/catnet_tests.exe"
