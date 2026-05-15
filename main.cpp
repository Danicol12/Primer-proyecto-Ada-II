#include <iostream>
#include <vector>
#include <fstream>
#include "Objetos/Tablon.h"

//Función de costo 
int calcularCosto(Tablon t, int tiempoActual){
    int tiempoFin = tiempoActual + t.tr;
    if(tiempoActual == t.ro){
        return t.ts-(tiempoFin);
    }
    else if(tiempoFin <= t.ts){
        return 2*(t.ts-(tiempoFin));
    }
    else{
        return 2*t.p * ((tiempoFin)-t.ts);
    }
}

//Función que se encarga de explorar las rutas posibles 
explorarRutas(std::vector<Tablon> finca, std::vector<bool> regado, int tiempoActual, int costoActual){
    //Caso base: todos los cultivos fueron regados
    int contf;
    int contv;
    for(int i = 0; i <= regado.size(); i++){
        if(regado[i] == false){
            contf ++;
        }
        else if(regado[i] == true){
            contV ++;
        }
    }
    if(contf == regado.size()){
        return 0;
    }
}

int fuerzaBruta (std::vector<Tablon> miFinca){
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
}


int main(){
    std::vector<Tablon> finca;
    std::vector<bool> regado;

    //Abrir el archivo txt
    std::ifstream archivo("finca.txt");

    //Verificar que el archivo se abrio correctamente
    if(!archivo.is_open()){
        std::cout<<"Error: No se pudo abrir el archivo"<<std::endl;
        return 1;
    }

    //Creación De tablón para guardar los datos de la finca
    Tablon tablonTemporal;

    //Leer los datos del txt
    while (archivo >> tablonTemporal.ts >> tablonTemporal.tr >> tablonTemporal.p >> tablonTemporal.rp){
        finca.push_back(tablonTemporal);
    }

    //Cerrar el archivo
    archivo.close();

    //Imprimir los datos de la finca
    for(const Tablon &tablon : finca){
        std::cout<<"ts: "<<tablon.ts<<" tr: "<<tablon.tr<<" p: "<<tablon.p<<" rp: "<<tablon.rp<<std::endl;
    }
    return 0;
}
