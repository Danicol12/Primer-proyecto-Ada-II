#include "Tablon.h"
#include <iostream>


Tablon::Tablon(int tiempo_de_Supervivencia, int tiempo_de_Regado, int prioridad, int tiempo_de_Riego_Perfecto) {
    this->tiempo_de_Supervivencia = tiempo_de_Supervivencia;
    this->tiempo_de_Regado = tiempo_de_Regado;
    this->prioridad = prioridad;
    this->tiempo_de_Riego_Perfecto = tiempo_de_Riego_Perfecto;
}

int Tablon::getTiempoDeSupervivencia() {
    return this->tiempo_de_Supervivencia;
}

int Tablon::getTiempoDeRegado() {
    return this->tiempo_de_Regado;
}

int Tablon::getPrioridad() {
    return this->prioridad;
}

int Tablon::getTiempoDeRiegoPerfecto() {
    return this->tiempo_de_Riego_Perfecto;
}
int Tablon::calcularCosto(int tiempo){
    int costo=0;
    if(getTiempoDeRiegoPerfecto()==tiempo){
        costo = getTiempoDeSupervivencia()-(tiempo+getTiempoDeRegado());
    }
    else if((getTiempoDeSupervivencia()-getTiempoDeRegado())>=tiempo){
        costo = 2*(getTiempoDeSupervivencia()-(tiempo+getTiempoDeRegado()));
    }
    else{
        costo = 2*getPrioridad()*((tiempo+getTiempoDeRegado())-getTiempoDeSupervivencia());
    }
    return costo;

}