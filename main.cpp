#include <iostream>
#include <vector>
#include <string>
#include "Objetos/Finca.h"
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

    auto print_result = [&](const string& name, const pair<vector<int>, double>& r){
        cout << "--- " << name << " ---" << endl;
        cout << "Costo: " << r.second << endl;
        cout << "Permutacion: < ";
        for(size_t i=0;i<r.first.size();i++){
            cout << r.first[i] << (i+1==r.first.size()?"":" , ");
        }
        cout << " >\n\n";
    };

    cout << "Numero de tablones: " << miFinca.numeroDeTablones() << "\n\n";

    // Ejecutar los 4 algoritmos y mostrar resultados
    auto resFB = miFinca.roFB();
    print_result("Fuerza Bruta (roFB)", resFB);

    auto resV = miFinca.roV();
    print_result("Voraz (roV)", resV);

    auto resPD = miFinca.roPD();
    print_result("Programacion Dinamica (roPD)", resPD);

    auto resD = miFinca.roD();
    print_result("roD (wrapper)", resD);

    // Comparacion rápida
    cout << "Resumen comparativo de costos:\n";
    cout << "  roFB: " << resFB.second << " | roV: " << resV.second << " | roPD: " << resPD.second << " | roD: " << resD.second << "\n";

    return 0;
}