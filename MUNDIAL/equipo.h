#ifndef EQUIPO_H
#define EQUIPO_H

#include "Jugador.h"
#include <string>

const int JUGADORES_POR_EQUIPO = 26;

class Equipo {
private:
    int ranking;
    std::string pais;
    std::string directorTecnico;
    std::string federacion;
    std::string confederacion;
    int golesAFavor;
    int golesEnContra;
    int partidosGanados;
    int partidosEmpatados;
    int partidosPerdidos;
    int tarjetasAmarillas;
    int tarjetasRojas;
    int faltas;
    Jugador* jugadores;

public:
    Equipo();
    Equipo(int rank, const std::string& pais, const std::string& dt,
           const std::string& fed, const std::string& conf,
           int gf, int gc, int pg, int pe, int pp);
    Equipo(const Equipo& otro);
    ~Equipo();
    Equipo& operator=(const Equipo& otro);   // operador de asignación

    // Getters
    int getRanking() const;
    std::string getPais() const;
    std::string getConfederacion() const;
    double getPromedioGolesAFavor() const;
    double getPromedioGolesEnContra() const;
    int getGolesAFavor() const;
    int getGolesEnContra() const;
    int getPartidosGanados() const;
    int getPartidosEmpatados() const;
    int getPartidosPerdidos() const;
    int getTarjetasAmarillas() const;
    int getTarjetasRojas() const;
    int getFaltas() const;

    void actualizarEstadisticas(int gf, int gc, int amarillas, int rojas,
                                int faltasPartido, bool ganado, bool empatado, bool perdido);
    void repartirGolesHistoricos();

    Jugador* getJugadores() const { return jugadores; }
    void setJugador(int idx, const Jugador& j);
    Jugador& getJugador(int idx);

    bool operator<(const Equipo& otro) const;
    friend std::ostream& operator<<(std::ostream& os, const Equipo& eq);
};

#endif
