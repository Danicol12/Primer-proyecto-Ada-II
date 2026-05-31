#ifndef TABLON_H
#define TABLON_H


class Tablon {
private:
    int tiempo_de_Supervivencia;
    int tiempo_de_Regado;
    int prioridad;
    int tiempo_de_Riego_Perfecto;

public:
    Tablon(int tiempo_de_Supervivencia, int tiempo_de_Regado, int prioridad, int tiempo_de_Riego_Perfecto);

    void setTiempoDeSupervivcencia(int tiempo_de_Supervivencia);
    void setTiempoDeRegado(int tiempo_de_Regado);
    void setPrioridad(int prioridad);
    void setTiempoDeRiegoPerfecto(int tiempo_de_Riego_Perfecto);

    int getTiempoDeSupervivencia() const;
    int getTiempoDeRegado() const;
    int getPrioridad() const;
    int getTiempoDeRiegoPerfecto() const;
    int calcularCosto(int tiempo) const;
    double valorVoraz() const;
};

#endif
