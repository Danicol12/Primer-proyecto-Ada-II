#ifndef TABLON_H
#define TABLON_H

#include <string>
#include <iostream>

using namespace std;

class Tablon {
private:
    int tiempo_de_Supervivencia;
    int tiempo_de_Regado;
    int prioridad;
    int tiempo_de_Riego_Perfecto;
public:
    Tablon(int tiempo_de_Supervivencia, int tiempo_de_Regado, int prioridad, int tiempo_de_Riego_Perfecto);
    int getTiempoDeSupervivencia();
    int getTiempoDeRegado();
    int getPrioridad();
    int getTiempoDeRiegoPerfecto();
    int calcularCosto(int);
    double valorVoraz();
};

#endif 