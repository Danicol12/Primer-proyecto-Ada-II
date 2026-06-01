================================================================
  PROYECTO 1 - OPTIMIZACION DE RIEGO  |  ADA II
================================================================

ESTRUCTURA
----------
  Proyecto/
  |-- Datos/
  |   |-- Entrantes/   Archivos .txt con los casos de prueba(Colocar aquí los archivos .txt para ser probados)
  |   |-- Salidas/     Resultados generados automaticamente
  |-- Interfaz/gui/    Interfaz grafica Win32
  |-- Objetos/         Clases Tablon y Finca
  |-- ejecutar.bat     Script de compilacion y ejecucion

FORMATO DE ENTRADA
------------------
  Primera linea: numero de tablones (n)
  Siguientes n lineas: ts, tr, p, rp  (separados por comas)

FORMATO DE SALIDA
-----------------
  Primera linea: costo total de la solucion
  Siguientes n lineas: indices de los tablones en orden de riego
  Nombre del archivo: <entrada>_<algoritmo>.txt
  Ejemplo: finca1_FB.txt, finca1_PD.txt

CLASES PRINCIPALES
------------------
  Tablon    Representa un tablon con sus atributos (ts, tr, p, rp)
            y calcula su costo individual dado un tiempo de inicio.

  Finca     Contiene el conjunto de tablones e implementa los
            cuatro algoritmos de optimizacion.

ALGORITMOS
----------
  roFB()        Fuerza Bruta      O(n!)
                Genera todas las permutaciones y devuelve la de
                menor costo. Optimo garantizado, viable hasta n~10.

  roFB_peor()   Peor caso         O(n!)
                Igual que FB pero devuelve la permutacion de mayor
                costo. Util como referencia del peor escenario.

  roV()         Voraz             O(n log n)
                Ordena los tablones por el criterio p/(tr*ts).
                No garantiza el optimo pero es muy rapido.

  roPD()        Prog. Dinamica    O(n^2 * 2^n)
                Usa bitmask DP para encontrar el orden optimo.
                Garantiza el optimo y es viable hasta n~20.

COMPILACION Y EJECUCION
-----------------------
  Ejecutar el archivo ejecutar.bat. 
  
  Utiliza el comando:

  ./ejecutar.bat
  
  El script compila
  automaticamente Tablon.cpp, Finca.cpp y la GUI, luego
  lanza gui.exe.
  Requisitos: MinGW-w64 (g++ C++17) instalado en PATH,
  entorno Windows.

================================================================