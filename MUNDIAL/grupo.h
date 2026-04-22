#ifndef GRUPO_H
#define GRUPO_H

#include "Equipo.h"
#include "Partido.h"

const int EQUIPOS_POR_GRUPO = 4;
const int PARTIDOS_POR_GRUPO = 6;

class Grupo {
private:
    char letra;
    Equipo* equipos[EQUIPOS_POR_GRUPO];
    Partido* partidos[PARTIDOS_POR_GRUPO];

public:
    Grupo(char letra);
    ~Grupo();

    void asignarEquipos(Equipo* e1, Equipo* e2, Equipo* e3, Equipo* e4);
    void setEquipo(int idx, Equipo* eq);
    Equipo* getEquipo(int idx) const;
    void calcularEstadisticas(
        int puntos[],
        int difGoles[],
        int golesFavor[],
        int golesContra[],
        int ganados[],
        int empatados[],
        int perdidos[],
        int pj[]
        ) const;
    void configurarPartidos(const std::string& f1,
                            const std::string& f2,
                            const std::string& f3);
    void simularEtapa(bool esEliminatoria = false);
    void actualizarHistoricos();

    // Nueva versión con 12 parámetros (3 equipos + 9 estadísticas)
    void obtenerClasificados(Equipo*& primero, Equipo*& segundo, Equipo*& tercero,
                             int& pts1, int& dif1, int& gf1,
                             int& pts2, int& dif2, int& gf2,
                             int& pts3, int& dif3, int& gf3) const;

    void imprimirTabla() const;
    void imprimirPartidos() const;
    char getLetra() const { return letra; }
};

#endif
