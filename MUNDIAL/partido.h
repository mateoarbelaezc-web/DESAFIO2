#ifndef PARTIDO_H
#define PARTIDO_H

#include "Equipo.h"
#include <string>

class EstadisticasJugadorPartido {
public:
    int goles;
    int tarjetasAmarillas;
    int tarjetasRojas;
    int faltas;
    int minutos;
    EstadisticasJugadorPartido() : goles(0), tarjetasAmarillas(0),
        tarjetasRojas(0), faltas(0), minutos(0) {}
};

class Convocado {
public:
    Jugador* jugador;
    EstadisticasJugadorPartido stats;
    Convocado() : jugador(nullptr) {}
};

class Partido {
private:
    std::string fecha;
    std::string hora;
    std::string sede;
    Equipo* equipo1;
    Equipo* equipo2;
    int golesEq1;
    int golesEq2;
    double posesionEq1;
    double posesionEq2;
    bool prorroga;
    Convocado* convocadosEq1;
    Convocado* convocadosEq2;

    double calcularGolesEsperados(Equipo* A, Equipo* B);
    void seleccionarConvocados(Equipo* equipo, Convocado* destino);
    void simularEventosJugador(Convocado& conv, int& golesRestantesEquipo, int minutosPartido);
    void calcularPosesion();

public:
    Partido(Equipo* eq1, Equipo* eq2, const std::string& fecha,
            const std::string& hora, const std::string& sede);
    ~Partido();
    Partido(const Partido&) = delete;
    Partido& operator=(const Partido&) = delete;

    void simular(bool esEliminatoria = false);
    void actualizarHistoricos();
    void imprimirResultado() const;
    void imprimirGoleadores() const;

    int getGolesEq1() const { return golesEq1; }
    int getGolesEq2() const { return golesEq2; }
    Equipo* getEquipo1() const { return equipo1; }
    Equipo* getEquipo2() const { return equipo2; }
    bool huboProrroga() const { return prorroga; }
};

#endif
