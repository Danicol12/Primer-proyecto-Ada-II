@echo off
title Optimizacion de Riego - ADA

echo.
echo ========================================
echo   Optimizacion de Riego - ADA
echo   Compilando y ejecutando...
echo ========================================
echo.

echo [1/2] Compilando servidor web...
g++ -std=c++17 -Wall -Wextra Interfaz/web/server.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o riego.exe -lws2_32
if errorlevel 1 goto error

echo [2/2] Iniciando interfaz web...
echo.
echo La aplicacion se abrira en tu navegador.
echo Si no se abre, anda a: http://localhost:9174
echo.
echo Presiona Ctrl+C en esta ventana para cerrar el servidor.
echo.

riego.exe
goto end

:error
echo.
echo ERROR: No se pudo compilar.
echo Asegurate de tener MinGW-w64 instalado.
echo.
pause
exit /b 1

:end
pause

