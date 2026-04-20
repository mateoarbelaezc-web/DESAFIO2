#ifndef TORNEO_H
#define TORNEO_H

#include "Equipo.h"
#include "Grupo.h"

const int NUM_EQUIPOS = 48;
const int NUM_GRUPOS = 12;
const int BOMBOS = 4;
const int EQUIPOS_POR_BOMBO = 12;

// Estructura auxiliar para guardar información de clasificados
struct Clasificado {
    Equipo* equipo;
    int puntos;
    int diferenciaGoles;
    int golesFavor;
    char grupo;
    int posicion; // 1=primero, 2=segundo, 3=tercero
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
    bool respetaConfederaciones(Equipo* equipo, Grupo* grupo, int& uefaCount);
    void recolectarClasificados(Clasificado primeros[], int& pCount,
                                Clasificado segundos[], int& sCount,
                                Clasificado terceros[], int& tCount);
    void ordenarTerceros(Clasificado terceros[], int& tCount);
    bool mismoGrupo(Equipo* a, Equipo* b);
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

public:
    Torneo();
    ~Torneo();

    void cargarEquipos(const std::string& archivo);
    void conformarBombos();
    void sortearGrupos();
    void simularFaseGrupos(const std::string& fechaInicio);
    void simularEliminatorias(const std::string& fechaEliminatorias);
    void generarEstadisticasFinales();
    void imprimirGrupos();

    // Métricas
    static long long totalIteraciones;
    static void incIteraciones(long long n = 1);
    static size_t calcularMemoriaActual();
    void mostrarMetricas(const std::string& etapa);
};

#endif
