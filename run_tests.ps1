Write-Host "Starting PowerShell test script..."
Write-Host ""

$compilerExe = "D:\Compiler\Compiler\build\Debug\Compiler.exe"
$testDir = "D:\Compiler\Compiler\compiler_inputs"

Get-ChildItem -Path $testDir -Filter "*.tc" | ForEach-Object {
    Write-Host "=======================================" -ForegroundColor Green
    Write-Host "Testing file: $($_.Name)" -ForegroundColor Green
    Write-Host "=======================================" -ForegroundColor Green

    # 调用你的编译器
    & $compilerExe $_.FullName

    Write-Host ""
}

Write-Host "All tests finished."
Read-Host "Press Enter to exit"