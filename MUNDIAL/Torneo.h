#ifndef TORNEO_H
#define TORNEO_H

#include "Equipo.h"
#include "Grupo.h"

const int NUM_EQUIPOS = 48;
const int NUM_GRUPOS = 12;
const int BOMBOS = 4;
const int EQUIPOS_POR_BOMBO = 12;

class Clasificado {
public:
    Equipo* equipo;
    int puntos;
    int diferenciaGoles;
    int golesFavor;
    char grupo;
    int posicion;
    Clasificado() : equipo(nullptr), puntos(0), diferenciaGoles(0),
        golesFavor(0), grupo(' '), posicion(0) {}
};

class Torneo {
private:
    Equipo equipos[NUM_EQUIPOS];
    Grupo* grupos[NUM_GRUPOS];
    int numEquipos;
    Equipo* bombos[BOMBOS][EQUIPOS_POR_BOMBO];

    // Métodos privados
    void ordenarEquiposPorRanking(Equipo* lista[], int n);
    void distribuirBombos();
    // ANTES: bool respetaConfederaciones(Equipo* equipo, Grupo* grupo, int& uefaCount);
    // AHORA: bool respetaConfederaciones(Equipo* equipo, Grupo* grupo);
    bool respetaConfederaciones(Equipo* equipo, Grupo* grupo);
    void recolectarClasificados(Clasificado primeros[], int& pCount,
                                Clasificado segundos[], int& sCount,
                                Clasificado terceros[], int& tCount);
    void ordenarTerceros(Clasificado terceros[], int& tCount);
    bool mismoGrupo(Equipo* a, Equipo* b);
    void contarConfederaciones(Equipo** lista, int n);
    void generarEnfrentamientosR16(Clasificado primeros[], int pCount,
                                   Clasificado segundos[], int sCount,
                                   Clasificado terceros[], int tCount,
                                   Equipo* enfrentamientos[32], int& eCount);
    void simularRonda(Equipo** participantes, int numParticipantes,
                      const std::string& fecha, bool esEliminatoria,
                      Equipo** ganadores, int& numGanadores);
    void imprimirGoleadoresGlobal();
    void calcularEstadisticasFinales(Equipo* campeon, Equipo* subcampeon,
                                     Equipo* tercero, Equipo* cuarto);
    Equipo* participantesR16[32];
    Equipo* participantesR8[16];
    Equipo* participantesR4[8];

    int countR16;
    int countR8;
    int countR4;

    // Resultados finales
    Equipo* campeon;
    Equipo* subcampeon;
    Equipo* terceroLugar;
    Equipo* cuartoLugar;

    // Flags de estado para el menú
    bool equiposCargados;
    bool bombosConfigurados;
    bool gruposSorteados;
    bool faseGruposSimulada;
    bool eliminatoriasSimuladas;

public:
    Torneo();
    ~Torneo();
    Torneo(const Torneo& otro);
    Torneo& operator=(const Torneo& otro);

    void cargarEquipos(const std::string& archivo);
    void conformarBombos();
    void sortearGrupos();
    void simularFaseGrupos(const std::string& fechaInicio);
    void simularEliminatorias(const std::string& fechaEliminatorias);
    void imprimirGrupos();

    static long long totalIteraciones;
    static void incIteraciones(long long n = 1);
    static size_t calcularMemoriaActual();
    void mostrarMetricas(const std::string& etapa, size_t bytesLocales = 0,
                         const std::string& componentesExternos = "");
    bool isEquiposCargados() const { return equiposCargados; }
    bool isBombosConfigurados() const { return bombosConfigurados; }
    bool isGruposSorteados() const { return gruposSorteados; }
    bool isFaseGruposSimulada() const { return faseGruposSimulada; }
    bool isEliminatoriasSimuladas() const { return eliminatoriasSimuladas; }

    void guardarEstadisticasJugadores(const std::string& archivo);
    void cargarEstadisticasJugadores(const std::string& archivo);

    // Estadísticas finales
    void generarEstadisticasFinales();
};

#endif
