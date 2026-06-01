#include "Finca.h"
#include <algorithm>
#include <utility>

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
    for(size_t i=0; i<permutacion.size();i++){
        costoAcum += tablones[permutacion[i]].calcularCosto(tiempo);
        tiempo = tiempo+tablones[permutacion[i]].getTiempoDeRegado();
    }
    return costoAcum;
}

// ===================== FUERZA BRUTA =====================

vector<int> Finca::agregarElemento(vector<int> arreglo, int elemento){
    arreglo.push_back(elemento);
    return arreglo;
}

vector<int> Finca::eliminarElemento(vector<int> arreglo, int indice){
    arreglo.erase(arreglo.begin()+ indice);
    return arreglo;
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
    for(size_t i = 0; i < indicesDisponibles.size(); i++){
        permutaciones(eliminarElemento(indicesDisponibles, i), 
                     agregarElemento(permActual, indicesDisponibles[i]), 
                     mejorCosto, mejorPermutacion, buscarMinimo);
    }
}

pair<vector<int>, double> Finca::roFB(){
    vector<int> indices(numeroDeTablones());
    for (int i = 0; i < numeroDeTablones(); i++) indices[i] = i;
    double mejorCosto = -1;
    vector<int> mejorPermutacion;
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

// ===================== VORAZ =====================

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

pair<vector<int>, double> Finca::roV(){
    vector<int> permutacion = ordenarPorCriterioVoraz();
    double costo = calcularCostoDeProgramacion(permutacion);
    return {permutacion, costo};
}

// ===================== PROGRAMACION DINAMICA =====================

int Finca::tiempoDesdeMascara(int mask) {
    int tiempo = 0;
    for (size_t i = 0; i < tablones.size(); i++) {
        if (mask & (1 << i)) {
            tiempo += tablones[i].getTiempoDeRegado();
        }
    }
    return tiempo;
}

double Finca::resolverPD(int mask, int n, vector<double>& dp, vector<int>& choice) {
    if (mask == (1 << n) - 1) return 0;
    if (dp[mask] != -1) return dp[mask];

    int tiempo_actual = tiempoDesdeMascara(mask);
    double minCosto = 1e18;
    int mejorIndice = -1;

    for (int i = 0; i < n; i++) {
        if (!(mask & (1 << i))) {
            double costo = tablones[i].calcularCosto(tiempo_actual)
                         + resolverPD(mask | (1 << i), n, dp, choice);
            if (costo < minCosto) {
                minCosto = costo;
                mejorIndice = i;
            }
        }
    }

    choice[mask] = mejorIndice;
    return dp[mask] = minCosto;
}

pair<vector<int>, double> Finca::roPD() {
    int n = numeroDeTablones();
    if (n == 0) return {{}, 0};

    vector<double> dp(1 << n, -1);
    vector<int> choice(1 << n, -1);

    double costoMinimo = resolverPD(0, n, dp, choice);

    vector<int> permutacion;
    int mask = 0;
    for (int paso = 0; paso < n; paso++) {
        int indice = choice[mask];
        permutacion.push_back(indice);
        mask |= (1 << indice);
    }

    return {permutacion, costoMinimo};
}