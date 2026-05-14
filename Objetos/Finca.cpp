#include "Finca.h"


Finca::Finca(vector<Tablon> tablones) {
    this->tablones = tablones;
}

int Finca::numeroDeTablones() {
    return this->tablones.size();
}

Tablon Finca::getTablon(int indice) {
    return this->tablones[indice];
}

int Finca::calcularCostoDeProgramacion(const vector<int>& permutacion){
    int costoAcum=0;
    int tiempo=0;
    for(int i=0; i<permutacion.size();i++){
        costoAcum += tablones[permutacion[i]].calcularCosto(tiempo);
        tiempo = tiempo+tablones[permutacion[i]].getTiempoDeRegado();

    }
    return costoAcum;
}

void Finca::permutaciones(vector<int> indicesDisponibles, vector<int> permActual, double& mejorCosto, vector<int>& mejorPermutacion){
    // Caso base: se armó una permutación completa
    if(indicesDisponibles.size() == 0){
        double costoActual = calcularCostoDeProgramacion(permActual);
        
        // Si es el primero que calculamos o es mejor que el anterior, lo guardamos
        if(mejorCosto == -1 || costoActual < mejorCosto){
            mejorCosto = costoActual;
            mejorPermutacion = permActual;
        }
        return;
    }

    for(int i = 0; i < indicesDisponibles.size(); i++){
        // Seguimos con la recursión pero pasando las variables de "el mejor"
        permutaciones(eliminarElemento(indicesDisponibles, i), 
                     agregarElemento(permActual, indicesDisponibles[i]), 
                     mejorCosto, mejorPermutacion);
    }
}

vector<int> Finca::agregarElemento(vector<int> arreglo, int elemento){
    arreglo.push_back(elemento);
    return arreglo;

}
vector<int> Finca::eliminarElemento(vector<int> arreglo, int indice){
    arreglo.erase(arreglo.begin()+ indice);
    return arreglo;

}
pair<vector<int>, double> Finca::roFB(){
    vector<int> indices(numeroDeTablones());
    for (int i = 0; i < numeroDeTablones(); i++) indices[i] = i;

    double mejorCosto = -1; // -1 indica que no se ha calculado nada
    vector<int> mejorPermutacion;

    // Llamamos a la recursión. Ella se encarga de comparar y no guarda nada en listas grandes.
    permutaciones(indices, {}, mejorCosto, mejorPermutacion);

    return {mejorPermutacion, mejorCosto};
}

pair<vector<int>, double> Finca::roV(){

    
}