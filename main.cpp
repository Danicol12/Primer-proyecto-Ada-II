#include <iostream>
#include <vector>
#include <fstream>
#include "Objetos/Finca.h"




int main() {
    std::vector<Tablon> tablones;

    //Abrir el archivo txt
    std::ifstream archivo("finca.txt");

    //Verificar que el archivo se abrio correctamente
    if (!archivo.is_open()) {
        std::cout << "Error: No se pudo abrir el archivo" << std::endl;
        return 1;
    }

    //Creación De tablón para guardar los datos de la finca
    int ts, tr, p, rp;

    //Leer los datos del txt
    while (archivo >> ts >> tr >> p >> rp) {
        tablones.emplace_back(ts, tr, p, rp);
    }

    //Cerrar el archivo
    archivo.close();

    Finca finca(tablones);

    //Imprimir los datos de la finca
    for (int i = 0; i < finca.numeroDeTablones(); i++) {
        Tablon t = finca.getTablon(i);
        std::cout << "ts: " << t.getTiempoDeSupervivencia()
                  << " tr: " << t.getTiempoDeRegado()
                  << " p: " << t.getPrioridad()
                  << " rp: " << t.getTiempoDeRiegoPerfecto() << std::endl;
    }

    // Solución con Programación Dinámica (máscaras de bits)
    std::cout << "\n--- Iniciando prueba de Programacion Dinamica (roPD) ---" << std::endl;
    auto resultadoPD = finca.roPD();
    std::cout << "Costo Minimo (CR): " << resultadoPD.second << std::endl;
    std::cout << "Programacion Optima (Pi): < ";
    for (size_t i = 0; i < resultadoPD.first.size(); i++) {
        std::cout << resultadoPD.first[i]
                  << (i + 1 == resultadoPD.first.size() ? "" : ", ");
    }
    std::cout << " >" << std::endl;

    return 0;
}
