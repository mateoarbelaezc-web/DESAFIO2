#include "Jugador.h"

Jugador::Jugador() : nombre(""), apellido(""), numeroCamiseta(0),
    partidosJugados(0), goles(0), minutosTotales(0), asistencias(0),
    tarjetasAmarillas(0), tarjetasRojas(0), faltas(0) {}

Jugador::Jugador(const std::string& nom, const std::string& ape, int num)
    : nombre(nom), apellido(ape), numeroCamiseta(num),
    partidosJugados(0), goles(0), minutosTotales(0), asistencias(0),
    tarjetasAmarillas(0), tarjetasRojas(0), faltas(0) {}

Jugador::Jugador(const Jugador& otro)
    : nombre(otro.nombre), apellido(otro.apellido), numeroCamiseta(otro.numeroCamiseta),
    partidosJugados(otro.partidosJugados), goles(otro.goles),
    minutosTotales(otro.minutosTotales), asistencias(otro.asistencias),
    tarjetasAmarillas(otro.tarjetasAmarillas), tarjetasRojas(otro.tarjetasRojas),
    faltas(otro.faltas) {}

Jugador::~Jugador() {}
Jugador& Jugador::operator=(const Jugador& otro) {
    if (this == &otro) return *this;
    nombre = otro.nombre;
    apellido = otro.apellido;
    numeroCamiseta = otro.numeroCamiseta;
    partidosJugados = otro.partidosJugados;
    goles = otro.goles;
    minutosTotales = otro.minutosTotales;
    asistencias = otro.asistencias;
    tarjetasAmarillas = otro.tarjetasAmarillas;
    tarjetasRojas = otro.tarjetasRojas;
    faltas = otro.faltas;
    return *this;
}

std::string Jugador::getNombre() const { return nombre; }
std::string Jugador::getApellido() const { return apellido; }
int Jugador::getNumero() const { return numeroCamiseta; }
int Jugador::getGoles() const { return goles; }
int Jugador::getPartidosJugados() const { return partidosJugados; }
int Jugador::getMinutos() const { return minutosTotales; }
int Jugador::getAsistencias() const { return asistencias; }
int Jugador::getAmarillas() const { return tarjetasAmarillas; }
int Jugador::getRojas() const { return tarjetasRojas; }
int Jugador::getFaltas() const { return faltas; }

void Jugador::setGoles(int g) { goles = g; }
void Jugador::setPartidosJugados(int p) { partidosJugados = p; }
void Jugador::setMinutos(int m) { minutosTotales = m; }
void Jugador::setAsistencias(int a) { asistencias = a; }
void Jugador::setAmarillas(int am) { tarjetasAmarillas = am; }
void Jugador::setRojas(int r) { tarjetasRojas = r; }
void Jugador::setFaltas(int f) { faltas = f; }

void Jugador::acumularEstadisticasPartido(int golesPartido, int amarillasPartido,
                                          int rojasPartido, int faltasPartido, int minutosPartido) {
    goles += golesPartido;
    tarjetasAmarillas += amarillasPartido;
    tarjetasRojas += rojasPartido;
    faltas += faltasPartido;
    minutosTotales += minutosPartido;
    partidosJugados++;
}

Jugador& Jugador::operator+=(const Jugador& otro) {
    goles += otro.goles;
    partidosJugados += otro.partidosJugados;
    minutosTotales += otro.minutosTotales;
    asistencias += otro.asistencias;
    tarjetasAmarillas += otro.tarjetasAmarillas;
    tarjetasRojas += otro.tarjetasRojas;
    faltas += otro.faltas;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Jugador& j) {
    os << j.nombre << " " << j.apellido << " (#" << j.numeroCamiseta << ") - Goles: " << j.goles
       << ", Amarillas: " << j.tarjetasAmarillas << ", Rojas: " << j.tarjetasRojas;
    return os;
}
