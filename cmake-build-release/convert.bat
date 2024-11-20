@echo off
REM 检查output.tga是否存在
if not exist "output.tga" (
    echo output.tga 文件不存在.
    exit /b 1
)

REM 使用FFmpeg进行转换
ffmpeg -i "output.tga" "output.png"

REM 检查转换是否成功
if %errorlevel% neq 0 (
    echo 转换失败.
    exit /b 9009
) else (
    echo 转换成功: output.png
)