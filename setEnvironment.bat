@echo off

setx RSPF_DIR %cd%\
setx dev_path %RSPF_DIR%\project\Release;
setx path %path%;%dev_path%

echo 环境设置完毕，需要重启或注销后生效
pause
exit