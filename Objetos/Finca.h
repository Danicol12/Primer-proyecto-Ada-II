#ifndef FINCA_H
#define FINCA_H

#include <vector>
#include "Tablon.h" 
using namespace std;


class Finca {
private:
    vector<Tablon> tablones; // Aquí se guardan todos los tablones

public:
    Finca(vector<Tablon> tablones); 

    int calcularCostoDeProgramacion(const vector<int>& permutacion);
    void permutaciones(vector<int>, vector<int>, double& , vector<int>&,bool );
    vector<int> agregarElemento(vector<int>,int);
    vector<int> eliminarElemento(vector<int>,int);
    pair<vector<int>, double> roFB(); 



    pair<vector<int>, double> roV();
    vector<int> ordenarPorCriterioVoraz();
    

    pair<vector<int>, double> roPD();
    
    int numeroDeTablones();
    
    Tablon getTablon(int indice);

    //Agregado para ver los peores
    pair<vector<int>, double> roFB_peor();
};

#endif