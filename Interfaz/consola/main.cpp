#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <functional>
#include "../../Objetos/Finca.h"

using namespace std;

// ─── Prototipos ───────────────────────────────────────────

vector<Tablon> leerArchivo(const string& ruta);
void pausa();
void mostrarDatos(const vector<Tablon>& tablones);
void mostrarResultado(const string& nombre, const pair<vector<int>, double>& res);
void verificarCosto(const vector<Tablon>& tablones, const vector<int>& permutacion, double costoEsperado);
void guardarSalida(const pair<vector<int>, double>& res, const string& ruta);
bool compararResultados(const vector<pair<string, pair<vector<int>, double>>>& resultados);

// ─── Menu principal ────────────────────────────────────────

int main() {
    vector<Tablon> tablones;
    string archivoActual;
    vector<pair<string, pair<vector<int>, double>>> resultados;

    while (true) {
        system("cls");

        cout << "╔══════════════════════════════════════════════════╗\n";
        cout << "║   SISTEMA DE OPTIMIZACION DE RIEGO — ADA        ║\n";
        cout << "╚══════════════════════════════════════════════════╝\n\n";

        if (!archivoActual.empty()) {
            cout << "Archivo: " << archivoActual
                 << "  |  Tablones: " << tablones.size() << "\n\n";
        } else {
            cout << "Archivo: (ninguno)  |  Tablones: ---\n\n";
        }

        cout << "  1. Cargar archivo de entrada\n";
        cout << "  2. Ver datos cargados\n";
        cout << "  --------------------------------\n";
        cout << "  3. Ejecutar Fuerza Bruta (roFB)\n";
        cout << "  4. Ejecutar Voraz (roV)\n";
        cout << "  5. Ejecutar Programacion Dinamica (roPD)\n";
        cout << "  6. Ejecutar TODOS los algoritmos\n";
        cout << "  --------------------------------\n";
        cout << "  7. Ver detalle de costo (paso a paso)\n";
        cout << "  8. Guardar resultado en archivo\n";
        cout << "  --------------------------------\n";
        cout << "  9. Comparar optimalidad (FB vs PD)\n";
        cout << "  0. Salir\n\n";
        cout << "Seleccione: ";

        string opcion;
        getline(cin, opcion);

        if (opcion == "0") {
            cout << "\n!Hasta luego!\n";
            break;
        }

        if (opcion == "1") {
            cout << "\nRuta del archivo: ";
            string ruta;
            getline(cin, ruta);
            if (ruta.empty()) ruta = "finca.txt";

            try {
                tablones = leerArchivo(ruta);
                archivoActual = ruta;
                resultados.clear();
                cout << "\nOK " << tablones.size() << " tablones cargados desde: " << ruta << "\n";
            } catch (const exception& e) {
                cout << "\nError al leer archivo: " << e.what() << "\n";
            }
            pausa();
        }
        else if (opcion == "2") {
            if (tablones.empty()) {
                cout << "\nNo hay datos cargados. Cargue un archivo primero (opcion 1).\n";
            } else {
                mostrarDatos(tablones);
            }
            pausa();
        }
        else if (opcion == "3" || opcion == "4" || opcion == "5" || opcion == "6" || opcion == "7" || opcion == "8" || opcion == "9") {
            if (tablones.empty()) {
                cout << "\nNo hay datos cargados. Cargue un archivo primero (opcion 1).\n";
                pausa();
                continue;
            }

            Finca finca(tablones);

            auto ejecutar = [&](const string& nombre, auto metodo) -> pair<string, pair<vector<int>, double>> {
                auto res = metodo(finca);
                return {nombre, res};
            };

            if (opcion == "3") {
                auto [nombre, res] = ejecutar("Fuerza Bruta (roFB)", [](Finca& f){ return f.roFB(); });
                resultados.push_back({nombre, res});
                mostrarResultado(nombre, res);
            }
            else if (opcion == "4") {
                auto [nombre, res] = ejecutar("Voraz (roV)", [](Finca& f){ return f.roV(); });
                resultados.push_back({nombre, res});
                mostrarResultado(nombre, res);
            }
            else if (opcion == "5") {
                auto [nombre, res] = ejecutar("PD (roPD)", [](Finca& f){ return f.roPD(); });
                resultados.push_back({nombre, res});
                mostrarResultado(nombre, res);
            }
            else if (opcion == "6") {
                resultados.clear();
                vector<pair<string, function<pair<vector<int>, double>(Finca&)>>> algos = {
                    {"Fuerza Bruta (roFB)",   [](Finca& f){ return f.roFB(); }},
                    {"Voraz (roV)",           [](Finca& f){ return f.roV(); }},
                    {"Programacion Dinamica (roPD)", [](Finca& f){ return f.roPD(); }},
                    {"roD (wrapper)",         [](Finca& f){ return f.roD(); }}
                };

                cout << "\n";
                for (auto& [nombre, metodo] : algos) {
                    auto res = metodo(finca);
                    resultados.push_back({nombre, res});
                    mostrarResultado(nombre, res);
                }

                cout << "\n======= RESUMEN COMPARATIVO =======\n";
                for (auto& [nombre, res] : resultados) {
                    cout << "  " << setw(28) << left << nombre
                         << " -> Costo: " << res.second << "\n";
                }
                cout << "====================================\n";

                if (resultados.size() >= 2) {
                    compararResultados(resultados);
                }
            }

            // ── Verificar costo paso a paso ────────────
            if (opcion == "7") {
                cout << "\nQue resultado desea verificar?\n";
                if (resultados.empty()) {
                    cout << "  (No hay resultados calculados aun)\n";
                    cout << "  Ejecute un algoritmo primero (opciones 3-6).\n";
                    pausa();
                    continue;
                }
                for (size_t i = 0; i < resultados.size(); i++) {
                    cout << "  " << (i+1) << ". " << resultados[i].first << "\n";
                }
                cout << "Seleccione: ";
                string sel;
                getline(cin, sel);
                int idx = stoi(sel) - 1;
                if (idx >= 0 && idx < (int)resultados.size()) {
                    verificarCosto(tablones, resultados[idx].second.first, resultados[idx].second.second);
                } else {
                    cout << "Opcion invalida.\n";
                }
            }

            // ── Guardar resultado ──────────────────────
            if (opcion == "8") {
                if (resultados.empty()) {
                    cout << "\nNo hay resultados. Ejecute un algoritmo primero.\n";
                    pausa();
                    continue;
                }
                cout << "\nQue resultado desea guardar?\n";
                for (size_t i = 0; i < resultados.size(); i++) {
                    cout << "  " << (i+1) << ". " << resultados[i].first << "\n";
                }
                cout << "Seleccione: ";
                string sel;
                getline(cin, sel);
                int idx = stoi(sel) - 1;
                if (idx >= 0 && idx < (int)resultados.size()) {
                    cout << "Nombre del archivo de salida: ";
                    string rutaOut;
                    getline(cin, rutaOut);
                    if (rutaOut.empty()) rutaOut = "salida.txt";
                    guardarSalida(resultados[idx].second, rutaOut);
                    cout << "\nOK Resultado guardado en: " << rutaOut << "\n";
                } else {
                    cout << "Opcion invalida.\n";
                }
            }

            // ── Comparar optimalidad ───────────────────
            if (opcion == "9") {
                resultados.clear();
                auto resFB = finca.roFB();
                auto resPD = finca.roPD();
                auto resV  = finca.roV();
                resultados = {
                    {"Fuerza Bruta (roFB)", resFB},
                    {"Voraz (roV)",         resV},
                    {"Programacion Dinamica (roPD)", resPD}
                };

                cout << "\n======= COMPARACION DE OPTIMALIDAD =======\n\n";
                mostrarResultado("Fuerza Bruta (roFB) - solucion exacta", resFB);
                mostrarResultado("Programacion Dinamica (roPD)", resPD);
                mostrarResultado("Voraz (roV) - heuristica", resV);

                cout << "\n--- VEREDICTO ---\n";
                if (resFB.second == resPD.second) {
                    cout << "[OK] FB y PD coinciden -> solucion OPTIMA encontrada.\n";
                    if (resV.second == resFB.second) {
                        cout << "[OK] El Voraz tambien encontro el optimo.\n";
                    } else {
                        cout << "[WARN] El Voraz NO encontro el optimo (costo: "
                             << resV.second << " vs " << resFB.second << ").\n";
                    }
                } else {
                    cout << "[ERROR] Diferencia entre FB y PD. Revise la implementacion.\n";
                    cout << "  FB: " << resFB.second << " | PD: " << resPD.second << "\n";
                }
            }

            pausa();
        }
        else {
            cout << "\nOpcion no valida. Intente de nuevo.\n";
            pausa();
        }
    }

    return 0;
}

// ─── Implementaciones ──────────────────────────────────────

vector<Tablon> leerArchivo(const string& ruta) {
    ifstream archivo(ruta);
    if (!archivo.is_open()) {
        throw runtime_error("No se pudo abrir el archivo: " + ruta);
    }

    int n;
    string linea;

    if (!getline(archivo, linea)) {
        throw runtime_error("Archivo vacio o primera linea invalida");
    }

    while (!linea.empty() && (linea[0] < '0' || linea[0] > '9')) {
        linea.erase(0, 1);
    }

    stringstream ss(linea);
    ss >> n;

    if (n <= 0) {
        throw runtime_error("El numero de tablones debe ser positivo");
    }

    vector<Tablon> tablones;
    int lineaActual = 0;

    while (getline(archivo, linea)) {
        if (lineaActual >= n) break;
        if (linea.empty()) continue;

        int ts, tr, p, rp;

        for (char& c : linea) {
            if (c == ',') c = ' ';
        }

        stringstream ssLinea(linea);
        if (ssLinea >> ts >> tr >> p >> rp) {
            tablones.emplace_back(ts, tr, p, rp);
            lineaActual++;
        } else {
            throw runtime_error("Error de formato en linea " + to_string(lineaActual + 2) + ": " + linea);
        }
    }

    if (tablones.size() != (size_t)n) {
        throw runtime_error("Se esperaban " + to_string(n) + " tablones, se leyeron " + to_string(tablones.size()));
    }

    archivo.close();
    return tablones;
}

void mostrarDatos(const vector<Tablon>& tablones) {
    cout << "\n======= DATOS DE LA FINCA =======\n";
    cout << "Total de tablones: " << tablones.size() << "\n\n";
    cout << left << setw(6) << "Indice"
         << setw(8) << "ts"
         << setw(8) << "tr"
         << setw(8) << "p"
         << setw(8) << "rp" << "\n";
    cout << string(38, '-') << "\n";

    for (size_t i = 0; i < tablones.size(); i++) {
        cout << left << setw(6) << i
             << setw(8) << tablones[i].getTiempoDeSupervivencia()
             << setw(8) << tablones[i].getTiempoDeRegado()
             << setw(8) << tablones[i].getPrioridad()
             << setw(8) << tablones[i].getTiempoDeRiegoPerfecto() << "\n";
    }
    cout << "===============================\n";
}

void mostrarResultado(const string& nombre, const pair<vector<int>, double>& res) {
    cout << "\n--- " << nombre << " ---\n";
    cout << "  Costo (CR): " << fixed << setprecision(0) << res.second << "\n";
    cout << "  Programacion (Pi): < ";
    for (size_t i = 0; i < res.first.size(); i++) {
        cout << res.first[i] << (i + 1 == res.first.size() ? "" : ", ");
    }
    cout << " >\n\n";
}

void verificarCosto(const vector<Tablon>& tablones, const vector<int>& permutacion, double costoEsperado) {
    cout << "\n======= VERIFICACION DE COSTO - PASO A PASO =======\n\n";

    if (permutacion.empty()) {
        cout << "Permutacion vacia.\n";
        return;
    }

    cout << "Permutacion: < ";
    for (size_t i = 0; i < permutacion.size(); i++) {
        cout << permutacion[i] << (i + 1 == permutacion.size() ? "" : ", ");
    }
    cout << " >\n\n";

    double costoAcum = 0;
    int tiempo = 0;

    for (size_t paso = 0; paso < permutacion.size(); paso++) {
        int idx = permutacion[paso];
        const Tablon& t = tablones[idx];

        int ts = t.getTiempoDeSupervivencia();
        int tr = t.getTiempoDeRegado();
        int p  = t.getPrioridad();
        int rp = t.getTiempoDeRiegoPerfecto();

        cout << "Paso " << (paso + 1) << ": Tablon " << idx
             << "  (ts=" << ts << ", tr=" << tr << ", p=" << p << ", rp=" << rp << ")\n";
        cout << "  t = " << tiempo << "\n";

        int costoPaso = 0;
        string rama;

        if (rp == tiempo) {
            costoPaso = ts - (tiempo + tr);
            rama = "rp == t -> Costo = ts - (t + tr) = "
                   + to_string(ts) + " - (" + to_string(tiempo) + " + " + to_string(tr) + ")"
                   + " = " + to_string(costoPaso);
        } else if ((ts - tr) >= tiempo) {
            costoPaso = 2 * (ts - (tiempo + tr));
            rama = "(ts - tr) >= t  -> Costo = 2 * (ts - (t + tr)) = 2 * ("
                   + to_string(ts) + " - (" + to_string(tiempo) + " + " + to_string(tr) + "))"
                   + " = " + to_string(costoPaso);
        } else {
            costoPaso = 2 * p * ((tiempo + tr) - ts);
            rama = "(ts - tr) < t  -> Costo = 2 * p * ((t + tr) - ts) = 2 * "
                   + to_string(p) + " * ((" + to_string(tiempo) + " + " + to_string(tr) + ") - "
                   + to_string(ts) + "))"
                   + " = " + to_string(costoPaso);
        }

        cout << "  -> " << rama << "\n";
        cout << "  Costo parcial: " << costoPaso << "\n";

        costoAcum += costoPaso;
        tiempo += tr;

        cout << "  Costo acumulado: " << costoAcum << "\n";
        cout << "  t += tr -> t = " << tiempo << "\n\n";
    }

    cout << string(50, '=') << "\n";
    cout << "Costo TOTAL calculado: " << costoAcum << "\n";
    cout << "Costo esperado:        " << fixed << setprecision(0) << costoEsperado << "\n";

    if (costoAcum == costoEsperado) {
        cout << "[OK] VERIFICACION EXITOSA - Los costos coinciden.\n";
    } else {
        cout << "[ERROR] El costo calculado no coincide con el esperado.\n";
    }
    cout << string(50, '=') << "\n";
}

void guardarSalida(const pair<vector<int>, double>& res, const string& ruta) {
    ofstream archivo(ruta);
    if (!archivo.is_open()) {
        cout << "Error: No se pudo crear el archivo: " << ruta << "\n";
        return;
    }

    archivo << fixed << setprecision(0) << res.second << "\n";
    for (int idx : res.first) {
        archivo << idx << "\n";
    }

    archivo.close();
}

bool compararResultados(const vector<pair<string, pair<vector<int>, double>>>& resultados) {
    if (resultados.size() < 2) return true;

    double costoRef = resultados[0].second.second;
    bool todosOk = true;

    for (size_t i = 1; i < resultados.size(); i++) {
        if (resultados[i].second.second != costoRef) {
            todosOk = false;
            break;
        }
    }

    cout << "\n--- VERIFICACION DE OPTIMALIDAD ---\n";
    if (todosOk) {
        cout << "[OK] TODOS los algoritmos coinciden en el mismo costo: "
             << fixed << setprecision(0) << costoRef << "\n";
    } else {
        cout << "[WARN] Los algoritmos NO coinciden. Revise diferencias arriba.\n";
    }

    return todosOk;
}

void pausa() {
    cout << "\nPresione Enter para continuar...";
    string dummy;
    getline(cin, dummy);
}
