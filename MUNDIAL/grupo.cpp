#include "Grupo.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>

Grupo::Grupo(char letra) : letra(letra) {
    for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i) equipos[i] = nullptr;
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i) partidos[i] = nullptr;
}

Grupo::~Grupo() {
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i)
        delete partidos[i];
}
// Constructor de copia de Grupo
// RAZÓN: copia profunda de los partidos
//        los punteros a equipos[] se comparten porque
//        los equipos son propiedad del Torneo
Grupo::Grupo(const Grupo& otro) : letra(otro.letra) {
    for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i)
        equipos[i] = otro.equipos[i];
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i) {
        if (otro.partidos[i])
            partidos[i] = new Partido(*otro.partidos[i]);
        else
            partidos[i] = nullptr;
    }
}

// Operador de asignación de Grupo
Grupo& Grupo::operator=(const Grupo& otro) {
    if (this == &otro) return *this;
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i)
        delete partidos[i];
    letra = otro.letra;
    for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i)
        equipos[i] = otro.equipos[i];
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i) {
        if (otro.partidos[i])
            partidos[i] = new Partido(*otro.partidos[i]);
        else
            partidos[i] = nullptr;
    }
    return *this;
}

void Grupo::setEquipo(int idx, Equipo* eq) {
    if (idx >= 0 && idx < EQUIPOS_POR_GRUPO) equipos[idx] = eq;
}

Equipo* Grupo::getEquipo(int idx) const {
    if (idx >= 0 && idx < EQUIPOS_POR_GRUPO) return equipos[idx];
    return nullptr;
}

void Grupo::calcularEstadisticas(
    int puntos[],
    int difGoles[],
    int golesFavor[],
    int golesContra[],
    int ganados[],
    int empatados[],
    int perdidos[],
    int pj[]
    ) const {

    // Inicializar
    for (int i = 0; i < 4; ++i) {
        puntos[i] = 0;
        difGoles[i] = 0;
        golesFavor[i] = 0;
        golesContra[i] = 0;
        ganados[i] = 0;
        empatados[i] = 0;
        perdidos[i] = 0;
        pj[i] = 0;
    }

    // Recorrer partidos
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i) {
        if (!partidos[i]) continue;

        int g1 = partidos[i]->getGolesEq1();
        int g2 = partidos[i]->getGolesEq2();

        Equipo* eq1 = partidos[i]->getEquipo1();
        Equipo* eq2 = partidos[i]->getEquipo2();

        int idx1 = -1, idx2 = -1;

        for (int j = 0; j < 4; ++j) {
            if (equipos[j] == eq1) idx1 = j;
            if (equipos[j] == eq2) idx2 = j;
        }

        if (idx1 == -1 || idx2 == -1) continue;

        pj[idx1]++;
        pj[idx2]++;

        golesFavor[idx1] += g1;
        golesFavor[idx2] += g2;

        golesContra[idx1] += g2;
        golesContra[idx2] += g1;

        difGoles[idx1] += (g1 - g2);
        difGoles[idx2] += (g2 - g1);

        if (g1 > g2) {
            puntos[idx1] += 3;
            ganados[idx1]++;
            perdidos[idx2]++;
        }
        else if (g2 > g1) {
            puntos[idx2] += 3;
            ganados[idx2]++;
            perdidos[idx1]++;
        }
        else {
            puntos[idx1]++;
            puntos[idx2]++;
            empatados[idx1]++;
            empatados[idx2]++;
        }
    }
}

void Grupo::asignarEquipos(Equipo* e1, Equipo* e2, Equipo* e3, Equipo* e4) {
    equipos[0] = e1; equipos[1] = e2; equipos[2] = e3; equipos[3] = e4;
}

// CAMBIO: configurarPartidos ahora recibe las 3 fechas como parámetros
// RAZÓN: La asignación de fechas debe hacerse desde Torneo porque necesita
//        conocer todos los partidos de todos los grupos simultáneamente
//        para respetar el límite de 4 partidos por día
void Grupo::configurarPartidos(const std::string& f1,
                               const std::string& f2,
                               const std::string& f3) {
    partidos[0] = new Partido(equipos[0], equipos[1], f1, "00:00", "nombreSede");
    partidos[1] = new Partido(equipos[2], equipos[3], f1, "00:00", "nombreSede");
    partidos[2] = new Partido(equipos[0], equipos[2], f2, "00:00", "nombreSede");
    partidos[3] = new Partido(equipos[1], equipos[3], f2, "00:00", "nombreSede");
    partidos[4] = new Partido(equipos[0], equipos[3], f3, "00:00", "nombreSede");
    partidos[5] = new Partido(equipos[1], equipos[2], f3, "00:00", "nombreSede");
}

void Grupo::simularEtapa(bool esEliminatoria) {
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i)
        if (partidos[i]) partidos[i]->simular(esEliminatoria);
}

void Grupo::actualizarHistoricos() {
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i)
        if (partidos[i]) partidos[i]->actualizarHistoricos();
}

void Grupo::obtenerClasificados(Equipo*& primero, Equipo*& segundo, Equipo*& tercero,
                                int& pts1, int& dif1, int& gf1,
                                int& pts2, int& dif2, int& gf2,
                                int& pts3, int& dif3, int& gf3) const {
    int puntos[4], difGoles[4], golesFavor[4], golesContra[4];
    int ganados[4], empatados[4], perdidos[4], pj[4];

    calcularEstadisticas(puntos, difGoles, golesFavor, golesContra,
                         ganados, empatados, perdidos, pj);

    int indices[4] = {0,1,2,3};
    for (int i = 0; i < 3; ++i) {
        for (int j = i+1; j < 4; ++j) {
            bool intercambiar = false;
            if (puntos[indices[j]] > puntos[indices[i]]) intercambiar = true;
            else if (puntos[indices[j]] == puntos[indices[i]]) {
                if (difGoles[indices[j]] > difGoles[indices[i]]) intercambiar = true;
                else if (difGoles[indices[j]] == difGoles[indices[i]] &&
                         golesFavor[indices[j]] > golesFavor[indices[i]])
                    intercambiar = true;
            }
            if (intercambiar) {
                int temp = indices[i];
                indices[i] = indices[j];
                indices[j] = temp;
            }
        }
    }
    primero = equipos[indices[0]];
    segundo = equipos[indices[1]];
    tercero = equipos[indices[2]];
    pts1 = puntos[indices[0]]; dif1 = difGoles[indices[0]]; gf1 = golesFavor[indices[0]];
    pts2 = puntos[indices[1]]; dif2 = difGoles[indices[1]]; gf2 = golesFavor[indices[1]];
    pts3 = puntos[indices[2]]; dif3 = difGoles[indices[2]]; gf3 = golesFavor[indices[2]];
}

void Grupo::imprimirTabla() const {
    int puntos[4], difGoles[4], golesFavor[4], golesContra[4];
    int ganados[4], empatados[4], perdidos[4], pj[4];

    calcularEstadisticas(puntos, difGoles, golesFavor, golesContra,
                         ganados, empatados, perdidos, pj);

    int indices[4] = {0,1,2,3};

    for (int i = 0; i < 3; ++i)
        for (int j = i+1; j < 4; ++j) {
            bool swap = false;

            if (puntos[indices[j]] > puntos[indices[i]]) swap = true;
            else if (puntos[indices[j]] == puntos[indices[i]]) {
                if (difGoles[indices[j]] > difGoles[indices[i]]) swap = true;
                else if (difGoles[indices[j]] == difGoles[indices[i]] &&
                         golesFavor[indices[j]] > golesFavor[indices[i]])
                    swap = true;
            }

            if (swap) {
                int t = indices[i];
                indices[i] = indices[j];
                indices[j] = t;
            }
        }

    std::cout << "\n+--Grupo " << letra
              << "-------------------------------------------+\n";
    std::cout << "| Equipo               | PJ| PG| PE| PP|  DG| PTS|\n";
    std::cout << "+----------------------+---+---+---+---+----+----+\n";

    for (int i = 0; i < 4; ++i) {
        int k = indices[i];

        std::string nombre = equipos[k]->getPais();
        while ((int)nombre.size() < 20) nombre += ' ';
        if ((int)nombre.size() > 20) nombre = nombre.substr(0, 20);

        char dg[8];
        sprintf(dg, "%+3d", difGoles[k]);

        printf("| %s | %2d| %2d| %2d| %2d| %s| %3d|\n",
               nombre.c_str(),
               pj[k],
               ganados[k],
               empatados[k],
               perdidos[k],
               dg,
               puntos[k]);
    }

    std::cout << "+----------------------+---+---+---+---+----+----+\n";
}

void Grupo::imprimirPartidos() const {
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i) {
        if (partidos[i]) {
            partidos[i]->imprimirResultado();
            partidos[i]->imprimirGoleadores();
        }
    }
}
