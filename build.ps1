Param(
  [ValidateSet('MSVC','Clang')]
  [string]$Compiler = 'Clang',
  [ValidateSet('Raygui')]
  [string]$UI = 'Raygui',
  [string]$RaylibInclude,
  [string]$RaylibLibs,
  [ValidateSet('build','chat-note','daily-snapshot','chat-clean')]
  [string]$Task = 'build',
  [string]$Text,
  [string]$LogName
)

$ErrorActionPreference = 'Stop'

# Project utility tasks (chat logs)
if ($Task -eq 'chat-note') {
  if (-not $Text) { Write-Error "Provide -Text for the chat note." }
  & (Join-Path $PSScriptRoot 'scripts/save_chat_note.ps1') -Text $Text -LogName $LogName
  return
}
elseif ($Task -eq 'daily-snapshot') {
  & (Join-Path $PSScriptRoot 'scripts/save_daily_snapshot.ps1')
  return
}
elseif ($Task -eq 'chat-clean') {
  $projectRoot = $PSScriptRoot
  $logDir = Join-Path $projectRoot 'docs\chat_logs'
  if (-not (Test-Path $logDir)) { Write-Host "Directory does not exist: $logDir"; return }
  Write-Host "Cleaning unnecessary files in $logDir ..."
  $kept = @()
  $removed = @()
  $attachmentExts = @('.png','.jpg','.jpeg','.gif','.svg','.pdf','.txt')
  Get-ChildItem -Path $logDir -Force -Recurse | ForEach-Object {
    $item = $_
    if ($item.PSIsContainer) {
      # remove common irrelevant directories
      if ($item.Name -match '^(vsdbg|node_modules)$') {
        try { Remove-Item -Path $item.FullName -Recurse -Force -ErrorAction Stop; $removed += $item.FullName } catch {}
      }
      return
    }
    $name = $item.Name
    $keep = $false
    if ($name -match '(^|-)chat\.md$') { $keep = $true }
    elseif ($name -match 'daily-snapshot\.md$') { $keep = $true }
    elseif ($name -match '^README\.md$') { $keep = $true }
    else {
      $ext = ([System.IO.Path]::GetExtension($name)).ToLower()
      if ($attachmentExts -contains $ext) { $keep = $true }
    }
    # Remove .json, .js, .html, and other non-md artifacts
    if ($keep) { $kept += $item.FullName }
    else {
      try { Remove-Item -Path $item.FullName -Force -ErrorAction Stop; $removed += $item.FullName } catch {}
    }
  }
  Write-Host "Kept: $($kept.Count) | Removed: $($removed.Count)"
  if ($removed.Count -gt 0) { Write-Host "Removed:"; $removed | ForEach-Object { Write-Host $_ } }
  return
}

New-Item -ItemType Directory -Force -Path "bin" | Out-Null

# Pre-build cleanup: remove stale object files in project root
$staleObjs = Get-ChildItem -Path $PWD.Path -Filter *.obj -File -ErrorAction SilentlyContinue
foreach ($o in $staleObjs) { try { Remove-Item -LiteralPath $o.FullName -Force -ErrorAction Stop } catch {} }

if ($Task -ne 'build') { return }

if ($Compiler -eq 'MSVC' -or $Compiler -eq 'Clang') {
  $isClang = ($Compiler -eq 'Clang')
  $compilerName = if ($isClang) { 'Clang (clang-cl/lld-link)' } else { 'MSVC (cl)' }
  Write-Host ("Building with {0} [Raygui only]" -f $compilerName)
  # Project sources only (src/); third-party will be added as needed
  $files = @(Get-ChildItem -Path (Join-Path $PWD 'src') -Recurse -Filter *.c | ForEach-Object { $_.FullName })
  $includes = ''
  $defines = '/D _CRT_SECURE_NO_WARNINGS'
  $cxxflags = ''
  $linkopts = ''
  $out = ''
  $libs = ''

  if ($UI -eq 'Raygui') {
    # GUI build (Raylib + Raygui)
    $out = (Join-Path $PWD.Path 'bin\catnet_scanner.exe')
    $libs = 'Ws2_32.lib Iphlpapi.lib user32.lib gdi32.lib shell32.lib kernel32.lib winmm.lib opengl32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib shcore.lib'
    # Configure Raylib
    $rlInc = $RaylibInclude; if (-not $rlInc) { $rlInc = $env:RAYLIB_INCLUDE }
    $rlLib = $RaylibLibs;    if (-not $rlLib) { $rlLib = $env:RAYLIB_LIBS }
    # Source files: C modules + main_raygui.c
    $files = $files | Where-Object { $_ -notmatch "\\src\\main.c$" }
    $files = $files | Where-Object { $_ -notmatch "\\src\\main_gacui.cpp$" }
    $files += Join-Path $PWD "src/main_raygui.c"
    # Compile as C (MSVC does not use /std, use /TC)
    $cxxflags = '/TC /utf-8 /MD'
    $linkopts = '/link /SUBSYSTEM:CONSOLE'

    # Force fallback: build raylib from source to ensure predictable binary
    $useLocalRaylib = $true
      $thirdParty = Join-Path $PWD 'third_party'
      New-Item -ItemType Directory -Force -Path $thirdParty | Out-Null
      $raylibDir = Join-Path $thirdParty 'raylib'
      $rayguiDir = Join-Path $thirdParty 'raygui'

      if (-not (Test-Path $raylibDir)) {
        Write-Host "Cloning raylib into $raylibDir ..."
        git clone https://github.com/raysan5/raylib.git $raylibDir | Out-Null
      } else { Write-Host "raylib present: $raylibDir" }

      if (-not (Test-Path $rayguiDir)) {
        Write-Host "Cloning raygui into $rayguiDir ..."
        git clone https://github.com/raysan5/raygui.git $rayguiDir | Out-Null
      } else { Write-Host "raygui present: $rayguiDir" }

      $rlSrc = Join-Path $raylibDir 'src'
      $rgSrc = Join-Path $rayguiDir 'src'
      # Current raylib (5.x) uses modular sources instead of monolithic raylib.c
      # raylib.c may not exist; we compile modules directly
      if (-not (Test-Path (Join-Path $rgSrc 'raygui.h'))) { Write-Error "raygui.h not found at $rgSrc" }

      # Include raylib and raygui headers
      $includes += " /I `"$rlSrc`" /I `"$rgSrc`""
      # Build core modules directly (avoid precompiled raylib.lib)
      $files += @(Join-Path $rlSrc 'rcore.c')
      $files += @(Join-Path $rlSrc 'rshapes.c')
      $files += @(Join-Path $rlSrc 'rtextures.c')
      $files += @(Join-Path $rlSrc 'rtext.c')
      $files += @(Join-Path $rlSrc 'rmodels.c')
      $files += @(Join-Path $rlSrc 'raudio.c')
      $files += @(Join-Path $rlSrc 'utils.c')
      # Select desktop RGFW backend to avoid GLFW dependency
      $defines += ' /D PLATFORM_DESKTOP_RGFW /D GRAPHICS_API_OPENGL_33'
      Write-Host "Using local build of raylib modules (PLATFORM_DESKTOP_RGFW) and raygui.h"
  }

  $cc = if ($isClang) { 'clang-cl' } else { 'cl' }
  if ($isClang) {
    if ((Get-Command lld-link -ErrorAction SilentlyContinue) -ne $null) { $linker = 'lld-link' } else { $linker = 'link' }
  } else { $linker = 'link' }
  $ccExists = (Get-Command $cc -ErrorAction SilentlyContinue) -ne $null
  if ($isClang -and -not $ccExists) {
    Write-Warning "clang-cl not found; falling back to MSVC 'cl'"
    $isClang = $false
    $cc = 'cl'
    $linker = 'link'
  }
  $ccCmd = "$cc /nologo /W3 /O2 $defines $cxxflags /Fe`"$out`" $($files -join ' ') $includes $linkopts $libs"

  if ($ccExists) {
    Write-Host "Files:"; Write-Host ($files -join ', ')
    Write-Host "Includes: $includes"
    Write-Host "Libs: $libs"
    if ($useLocalRaylib) {
      # Fase 1: compilar objetos (evitar colisão de nomes usando /Fo por arquivo)
      $objRoot = $PWD.Path
      $objs = @()
      foreach ($f in $files) {
        $leaf = Split-Path $f -Leaf
        $sub = 'prj'
        if ($f -like "*\\third_party\\raylib\\src\\*") { $sub = 'rl' }
        elseif ($f -like "*\\third_party\\raygui\\src\\*") { $sub = 'rg' }
        $objName = "$sub" + '_' + ($leaf -replace '\.c$', '.obj')
        $objPath = Join-Path $objRoot $objName
        $compileCmd = "$cc /nologo /W3 /O2 $defines $cxxflags $includes /c `"$f`" /Fo`"$objPath`""
        Write-Host $compileCmd
        Invoke-Expression $compileCmd
        if ($LASTEXITCODE -ne 0) { Write-Error "Compilation failed for $f with exit code $LASTEXITCODE" }
        $objs += $objPath
      }
      # Fase 2: linkar objetos
      $linkOptsForLink = ($linkopts -replace '^\s*/link\s+', '')
      if ($linker -eq 'lld-link') {
        $linkCmd = "lld-link /OUT:`"$out`" $($objs -join ' ') $libs $linkOptsForLink"
      } else {
        $linkCmd = "link /nologo /OUT:`"$out`" $($objs -join ' ') $libs $linkOptsForLink"
      }
      Write-Host $linkCmd
      Write-Host $linkCmd
      Invoke-Expression $linkCmd
      if ($LASTEXITCODE -ne 0) { Write-Error "Link failed with exit code $LASTEXITCODE" }
      if (Test-Path -LiteralPath $out) {
        Write-Host "Executable created: $out"
      } else {
        Write-Warning "Link stage reported success but executable not found at $out"
      }
    }
    else {
      Write-Host $ccCmd
      Invoke-Expression $ccCmd
    }
  } else {
    # Tentar localizar VsDevCmd via vswhere (necessário para cabeçalhos/SDK e libs no Windows)
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio/Installer/vswhere.exe"
    if (-not (Test-Path $vswhere)) {
      Write-Error "Compiler '$cc' not found and 'vswhere.exe' is missing. Open the 'Developer Command Prompt for VS' and run this script."
    }
    $installPath = & $vswhere -latest -products * -property installationPath
    if (-not $installPath) {
      Write-Error "Visual Studio installation not found by vswhere. Please install Visual Studio Build Tools."
    }
    $vsDevCmd = Join-Path $installPath "Common7/Tools/VsDevCmd.bat"
    if (-not (Test-Path $vsDevCmd)) {
      Write-Error "VsDevCmd.bat not found at '$vsDevCmd'. Check your Build Tools installation."
    }
    $fullCmd = "`"$vsDevCmd`" -arch=amd64 -host_arch=amd64 && $ccCmd"
    Write-Host "Files:"; Write-Host ($files -join ', ')
    Write-Host "Includes: $includes"
    Write-Host "Libs: $libs"
    Write-Host "Activating VS environment and building..."
    Write-Host $fullCmd
    & cmd /c $fullCmd
    # Fallback: se o executável não existir, tentar link explícito dos .obj gerados
    if (-not (Test-Path -LiteralPath $out)) {
      Write-Warning "Executável não encontrado após build; tentando link manual."
      $objLeafs = @()
      foreach ($f in $files) { $objLeafs += (Split-Path $f -Leaf) -replace '\.c$', '.obj' }
      $linkOptsForLink = ($linkopts -replace '^\s*/link\s+', '')
      $objArgs = ($objLeafs -join ' ')
      if ($isClang -and ((Get-Command lld-link -ErrorAction SilentlyContinue) -ne $null)) {
        $linkerVs = 'lld-link'
      } else {
        $linkerVs = 'link'
      }
      $linkCmdVs = '"' + $vsDevCmd + '" -arch=amd64 -host_arch=amd64 && ' + $linkerVs + ' /OUT:"' + $out + '" ' + $objArgs + ' ' + $libs + ' ' + $linkOptsForLink
      Write-Host $linkCmdVs
      & cmd /c $linkCmdVs
      if (Test-Path -LiteralPath $out) { Write-Host "Executable created (fallback): $out" }
      else { Write-Error "Link fallback failed; executável ainda não encontrado: $out" }
    }
  }
}
Write-Host "Build finished (GUI). Executable at $out"