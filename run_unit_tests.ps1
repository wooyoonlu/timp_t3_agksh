$ErrorActionPreference = "Stop"

$unitTestDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDirectory = Join-Path $unitTestDirectory "_build"
$releaseDirectory = Join-Path $buildDirectory "release"
$projectFile = Join-Path $unitTestDirectory "UnitTest.pro"
$resultFile = Join-Path $unitTestDirectory "UnitTest_Result.txt"
$localResultFile = Join-Path $releaseDirectory "UnitTest_Result.txt"

$qtBinDirectory = "C:\msys64\mingw64\bin"
$qmake = Join-Path $qtBinDirectory "qmake6.exe"
$make = Join-Path $qtBinDirectory "mingw32-make.exe"
$testExecutable = Join-Path $releaseDirectory "ServerParseUnitTest.exe"

$env:Path = "$qtBinDirectory;$env:Path"
New-Item -ItemType Directory -Path $buildDirectory -Force | Out-Null

Push-Location $buildDirectory
try {
    & $qmake $projectFile
    if ($LASTEXITCODE -ne 0) {
        throw "qmake failed with exit code $LASTEXITCODE"
    }

    & $make -j2
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
} finally {
    Pop-Location
}

Push-Location $releaseDirectory
try {
    Remove-Item -LiteralPath $localResultFile -ErrorAction SilentlyContinue
    & $testExecutable -o UnitTest_Result.txt,txt
    $testExitCode = $LASTEXITCODE
    Copy-Item -LiteralPath $localResultFile -Destination $resultFile -Force
    Get-Content -LiteralPath $localResultFile
} finally {
    Pop-Location
}

exit $testExitCode
