#include "Finca.h"
#include <algorithm>


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

void Finca::permutaciones(vector<int> indicesDisponibles, vector<int> permActual, double& mejorCosto, vector<int>& mejorPermutacion, bool buscarMinimo){
    if(indicesDisponibles.size() == 0){
        double costoActual = calcularCostoDeProgramacion(permActual);
        
        if(mejorCosto == -1 || (buscarMinimo ? costoActual < mejorCosto : costoActual > mejorCosto)){
            mejorCosto = costoActual;
            mejorPermutacion = permActual;
        }
        return;
    }

    for(int i = 0; i < indicesDisponibles.size(); i++){
        permutaciones(eliminarElemento(indicesDisponibles, i), 
                     agregarElemento(permActual, indicesDisponibles[i]), 
                     mejorCosto, mejorPermutacion, buscarMinimo);
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
    double mejorCosto = -1;
    vector<int> mejorPermutacion;

    // Llamamos a la recursión. Ella se encarga de comparar y no guarda nada en listas grandes y agregé el true para vel lo de mejor o peor.
    permutaciones(indices, {}, mejorCosto, mejorPermutacion, true);
    return {mejorPermutacion, mejorCosto};
}

pair<vector<int>, double> Finca::roFB_peor(){
    vector<int> indices(numeroDeTablones());
    for (int i = 0; i < numeroDeTablones(); i++) indices[i] = i;
    double peorCosto = -1;
    vector<int> peorPermutacion;
    permutaciones(indices, {}, peorCosto, peorPermutacion, false);
    return {peorPermutacion, peorCosto};
}


pair<vector<int>, double> Finca::roV(){
    vector<int> permutacion = ordenarPorCriterioVoraz();
    double costo = calcularCostoDeProgramacion(permutacion);
    return {permutacion, costo};
}

vector<int> Finca::ordenarPorCriterioVoraz(){
    vector<pair<int,double>> criterios(numeroDeTablones());
    
    for(int i = 0; i < numeroDeTablones(); i++){
        criterios[i] = {i, getTablon(i).valorVoraz()};
    }
    
    sort(criterios.begin(), criterios.end(), [](const pair<int,double>& a, const pair<int,double>& b){
        return a.second > b.second;
    });
    
    vector<int> permutacion(numeroDeTablones());
    for(int i = 0; i < numeroDeTablones(); i++){
        permutacion[i] = criterios[i].first;
    }
    
    return permutacion;
}