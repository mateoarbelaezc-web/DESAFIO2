#include "Equipo.h"

Equipo::Equipo() : ranking(0), pais(""), directorTecnico(""), federacion(""), confederacion(""),
    golesAFavor(0), golesEnContra(0), partidosGanados(0), partidosEmpatados(0), partidosPerdidos(0),
    tarjetasAmarillas(0), tarjetasRojas(0), faltas(0) {
    jugadores = new Jugador[JUGADORES_POR_EQUIPO];
}

Equipo::Equipo(int rank, const std::string& pais, const std::string& dt,
               const std::string& fed, const std::string& conf,
               int gf, int gc, int pg, int pe, int pp)
    : ranking(rank), pais(pais), directorTecnico(dt), federacion(fed), confederacion(conf),
    golesAFavor(gf), golesEnContra(gc), partidosGanados(pg), partidosEmpatados(pe), partidosPerdidos(pp),
    tarjetasAmarillas(0), tarjetasRojas(0), faltas(0) {
    jugadores = new Jugador[JUGADORES_POR_EQUIPO];
}

Equipo::Equipo(const Equipo& otro)
    : ranking(otro.ranking), pais(otro.pais), directorTecnico(otro.directorTecnico),
    federacion(otro.federacion), confederacion(otro.confederacion),
    golesAFavor(otro.golesAFavor), golesEnContra(otro.golesEnContra),
    partidosGanados(otro.partidosGanados), partidosEmpatados(otro.partidosEmpatados),
    partidosPerdidos(otro.partidosPerdidos),
    tarjetasAmarillas(otro.tarjetasAmarillas), tarjetasRojas(otro.tarjetasRojas),
    faltas(otro.faltas) {
    jugadores = new Jugador[JUGADORES_POR_EQUIPO];
    for (int i = 0; i < JUGADORES_POR_EQUIPO; ++i)
        jugadores[i] = otro.jugadores[i];
}

Equipo::~Equipo() {
    delete[] jugadores;
}

Equipo& Equipo::operator=(const Equipo& otro) {
    if (this == &otro) return *this;
    delete[] jugadores;
    ranking = otro.ranking;
    pais = otro.pais;
    directorTecnico = otro.directorTecnico;
    federacion = otro.federacion;
    confederacion = otro.confederacion;
    golesAFavor = otro.golesAFavor;
    golesEnContra = otro.golesEnContra;
    partidosGanados = otro.partidosGanados;
    partidosEmpatados = otro.partidosEmpatados;
    partidosPerdidos = otro.partidosPerdidos;
    tarjetasAmarillas = otro.tarjetasAmarillas;
    tarjetasRojas = otro.tarjetasRojas;
    faltas = otro.faltas;
    jugadores = new Jugador[JUGADORES_POR_EQUIPO];
    for (int i = 0; i < JUGADORES_POR_EQUIPO; ++i)
        jugadores[i] = otro.jugadores[i];
    return *this;
}

int Equipo::getRanking() const { return ranking; }
std::string Equipo::getPais() const { return pais; }
std::string Equipo::getConfederacion() const { return confederacion; }
double Equipo::getPromedioGolesAFavor() const {
    int total = partidosGanados + partidosEmpatados + partidosPerdidos;
    if (total == 0) return 0;
    return static_cast<double>(golesAFavor) / total;
}
double Equipo::getPromedioGolesEnContra() const {
    int total = partidosGanados + partidosEmpatados + partidosPerdidos;
    if (total == 0) return 0;
    return static_cast<double>(golesEnContra) / total;
}
int Equipo::getGolesAFavor() const { return golesAFavor; }
int Equipo::getGolesEnContra() const { return golesEnContra; }
int Equipo::getPartidosGanados() const { return partidosGanados; }
int Equipo::getPartidosEmpatados() const { return partidosEmpatados; }
int Equipo::getPartidosPerdidos() const { return partidosPerdidos; }
int Equipo::getTarjetasAmarillas() const { return tarjetasAmarillas; }
int Equipo::getTarjetasRojas() const { return tarjetasRojas; }
int Equipo::getFaltas() const { return faltas; }

void Equipo::actualizarEstadisticas(int gf, int gc, int amarillas, int rojas,
                                    int faltasPartido, bool ganado, bool empatado, bool perdido) {
    golesAFavor += gf;
    golesEnContra += gc;
    tarjetasAmarillas += amarillas;
    tarjetasRojas += rojas;
    faltas += faltasPartido;
    if (ganado) partidosGanados++;
    else if (empatado) partidosEmpatados++;
    else if (perdido) partidosPerdidos++;
}

void Equipo::repartirGolesHistoricos() {
    int totalGoles = golesAFavor;
    int porJugador = totalGoles / JUGADORES_POR_EQUIPO;
    int resto = totalGoles % JUGADORES_POR_EQUIPO;
    for (int i = 0; i < JUGADORES_POR_EQUIPO; ++i) {
        int asignar = porJugador;
        if (i < resto) asignar++;
        jugadores[i].setGoles(asignar);
    }
}

void Equipo::setJugador(int idx, const Jugador& j) {
    if (idx >= 0 && idx < JUGADORES_POR_EQUIPO)
        jugadores[idx] = j;
}

Jugador& Equipo::getJugador(int idx) {
    return jugadores[idx];
}

bool Equipo::operator<(const Equipo& otro) const {
    return ranking < otro.ranking;
}

std::ostream& operator<<(std::ostream& os, const Equipo& eq) {
    os << eq.pais << " (Ranking " << eq.ranking << ", " << eq.confederacion << ")";
    return os;
}
