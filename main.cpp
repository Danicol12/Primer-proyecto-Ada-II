#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "Objetos/Finca.h"

using namespace std;

vector<Tablon> leerArchivo(const string& ruta) {
    ifstream archivo(ruta);
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir " << ruta << endl;
        exit(1);
    }

    int n;
    string linea;
    getline(archivo, linea);
    stringstream(linea) >> n;

    vector<Tablon> tablones;
    while (getline(archivo, linea)) {
        if ((int)tablones.size() >= n) break;
        if (linea.empty()) continue;
        for (char& c : linea) if (c == ',') c = ' ';

        int ts, tr, p, rp;
        stringstream ss(linea);
        if (ss >> ts >> tr >> p >> rp)
            tablones.emplace_back(ts, tr, p, rp);
    }
    return tablones;
}

void printResultado(const string& nombre, const pair<vector<int>, double>& res) {
    cout << left << setw(32) << nombre
         << "Costo: " << fixed << setprecision(0) << setw(6) << res.second
         << "  Pi: < ";
    for (size_t i = 0; i < res.first.size(); i++)
        cout << res.first[i] << (i+1 < res.first.size() ? ", " : "");
    cout << " >" << endl;
}

int main() {
    auto tablones = leerArchivo("finca.txt");
    Finca finca(tablones);

    cout << "Tabones cargados: " << tablones.size() << "\n\n";

    auto resFB  = finca.roFB();
    auto resV   = finca.roV();
    auto resPD  = finca.roPD();

    printResultado("Fuerza Bruta (roFB)", resFB);
    printResultado("Voraz (roV)",         resV);
    printResultado("PD (roPD)",           resPD);

    cout << "\nOptimalidad: "
         << (resFB.second == resPD.second ? "OK" : "ERROR")
         << " (FB vs PD)"
         << (resV.second == resFB.second ? " — Voraz tambien optimo" : "") << endl;

    return 0;
}
