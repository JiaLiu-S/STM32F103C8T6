@echo off
setlocal
"%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /target:winexe /codepage:65001 /out:"%~dp0VillaSecurityMonitor.exe" /r:System.Windows.Forms.dll /r:System.Drawing.dll "%~dp0Program.cs" "%~dp0Protocol.cs"
if errorlevel 1 pause
