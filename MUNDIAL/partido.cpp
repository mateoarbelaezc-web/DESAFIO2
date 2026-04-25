#include "Partido.h"
#include <cstdlib>
#include <cmath>
#include <iostream>
#include "Metricas.h"

const double MU = 1.35;
const double ALPHA = 0.6;
const double BETA = 0.4;
const double PROB_PRIMERA_AMARILLA = 0.06;
const double PROB_SEGUNDA_AMARILLA = 0.0115;
const double PROB_PRIMERA_FALTA = 0.13;
const double PROB_SEGUNDA_FALTA = 0.0275;
const double PROB_TERCERA_FALTA = 0.007;
const double PROB_GOL_POR_JUGADOR = 0.04;

bool ocurreConProbabilidad(double prob) {
    double r = static_cast<double>(rand()) / RAND_MAX;
    return r < prob;
}

double Partido::calcularGolesEsperados(Equipo* A, Equipo* B) {
    double gfA = A->getPromedioGolesAFavor();
    double gcB = B->getPromedioGolesEnContra();
    if (gfA < 0.01) gfA = 0.01;
    if (gcB < 0.01) gcB = 0.01;
    return MU * pow(gfA / MU, ALPHA) * pow(gcB / MU, BETA);
}

void Partido::seleccionarConvocados(Equipo* equipo, Convocado* destino) {
    int indices[26];
    for (int i = 0; i < 26; ++i) indices[i] = i;
    for (int i = 0; i < 11; ++i) {
        int j = i + rand() % (26 - i);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    for (int i = 0; i < 11; ++i) {
        incIter(1);
        destino[i].jugador = &(equipo->getJugador(indices[i]));
        destino[i].stats.goles = 0;
        destino[i].stats.tarjetasAmarillas = 0;
        destino[i].stats.tarjetasRojas = 0;
        destino[i].stats.faltas = 0;
        destino[i].stats.minutos = 0;
    }
}

void Partido::simularEventosJugador(Convocado& conv, int& golesRestantesEquipo, int minutosPartido) {
    conv.stats.minutos = minutosPartido;
    // Amarillas
    int amarillas = 0;
    if (ocurreConProbabilidad(PROB_PRIMERA_AMARILLA)) {
        amarillas = 1;
        if (ocurreConProbabilidad(PROB_SEGUNDA_AMARILLA))
            amarillas = 2;
    }
    if (amarillas == 2) {
        conv.stats.tarjetasAmarillas = 1;
        conv.stats.tarjetasRojas = 1;
    } else {
        conv.stats.tarjetasAmarillas = amarillas;
        conv.stats.tarjetasRojas = 0;
    }
    // Faltas
    int faltas = 0;
    if (ocurreConProbabilidad(PROB_PRIMERA_FALTA)) {
        faltas = 1;
        if (ocurreConProbabilidad(PROB_SEGUNDA_FALTA)) {
            faltas = 2;
            if (ocurreConProbabilidad(PROB_TERCERA_FALTA))
                faltas = 3;
        }
    }
    conv.stats.faltas = faltas;

    if (golesRestantesEquipo > 0 && ocurreConProbabilidad(PROB_GOL_POR_JUGADOR)) {
        conv.stats.goles++;
        golesRestantesEquipo--;
    }
}

void Partido::calcularPosesion() {
    int r1 = equipo1->getRanking();
    int r2 = equipo2->getRanking();
    double total = r1 + r2;
    if (total == 0) total = 1;
    posesionEq1 = static_cast<double>(r2) / total;
    posesionEq1 = 0.3 + 0.4 * posesionEq1;
    posesionEq2 = 1.0 - posesionEq1;
}

Partido::Partido(Equipo* eq1, Equipo* eq2, const std::string& fecha,
                 const std::string& hora, const std::string& sede)
    : fecha(fecha), hora(hora), sede(sede), equipo1(eq1), equipo2(eq2),
    golesEq1(0), golesEq2(0), posesionEq1(0.5), posesionEq2(0.5), prorroga(false) {
    convocadosEq1 = new Convocado[11];
    convocadosEq2 = new Convocado[11];
}

Partido::~Partido() {
    delete[] convocadosEq1;
    delete[] convocadosEq2;
}

Partido::Partido(const Partido& otro)
    : fecha(otro.fecha), hora(otro.hora), sede(otro.sede),
    equipo1(otro.equipo1), equipo2(otro.equipo2),
    golesEq1(otro.golesEq1), golesEq2(otro.golesEq2),
    posesionEq1(otro.posesionEq1), posesionEq2(otro.posesionEq2),
    prorroga(otro.prorroga) {
    // Copia profunda de los convocados
    convocadosEq1 = new Convocado[11];
    convocadosEq2 = new Convocado[11];
    for (int i = 0; i < 11; ++i) {
        convocadosEq1[i] = otro.convocadosEq1[i];
        convocadosEq2[i] = otro.convocadosEq2[i];
    }
}

// Operador de asignación de Partido
Partido& Partido::operator=(const Partido& otro) {
    if (this == &otro) return *this;
    delete[] convocadosEq1;
    delete[] convocadosEq2;
    fecha = otro.fecha; hora = otro.hora; sede = otro.sede;
    equipo1 = otro.equipo1; equipo2 = otro.equipo2;
    golesEq1 = otro.golesEq1; golesEq2 = otro.golesEq2;
    posesionEq1 = otro.posesionEq1; posesionEq2 = otro.posesionEq2;
    prorroga = otro.prorroga;
    convocadosEq1 = new Convocado[11];
    convocadosEq2 = new Convocado[11];
    for (int i = 0; i < 11; ++i) {
        convocadosEq1[i] = otro.convocadosEq1[i];
        convocadosEq2[i] = otro.convocadosEq2[i];
    }
    return *this;
}
void Partido::simular(bool esEliminatoria) {
    double lambda1 = calcularGolesEsperados(equipo1, equipo2);
    double lambda2 = calcularGolesEsperados(equipo2, equipo1);
    int golesEsperados1 = static_cast<int>(lambda1 + 0.5) + (rand() % 3) - 1;
    int golesEsperados2 = static_cast<int>(lambda2 + 0.5) + (rand() % 3) - 1;
    if (golesEsperados1 < 0) golesEsperados1 = 0;
    if (golesEsperados2 < 0) golesEsperados2 = 0;

    seleccionarConvocados(equipo1, convocadosEq1);
    seleccionarConvocados(equipo2, convocadosEq2);

    int golesRestantes1 = golesEsperados1;
    int golesRestantes2 = golesEsperados2;
    int minutosPartido = 90;

    for (int i = 0; i < 11; ++i) {
        incIter(1);
        simularEventosJugador(convocadosEq1[i], golesRestantes1, minutosPartido);
        simularEventosJugador(convocadosEq2[i], golesRestantes2, minutosPartido);
    }

    // CAMBIO: si después de recorrer los 11 jugadores aún quedan goles,
    // se reparten en una segunda pasada hasta que todos sean anotados
    // RAZÓN: garantiza que los golesEsperados siempre se cumplan completamente
    while (golesRestantes1 > 0) {
        for (int i = 0; i < 11 && golesRestantes1 > 0; ++i) {
            incIter(1);
            if (ocurreConProbabilidad(PROB_GOL_POR_JUGADOR)) {
                convocadosEq1[i].stats.goles++;
                golesRestantes1--;
            }
        }
    }
    while (golesRestantes2 > 0) {
        for (int i = 0; i < 11 && golesRestantes2 > 0; ++i) {
            incIter(1);
            if (ocurreConProbabilidad(PROB_GOL_POR_JUGADOR)) {
                convocadosEq2[i].stats.goles++;
                golesRestantes2--;
            }
        }
    }

    golesEq1 = 0;
    for (int i = 0; i < 11; ++i) golesEq1 += convocadosEq1[i].stats.goles;
    golesEq2 = 0;
    for (int i = 0; i < 11; ++i) golesEq2 += convocadosEq2[i].stats.goles;

    prorroga = false;
    if (esEliminatoria && golesEq1 == golesEq2) {
        prorroga = true;
        minutosPartido = 120;
        for (int i = 0; i < 11; ++i) {
            convocadosEq1[i].stats.minutos = minutosPartido;
            convocadosEq2[i].stats.minutos = minutosPartido;
        }
        // Sesgar aleatoriedad por ranking: menor ranking = mejor equipo = mayor probabilidad
        // Si r1=1 y r2=10, probEq1 = 10/11 ≈ 91% de ganar
        int r1 = equipo1->getRanking();
        int r2 = equipo2->getRanking();
        double probEq1 = static_cast<double>(r2) / (r1 + r2);
        double aleatorio = static_cast<double>(rand()) / RAND_MAX;
        if (aleatorio < probEq1) {
            golesEq1++;
            int idx = rand() % 11;
            convocadosEq1[idx].stats.goles++;
        } else {
            golesEq2++;
            int idx = rand() % 11;
            convocadosEq2[idx].stats.goles++;
        }
    }
    calcularPosesion();
}

void Partido::actualizarHistoricos() {
    bool ganado1 = (golesEq1 > golesEq2);
    bool ganado2 = (golesEq2 > golesEq1);
    bool empatado = (golesEq1 == golesEq2);
    int amarillasEq1 = 0, rojasEq1 = 0, faltasEq1 = 0;
    int amarillasEq2 = 0, rojasEq2 = 0, faltasEq2 = 0;
    for (int i = 0; i < 11; ++i) {
        amarillasEq1 += convocadosEq1[i].stats.tarjetasAmarillas;
        rojasEq1 += convocadosEq1[i].stats.tarjetasRojas;
        faltasEq1 += convocadosEq1[i].stats.faltas;
        amarillasEq2 += convocadosEq2[i].stats.tarjetasAmarillas;
        rojasEq2 += convocadosEq2[i].stats.tarjetasRojas;
        faltasEq2 += convocadosEq2[i].stats.faltas;
    }
    equipo1->actualizarEstadisticas(golesEq1, golesEq2, amarillasEq1, rojasEq1, faltasEq1, ganado1, empatado, !ganado1 && !empatado);
    equipo2->actualizarEstadisticas(golesEq2, golesEq1, amarillasEq2, rojasEq2, faltasEq2, ganado2, empatado, !ganado2 && !empatado);
    for (int i = 0; i < 11; ++i) {
        incIter(1);
        convocadosEq1[i].jugador->acumularEstadisticasPartido(convocadosEq1[i].stats.goles,
                                                              convocadosEq1[i].stats.tarjetasAmarillas,
                                                              convocadosEq1[i].stats.tarjetasRojas,
                                                              convocadosEq1[i].stats.faltas,
                                                              convocadosEq1[i].stats.minutos);
        convocadosEq2[i].jugador->acumularEstadisticasPartido(convocadosEq2[i].stats.goles,
                                                              convocadosEq2[i].stats.tarjetasAmarillas,
                                                              convocadosEq2[i].stats.tarjetasRojas,
                                                              convocadosEq2[i].stats.faltas,
                                                              convocadosEq2[i].stats.minutos);
    }
}

void Partido::imprimirResultado() const {
    std::cout << equipo1->getPais() << " " << golesEq1 << " - " << golesEq2 << " " << equipo2->getPais()
    << " (Fecha: " << fecha << ", " << sede << ")\n";
    if (prorroga) std::cout << "   *** Partido definido en prórroga ***\n";
    std::cout << "   Posesión: " << equipo1->getPais() << " " << (posesionEq1*100) << "% - "
              << (posesionEq2*100) << "% " << equipo2->getPais() << "\n";
}

void Partido::imprimirGoleadores() const {
    std::cout << "   Goleadores de " << equipo1->getPais() << ": ";
    bool primero = true;
    for (int i = 0; i < 11; ++i) {
        if (convocadosEq1[i].stats.goles > 0) {
            if (!primero) std::cout << ", ";
            std::cout << convocadosEq1[i].jugador->getNombre() << " "
                      << convocadosEq1[i].jugador->getApellido() << " (#"
                      << convocadosEq1[i].jugador->getNumero() << ") - "
                      << convocadosEq1[i].stats.goles << " gol(es)";
            primero = false;
        }
    }
    if (primero) std::cout << "ninguno";
    std::cout << "\n";
    std::cout << "   Goleadores de " << equipo2->getPais() << ": ";
    primero = true;
    for (int i = 0; i < 11; ++i) {
        if (convocadosEq2[i].stats.goles > 0) {
            if (!primero) std::cout << ", ";
            std::cout << convocadosEq2[i].jugador->getNombre() << " "
                      << convocadosEq2[i].jugador->getApellido() << " (#"
                      << convocadosEq2[i].jugador->getNumero() << ") - "
                      << convocadosEq2[i].stats.goles << " gol(es)";
            primero = false;
        }
    }
    if (primero) std::cout << "ninguno";
    std::cout << "\n";
}
