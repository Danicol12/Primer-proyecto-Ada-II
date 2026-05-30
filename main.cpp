#include "./Objetos/Tablon.h"
#include "./Objetos/Finca.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    // 1. Creamos los tablones del ejemplo del PDF: <ts, tr, p, ro>
    vector<Tablon> listaTablones;
    listaTablones.push_back(Tablon(10, 3, 4, 0)); // T0
    listaTablones.push_back(Tablon(6, 3, 3, 1));  // T1
    listaTablones.push_back(Tablon(2, 2, 1, 0));  // T2
    listaTablones.push_back(Tablon(8, 1, 1, 6));  // T3
    listaTablones.push_back(Tablon(10, 4, 2, 5)); // T4

    // 2. Inicializamos la Finca con estos tablones
    Finca miFinca(listaTablones);

    cout << "Holaaaa"<< endl;
    cout << "--- Iniciando prueba de Fuerza Bruta (roFB) ---" << endl;
    cout << "Numero de tablones a procesar: " << miFinca.numeroDeTablones() << endl;

    // 3. Ejecutamos el algoritmo de Fuerza Bruta
    pair<vector<int>, double> resultado = miFinca.roFB();

    // 4. Mostramos los resultados
    cout << "\nRESULTADO OPTIMO ENCONTRADO:" << endl;
    cout << "Costo Minimo (CR): " << resultado.second << endl;
    
    cout << "Programacion Optima (Pi): < ";
    for (int i = 0; i < resultado.first.size(); i++) {
        cout << resultado.first[i] << (i == resultado.first.size() - 1 ? "" : ", ");
    }
    cout << " >" << endl;

    return 0;
}
