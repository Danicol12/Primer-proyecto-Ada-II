# Sistema de Optimización de Riego — ADA

## Estructura del Proyecto

```
Primer-proyecto-Ada-II/
│
├── main.exe               ← Core: ejecuta los 4 algoritmos y muestra resultados
├── main_consola.exe        ← Interfaz de terminal (menú interactivo)
├── main_gui.exe            ← Interfaz gráfica (ventana Win32)
│
├── main.cpp                ← Código fuente del core
├── finca.txt               ← Archivo de datos de entrada (formato: ver sección 3.4.1)
│
├── Interfaz/
│   ├── consola/
│   │   └── main.cpp        ← Código fuente de la interfaz de terminal
│   ├── gui/
│   │   ├── gui.cpp         ← Código fuente de la interfaz gráfica
│   │   ├── gui.exe.manifest ← Manifiesto de estilos visuales (XML)
│   │   └── resources.rc    ← Script de recursos (embed del manifest)
│   └── web/
│       ├── server.cpp      ← Servidor HTTP embebido (Winsock)
│       ├── index.html      ← Frontend web moderno
│       ├── style.css       ← Estilos oscuros tipo app moderna
│       └── app.js          ← Lógica del frontend
│
├── Objetos/
│   ├── Finca.h / Finca.cpp ← Clase Finca (lógica de los algoritmos)
│   ├── Tablon.h / Tablon.cpp ← Clase Tablon (representación de un tablón)
│
├── .vscode/
│   ├── tasks.json           ← Tasks de compilación para VS Code
│   ├── c_cpp_properties.json ← Configuración de IntelliSense
│   └── settings.json       ← Configuración del editor
├── INSTRUCCIONES.md         ← Este archivo
├── Descomposicion.md        ← Explicación teórica de la PD
└── Explicacion_roD.md       ← Documentación de la solución dinámica
```

---

## Cómo ejecutar

Hay **4 formas** de ejecutar el programa, según lo que necesites:

### 1. Core rápido (`main.exe`)

Ejecuta los 4 algoritmos automáticamente y muestra resultados en consola:

```
main.exe
```

**Salida típica:**

```
Tabones cargados: 5

Fuerza Bruta (roFB)             Costo: 20      Pi: < 2, 1, 0, 3, 4 >
Voraz (roV)                     Costo: 20      Pi: < 2, 1, 0, 3, 4 >
PD (roPD)                       Costo: 20      Pi: < 2, 1, 0, 3, 4 >
roD (wrapper)                   Costo: 20      Pi: < 2, 1, 0, 3, 4 >

Optimalidad: OK (FB vs PD) — Voraz tambien optimo
```

Ideal para: verificar rápido que todo funciona.

### 2. Interfaz de terminal (`main_consola.exe`)

Menú interactivo con todas las opciones:

```
main_consola.exe
```

**Opciones del menú:**

| Opción | Función |
|--------|---------|
| 1 | Cargar archivo de entrada |
| 2 | Ver datos cargados en pantalla |
| 3 | Ejecutar Fuerza Bruta (roFB) |
| 4 | Ejecutar Voraz (roV) |
| 5 | Ejecutar Programación Dinámica (roPD) |
| 6 | Ejecutar TODOS los algoritmos + comparación |
| 7 | Ver detalle de costo paso a paso |
| 8 | Guardar resultado en archivo |
| 9 | Comparar optimalidad (FB vs PD vs Voraz) |
| 0 | Salir |

**Opción 7 — Detalle paso a paso:**

Muestra el cálculo completo de cada tablón en la permutación:
1. Valores del tablón (ts, tr, p, rp)
2. Rama de `calcularCosto` que se ejecutó (rp==t, (ts-tr)>=t, o (ts-tr)<t)
3. Fórmula completa con números reemplazados
4. Costo parcial y acumulado
5. Verificación final contra el costo esperado

### 3. Interfaz gráfica (`main_gui.exe`)

Ventana nativa de Windows con controles visuales:

```
main_gui.exe
```

**Funcionalidades:**
- **Cargar archivo**: Botón o menú Archivo > Abrir — abre el diálogo nativo de Windows
- **Tabla de datos**: Muestra los tablones en columnas (#, ts, tr, p, rp)
- **Botones de algoritmos**: FB, Voraz, PD, Todos
- **Verificar**: Muestra el detalle paso a paso de cada rama de `calcularCosto`
- **Guardar**: Guarda el resultado en formato estándar (ver sección 3.4.2)
- **Panel de resultados**: Texto con fuente monoespaciada para fácil lectura

### 4. Interfaz web (`riego.exe`) ⭐ RECOMENDADA

Aplicación web moderna con servidor HTTP embebido. Se abre automáticamente en el navegador:

```
riego.exe
```

Esto abre `http://localhost:9174` en tu navegador predeterminado con una interfaz moderna tipo app.

**Funcionalidades:**
- **🎯 Diseño moderno**: Tema oscuro, colores neón, animaciones suaves, tipografía limpia
- **📁 Cargar archivo**: Selector de archivos nativo del navegador (cualquier `.txt`)
- **📊 Tabla de datos**: Grid estilizado con los tablones cargados
- **⚡ Algoritmos**: Botones FB, Voraz, PD, Todos — con feedback visual y timer
- **🔍 Verificación paso a paso**: Modal interactivo con detalle de cada paso, rama, y fórmula
- **💾 Descarga de resultados**: Descarga directa del archivo de salida (formato 3.4.2)
- **📱 Responsive**: Se adapta a cualquier tamaño de pantalla

**Ventajas sobre las otras interfaces:**
- Se ve como una aplicación web moderna (no como un programa del 95)
- Cero dependencias externas (Winsock + navegador ya están en Windows)
- Se puede modificar el HTML/CSS/JS sin recompilar
- Timer de ejecución integrado
- Feedback visual inmediato en todas las acciones

**Nota:** El servidor usa Winsock (API de sockets de Windows, presente desde Windows 95).
No requiere instalar nada adicional — Windows 10/11 ya tiene todo lo necesario.

---

## Formato de archivos

### Formato de entrada (sección 3.4.1 del enunciado)

```
n
ts0,tr0,p0,rp0
ts1,tr1,p1,rp1
...
ts(n-1),tr(n-1),p(n-1),rp(n-1)
```

- Primera línea: número de tablones (n)
- Siguientes n líneas: valores separados por coma
- Ejemplo (`finca.txt`):
  ```
  5
  10,3,4,0
  6,3,3,1
  2,2,1,0
  8,1,1,6
  10,4,2,5
  ```

### Formato de salida (sección 3.4.2 del enunciado)

```
Costo
pi0
pi1
...
pi(n-1)
```

- Primera línea: costo de la solución
- Siguientes n líneas: índices de los tablones en orden de riego
- Ejemplo:
  ```
  20
  2
  1
  0
  3
  4
  ```

---

## Compilación

### Requisitos

- Compilador: **MinGW-w64** (g++) — el que viene con MSYS2
- Estándar: **C++17**
- Sistema operativo: **Windows** (la GUI usa Win32 API)

### Comandos de compilación

```bash
# === Core (main.exe) ===
g++ -std=c++17 -Wall -Wextra main.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o main.exe

# === Interfaz de terminal (main_consola.exe) ===
g++ -std=c++17 -Wall -Wextra Interfaz/consola/main.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o main_consola.exe

# === Interfaz gráfica (main_gui.exe) ===
windres Interfaz/gui/resources.rc -O coff -o Interfaz/gui/resources.res
g++ -std=c++17 -Wall -Wextra Interfaz/gui/gui.cpp Interfaz/gui/resources.res Objetos/Finca.cpp Objetos/Tablon.cpp -o main_gui.exe -lcomctl32 -lcomdlg32 -mwindows

# === Servidor Web (riego.exe) ⭐ RECOMENDADA ===
g++ -std=c++17 -Wall -Wextra Interfaz/web/server.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o riego.exe -lws2_32
```

### Compilar todo de una vez (Windows PowerShell)

```powershell
g++ -std=c++17 -Wall -Wextra main.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o main.exe; if ($?) { g++ -std=c++17 -Wall -Wextra Interfaz/consola/main.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o main_consola.exe }; if ($?) { windres Interfaz/gui/resources.rc -O coff -o Interfaz/gui/resources.res }; if ($?) { g++ -std=c++17 -Wall -Wextra Interfaz/gui/gui.cpp Interfaz/gui/resources.res Objetos/Finca.cpp Objetos/Tablon.cpp -o main_gui.exe -lcomctl32 -lcomdlg32 -mwindows }; if ($?) { g++ -std=c++17 -Wall -Wextra Interfaz/web/server.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o riego.exe -lws2_32 }
```

### Compilar todo de una vez (CMD)

```cmd
g++ -std=c++17 -Wall -Wextra main.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o main.exe && g++ -std=c++17 -Wall -Wextra Interfaz/consola/main.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o main_consola.exe && windres Interfaz/gui/resources.rc -O coff -o Interfaz/gui/resources.res && g++ -std=c++17 -Wall -Wextra Interfaz/gui/gui.cpp Interfaz/gui/resources.res Objetos/Finca.cpp Objetos/Tablon.cpp -o main_gui.exe -lcomctl32 -lcomdlg32 -mwindows && g++ -std=c++17 -Wall -Wextra Interfaz/web/server.cpp Objetos/Finca.cpp Objetos/Tablon.cpp -o riego.exe -lws2_32
```

### Desde VS Code

Usar `Ctrl+Shift+B` y elegir entre las tasks configuradas:
- **Compilar Core** — solo el core (main.exe)
- **Compilar Interfaz CONSOLA** — menú de terminal
- **Compilar Interfaz GUI (Win32)** — ventana Win32 (corre automáticamente `windres` como dependiente)
- **Compilar Servidor Web** — interfaz web moderna (riego.exe)

### Notas importantes sobre compilación

- **Web**: El flag `-lws2_32` es necesario para Winsock (sockets de Windows)
- **GUI**: Requiere compilar el recurso `.rc` primero con `windres` para embeber el manifiesto de estilos visuales
- **GUI**: El flag `-lcomctl32 -lcomdlg32` es necesario (common controls y diálogos)
- **GUI**: El flag `-mwindows` oculta la ventana de consola al ejecutar la GUI
- **Web**: El servidor busca los archivos estáticos en `Interfaz/web/` relativo al directorio del ejecutable
- Todos los ejecutables se generan en la raíz del proyecto

---

## Notas sobre la interfaz gráfica

### Dependencias

**Cero dependencias externas.** La GUI usa exclusivamente la API nativa de Windows (Win32 API), que está disponible en todas las versiones de Windows desde Windows 95 en adelante. No requiere instalar Qt, GTK, ni ninguna librería adicional.

### Visual Styles

La GUI usa los estilos visuales modernos de Windows (ComCtl32 v6). Si se ejecuta en Windows 10/11, los botones y controles se ven con el estilo nativo del sistema operativo.

### Verificación de costo paso a paso

Al hacer clic en "Verificar", la GUI muestra:
1. La permutación completa del resultado seleccionado
2. Para cada tablón: todas las variables (ts, tr, p, rp), el tiempo actual, la rama del costo que se aplicó
3. La fórmula completa con los números reemplazados
4. El costo parcial y acumulado después de cada paso
5. Una verificación final de que el costo calculado coincide con el reportado

---

## Notas sobre la interfaz web

### ¿Cómo funciona?

`riego.exe` levanta un **servidor HTTP** en `localhost:9174` usando la API de sockets de Windows (Winsock).
Cuando iniciás el programa:

1. Se crea un socket TCP en el puerto 9174 (solo accesible desde tu PC)
2. Se abre tu navegador predeterminado en `http://localhost:9174`
3. El navegador carga `index.html`, `style.css` y `app.js`
4. Cada clic en la interfaz envía una solicitud `fetch()` al servidor
5. El servidor procesa (corre algoritmos, calcula costos, etc.) y devuelve JSON
6. La interfaz se actualiza dinámicamente sin recargar la página

### API REST del servidor

| Ruta | Método | Descripción |
|------|--------|-------------|
| `/api/status` | GET | Estado del servidor y número de tablones cargados |
| `/api/load` | POST | Carga datos desde el body (texto plano) |
| `/api/run` | POST | Ejecuta un algoritmo (`fb`, `voraz`, `pd`, o `todos`) |
| `/api/verify` | POST | Verificación paso a paso del costo |
| `/api/download` | POST | Descarga el resultado como archivo de texto |

### Personalizar la apariencia

Podés modificar `Interfaz/web/style.css` para cambiar colores, fuentes, layout, etc.
Los cambios se ven **sin recompilar** — solo refrescás la página del navegador.

---

## Algoritmos disponibles

| Algoritmo | Función | Tipo |
|-----------|---------|------|
| Fuerza Bruta | `roFB()` | Exacto — prueba todas las permutaciones (n!) |
| Voraz | `roV()` | Heurístico — criterio: prioridad / (tr × ts) |
| PD (máscaras de bits) | `roPD()` | Exacto — programación dinámica O(n·2^n) |
| D (wrapper) | `roD()` | Llama a `roPD()` — existe como alias |

---

## Solución de problemas

| Problema | Posible causa | Solución |
|----------|---------------|----------|
| "No se pudo abrir el archivo" | El archivo no está en el directorio correcto | Ejecutar desde la raíz del proyecto donde está `finca.txt` |
| La GUI no se abre o se cierra | Falta `-lcomctl32 -lcomdlg32 -mwindows` al compilar | Recompilar con los flags correctos |
| Resultados inconsistentes | Cambios en el core sin recompilar | Recompilar TODO después de modificar `Objetos/` |
| La interfaz gráfica se ve "antigua" | Sin estilos visuales | Asegurarse de tener Windows 10+ o incluir manifest |
| La interfaz web no se abre | Puerto 9174 ocupado | Cerrar otros programas o cambiar el puerto en `server.cpp` |
| La interfaz web no carga los estilos | Los archivos `web/` no están junto al .exe | Ejecutar desde la raíz del proyecto o copiar `Interfaz/web/` al lado del .exe |
| Error de compilación `'function' not declared` | Falta `#include <functional>` | Agregar el include o compilar con C++17 |
| "Error de conexión" en la web | El servidor no está corriendo | Ejecutar `riego.exe` primero y esperar a que abra el navegador |
| IntelliSense marca errores en rojo | Falta configuración del C++ extension | Ya incluimos `c_cpp_properties.json` — reiniciar VS Code |
