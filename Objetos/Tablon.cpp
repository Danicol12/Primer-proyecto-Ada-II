#include "Tablon.h"


Tablon::Tablon(int tiempo_de_Supervivencia, int tiempo_de_Regado, int prioridad, int tiempo_de_Riego_Perfecto) {
    this->tiempo_de_Supervivencia = tiempo_de_Supervivencia;
    this->tiempo_de_Regado = tiempo_de_Regado;
    this->prioridad = prioridad;
    this->tiempo_de_Riego_Perfecto = tiempo_de_Riego_Perfecto;
}

void Tablon::setTiempoDeSupervivcencia(int tiempo_de_Supervivencia) {
    this->tiempo_de_Supervivencia = tiempo_de_Supervivencia;
}

void Tablon::setTiempoDeRegado(int tiempo_de_Regado) {
    this->tiempo_de_Regado = tiempo_de_Regado;
}

void Tablon::setPrioridad(int prioridad) {
    this->prioridad = prioridad;
}

void Tablon::setTiempoDeRiegoPerfecto(int tiempo_de_Riego_Perfecto) {
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

double Tablon::valorVoraz(){

    return (double)getPrioridad() / ((double)getTiempoDeRegado() * (double)getTiempoDeSupervivencia());
}