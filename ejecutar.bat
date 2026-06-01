@echo off
title Optimizacion de Riego - ADA II
chcp 65001 >nul

echo.
echo ========================================
echo   Optimizacion de Riego - ADA II
echo   Compilando interfaz grafica...
echo ========================================
echo.

:: Cerrar instancia anterior si esta corriendo
taskkill /F /IM gui.exe >nul 2>&1
if exist gui.exe del /F gui.exe

echo [1/3] Compilando Tablon...
g++ -std=c++17 -c Objetos/Tablon.cpp -o Tablon.o
if errorlevel 1 goto error

echo [2/3] Compilando Finca...
g++ -std=c++17 -c Objetos/Finca.cpp -o Finca.o
if errorlevel 1 goto error

echo [3/3] Compilando y enlazando GUI...
g++ -std=c++17 -I. -o gui.exe Interfaz/gui/gui.cpp Finca.o Tablon.o Interfaz/gui/resources.res -lcomctl32 -lcomdlg32 -lgdi32 -mwindows
if errorlevel 1 goto error

:: Limpiar archivos objeto temporales
del /F Tablon.o Finca.o >nul 2>&1

echo.
echo Compilacion exitosa. Iniciando GUI...
echo.
start "" gui.exe
goto end

:error
echo.
echo ERROR: Fallo la compilacion. Revisa los mensajes anteriores.
echo Asegurate de tener MinGW-w64 instalado y en el PATH.
echo.
del /F Tablon.o Finca.o >nul 2>&1
pause
exit /b 1

:end
