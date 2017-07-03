lupdate ..\ImgProcUI.pro

pause
linguist mx_zh.ts

lrelease mx_zh.ts

xcopy /Y mx_zh.qm ..\Res
pause