#include "./Objetos/Tablon.h"
#include "./Objetos/Finca.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    vector<Tablon> listaTablones;
listaTablones.push_back(Tablon(7, 2, 3, 0));  // T0
listaTablones.push_back(Tablon(5, 2, 4, 1));  // T1
listaTablones.push_back(Tablon(3, 1, 2, 0));  // T2
listaTablones.push_back(Tablon(9, 3, 1, 5));  // T3
listaTablones.push_back(Tablon(12, 4, 2, 6)); // T4

    Finca miFinca(listaTablones);

    cout << "Holaaaa111"<< endl;
    cout << "--- Iniciando prueba de Fuerza Bruta (roFB) ---" << endl;
    cout << "Numero de tablones a procesar: " << miFinca.numeroDeTablones() << endl;

    pair<vector<int>, double> resultado = miFinca.roFB();

    cout << "\nRESULTADO OPTIMO ENCONTRADO:" << endl;
    cout << "Costo Minimo (CR): " << resultado.second << endl;
    
    cout << "Programacion Optima (Pi): < ";
    for (int i = 0; i < resultado.first.size(); i++) {
        cout << resultado.first[i] << (i == resultado.first.size() - 1 ? "" : ", ");
    }
    cout << " >" << endl;

    cout << "\n--- Iniciando prueba Voraz (roV) ---" << endl;

    pair<vector<int>, double> resultadoV = miFinca.roV();

    cout << "\nRESULTADO VORAZ ENCONTRADO:" << endl;
    cout << "Costo (CR): " << resultadoV.second << endl;

    cout << "Programacion Voraz (Pi): < ";
    for (int i = 0; i < resultadoV.first.size(); i++) {
        cout << resultadoV.first[i] << (i == resultadoV.first.size() - 1 ? "" : ", ");
    }
    cout << " >" << endl;

    return 0;
}