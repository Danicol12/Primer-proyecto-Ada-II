================================================================
  PROYECTO 1 - OPTIMIZACION DE RIEGO
  Analisis y Diseno de Algoritmos II (ADA II)
================================================================

----------------------------------------------------------------
  ESTRUCTURA DEL PROYECTO
----------------------------------------------------------------

  Proyecto/
  |-- Datos/
  |-- Interfaz/
  |   |-- consola/
  |   |-- gui/
  |-- Objetos/
  |-- ejecutar.bat
  |-- main.cpp

----------------------------------------------------------------
  CARPETA: Datos/
----------------------------------------------------------------

Contiene los archivos de entrada y salida del sistema.

  Datos/Entrantes/
      Archivos .txt con los casos de prueba. Cada archivo
      representa una finca. El formato es:

          n                    <- numero de tablones
          ts, tr, p, rp        <- datos de cada tablon (n lineas)

  Datos/Salidas/
      Archivos .txt generados automaticamente al resolver.
      El nombre del archivo sigue el patron:
          <nombre_entrada>_<algoritmo>.txt
      Ejemplo: finca1_FB.txt, finca1_PD.txt

      Formato de salida:
          <costo>              <- primera linea: costo total CR
          <indice_tablon>      <- una linea por tablon, en orden
          ...

----------------------------------------------------------------
  CARPETA: Interfaz/
----------------------------------------------------------------

Contiene las interfaces del sistema. Actualmente solo se usa
la interfaz grafica.

  Interfaz/gui/
      Interfaz grafica de escritorio construida con la API
      Win32 (Windows). Archivos principales:

      - gui.cpp        Codigo fuente completo de la GUI.
                       Maneja la ventana, los controles, la
                       carga de archivos, la ejecucion de
                       algoritmos y la presentacion de
                       resultados.

      - resources.rc   Manifiesto XML que activa los estilos
                       visuales modernos (ComCtl32 v6).
      - resources.res  Recurso compilado generado desde .rc

      La GUI permite:
        - Cargar archivos de Datos/Entrantes
        - Ver el contenido del archivo y los tablones
        - Elegir entre los algoritmos FB, Voraz, PD y Peor
        - Ver el costo, el orden de riego y el tiempo de
          ejecucion (medido con std::chrono)
        - Guardar automaticamente el resultado en Datos/Salidas

  Interfaz/consola/
      Interfaz de linea de comandos para ejecutar los
      algoritmos sin entorno grafico.

----------------------------------------------------------------
  CARPETA: Objetos/
----------------------------------------------------------------

Contiene las clases del dominio del problema.

  ============================================================
  CLASE Tablon  (Tablon.h / Tablon.cpp)
  ============================================================

  Representa un tablon de tierra que necesita ser regado.
  Cada tablon tiene cuatro atributos:

    ts  (tiempo_de_Supervivencia)
        Tiempo maximo antes de que el tablon muera si no ha
        sido regado. Si el regado termina despues de ts, hay
        penalizacion.

    tr  (tiempo_de_Regado)
        Duracion del proceso de regado de este tablon.

    p   (prioridad)
        Factor de penalizacion. Cuanto mayor sea, mas costoso
        es regarle tarde.

    rp  (tiempo_de_Riego_Perfecto)
        El instante de tiempo ideal para comenzar a regar
        este tablon. Si se empieza justo en rp, el costo se
        reduce a la mitad respecto al caso normal.

  --- Calculo de costo individual (calcularCosto(t)) ---

  Dado que el regado de este tablon comienza en el instante t:

    Si rp == t  (riego perfecto):
        costo = ts - (t + tr)

    Si (ts - tr) >= t  (termina antes de morir):
        costo = 2 * (ts - (t + tr))

    Si (ts - tr) < t  (termina despues de ts, penalizacion):
        costo = 2 * p * ((t + tr) - ts)

  El costo total de una programacion es la suma de los costos
  individuales de todos los tablones en el orden dado.

  --- Criterio voraz (valorVoraz()) ---

        valor = p / (tr * ts)

  Cuanto mayor sea este valor, mas urgente/prioritario es
  regar primero ese tablon.

  ============================================================
  CLASE Finca  (Finca.h / Finca.cpp)
  ============================================================

  Representa el conjunto de tablones de una finca. Contiene
  la logica de los cuatro algoritmos de optimizacion.

  La funcion central es:
      calcularCostoDeProgramacion(permutacion)
  Recibe un orden de riego (lista de indices) y devuelve el
  costo total de programacion segun la formula de Tablon.

  ============================================================
  ALGORITMOS IMPLEMENTADOS
  ============================================================

  1. FUERZA BRUTA / INGENUO  -  roFB()
  -----------------------------------------------------------
  Complejidad: O(n!)

  Genera todas las permutaciones posibles del conjunto de
  tablones y calcula el costo de cada una. Se queda con la
  permutacion que produce el MENOR costo total.

  Es el unico algoritmo que garantiza la solucion optima
  de forma exhaustiva, pero es impracticable para n > ~10
  ya que el numero de permutaciones crece factorialmente.

  2. PEOR COSTO  -  roFB_peor()
  -----------------------------------------------------------
  Complejidad: O(n!)

  Identico al anterior en mecanismo (recorre todas las
  permutaciones), pero en vez de quedarse con el minimo,
  se queda con la permutacion que produce el MAYOR costo.

  Sirve como referencia para conocer el peor escenario
  posible de programacion de riego.

  3. VORAZ (Greedy)  -  roV()
  -----------------------------------------------------------
  Complejidad: O(n log n)

  Ordena los tablones de mayor a menor segun su criterio
  voraz:
      valor = p / (tr * ts)

  Los tablones con mayor valor (alta prioridad, poco tiempo
  de regado y poca supervivencia) se riegan primero.

  No garantiza la solucion optima en todos los casos, pero
  es extremadamente rapido y suele dar buenos resultados.

  4. PROGRAMACION DINAMICA  -  roPD()
  -----------------------------------------------------------
  Complejidad: O(n^2 * 2^n)

  Usa bitmask DP: representa el subconjunto de tablones ya
  regados como una mascara de bits.

  Para cada estado (mascara), calcula el costo de agregar
  cada tablon aun no regado y guarda la decision optima.
  Al reconstruir las decisiones (choice[]) se obtiene el
  orden optimo de riego.

  Garantiza la solucion optima como Fuerza Bruta, pero es
  significativamente mas rapido (viable hasta ~20 tablones).

----------------------------------------------------------------
  COMPILACION Y EJECUCION
----------------------------------------------------------------

  Ejecutar el archivo:
      ejecutar.bat

  El script realiza automaticamente:
    1. Cierra cualquier instancia previa de gui.exe
    2. Compila Tablon.cpp
    3. Compila Finca.cpp
    4. Enlaza todo junto con la GUI y los recursos Win32
    5. Lanza gui.exe

  Requisitos:
    - MinGW-w64 (g++ con soporte C++17) instalado y en PATH
    - Entorno Windows (la GUI usa la API Win32)

----------------------------------------------------------------
  NOTAS
----------------------------------------------------------------

  - Los tiempos de ejecucion se miden con
    std::chrono::high_resolution_clock y se muestran en la
    interfaz pero NO se escriben en los archivos de salida.

  - El .gitignore excluye todos los .exe del repositorio.

================================================================
