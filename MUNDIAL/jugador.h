#ifndef JUGADOR_H
#define JUGADOR_H

#include <iostream>
#include <string>

class Jugador {
private:
    std::string nombre;
    std::string apellido;
    int numeroCamiseta;
    int partidosJugados;
    int goles;
    int minutosTotales;
    int asistencias;
    int tarjetasAmarillas;
    int tarjetasRojas;
    int faltas;

public:
    Jugador();
    Jugador(const std::string& nom, const std::string& ape, int num);
    Jugador(const Jugador& otro);
    ~Jugador();

    // Getters
    std::string getNombre() const;
    std::string getApellido() const;
    int getNumero() const;
    int getGoles() const;
    int getPartidosJugados() const;
    int getMinutos() const;
    int getAsistencias() const;
    int getAmarillas() const;
    int getRojas() const;
    int getFaltas() const;

    // Setters
    void setGoles(int g);
    void setPartidosJugados(int p);
    void setMinutos(int m);
    void setAsistencias(int a);
    void setAmarillas(int am);
    void setRojas(int r);
    void setFaltas(int f);

    void acumularEstadisticasPartido(int golesPartido, int amarillasPartido,
                                     int rojasPartido, int faltasPartido, int minutosPartido);

    Jugador& operator+=(const Jugador& otro);
    friend std::ostream& operator<<(std::ostream& os, const Jugador& j);
};

#endif
