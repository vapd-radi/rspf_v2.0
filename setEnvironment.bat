@echo off

setx RSPF_DIR %cd%\
setx dev_path %RSPF_DIR%\project\Release;
setx path %path%;%dev_path%

echo ����������ϣ���Ҫ������ע������Ч
pause
exit