#include "Grupo.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>

std::string sumarDias(const std::string& fecha, int dias) {
    int dia, mes, anio;
    sscanf(fecha.c_str(), "%d/%d/%d", &dia, &mes, &anio);
    dia += dias;
    while (dia > 30) {
        dia -= 30;
        mes++;
        if (mes > 12) {
            mes = 1;
            anio++;
        }
    }
    char buffer[11];
    sprintf(buffer, "%02d/%02d/%04d", dia, mes, anio);
    return std::string(buffer);
}

Grupo::Grupo(char letra) : letra(letra) {
    for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i) equipos[i] = nullptr;
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i) partidos[i] = nullptr;
}

Grupo::~Grupo() {
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i)
        delete partidos[i];
}

void Grupo::setEquipo(int idx, Equipo* eq) {
    if (idx >= 0 && idx < EQUIPOS_POR_GRUPO) equipos[idx] = eq;
}

Equipo* Grupo::getEquipo(int idx) const {
    if (idx >= 0 && idx < EQUIPOS_POR_GRUPO) return equipos[idx];
    return nullptr;
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
    int puntos[4] = {0};
    int difGoles[4] = {0};
    int golesFavor[4] = {0};
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
        if (g1 > g2) puntos[idx1] += 3;
        else if (g2 > g1) puntos[idx2] += 3;
        else { puntos[idx1] += 1; puntos[idx2] += 1; }
        difGoles[idx1] += (g1 - g2);
        difGoles[idx2] += (g2 - g1);
        golesFavor[idx1] += g1;
        golesFavor[idx2] += g2;
    }
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
    Equipo *p1, *p2, *p3;
    int pts1, dif1, gf1, pts2, dif2, gf2, pts3, dif3, gf3;
    obtenerClasificados(p1, p2, p3, pts1, dif1, gf1, pts2, dif2, gf2, pts3, dif3, gf3);
    std::cout << "Grupo " << letra << ":\n";
    std::cout << "  1. " << p1->getPais() << "\n  2. " << p2->getPais() << "\n  3. " << p3->getPais() << "\n  4. ";
    for (int i = 0; i < 4; ++i)
        if (equipos[i] != p1 && equipos[i] != p2 && equipos[i] != p3)
            std::cout << equipos[i]->getPais() << "\n";
}

void Grupo::imprimirPartidos() const {
    for (int i = 0; i < PARTIDOS_POR_GRUPO; ++i) {
        if (partidos[i]) {
            partidos[i]->imprimirResultado();
            partidos[i]->imprimirGoleadores();
        }
    }
}
