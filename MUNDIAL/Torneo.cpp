#include "Torneo.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>

// ----------------------------------------------------------------------
// Constructor y destructor
// ----------------------------------------------------------------------
Torneo::Torneo() : numEquipos(0) {
    for (int i = 0; i < NUM_GRUPOS; ++i)
        grupos[i] = new Grupo('A' + i);
    for (int i = 0; i < BOMBOS; ++i)
        for (int j = 0; j < EQUIPOS_POR_BOMBO; ++j)
            bombos[i][j] = nullptr;
}

Torneo::~Torneo() {
    for (int i = 0; i < NUM_GRUPOS; ++i)
        delete grupos[i];
}

// ----------------------------------------------------------------------
// Ordenamiento auxiliar
// ----------------------------------------------------------------------
void Torneo::ordenarEquiposPorRanking(Equipo* lista[], int n) {
    for (int i = 0; i < n-1; ++i)
        for (int j = 0; j < n-i-1; ++j)
            if (lista[j]->getRanking() > lista[j+1]->getRanking()) {
                Equipo* temp = lista[j];
                lista[j] = lista[j+1];
                lista[j+1] = temp;
            }
}

// ----------------------------------------------------------------------
// Distribución de bombos (sorteo inicial)
// ----------------------------------------------------------------------
void Torneo::distribuirBombos() {
    Equipo* anfitrion = nullptr;
    Equipo* otros[NUM_EQUIPOS];
    int otrosCount = 0;
    for (int i = 0; i < numEquipos; ++i) {
        if (equipos[i].getPais() == "United States")
            anfitrion = &equipos[i];
        else
            otros[otrosCount++] = &equipos[i];
    }
    if (!anfitrion) {
        anfitrion = &equipos[0];
        otrosCount--;
        for (int i = 1; i < numEquipos; ++i) otros[i-1] = &equipos[i];
    }
    ordenarEquiposPorRanking(otros, otrosCount);
    bombos[0][0] = anfitrion;
    for (int i = 1; i < EQUIPOS_POR_BOMBO; ++i)
        bombos[0][i] = otros[i-1];
    int idx = EQUIPOS_POR_BOMBO - 1;
    for (int bombo = 1; bombo < BOMBOS; ++bombo)
        for (int i = 0; i < EQUIPOS_POR_BOMBO; ++i)
            bombos[bombo][i] = otros[idx++];
}

// ----------------------------------------------------------------------
// Restricción de confederaciones para el sorteo
// ----------------------------------------------------------------------
bool Torneo::respetaConfederaciones(Equipo* equipo, Grupo* grupo, int& uefaCount) {
    std::string conf = equipo->getConfederacion();
    for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i) {
        Equipo* eq = grupo->getEquipo(i);
        if (!eq) continue;
        if (eq->getConfederacion() == conf) {
            if (conf == "UEFA" && uefaCount < 2) return true;
            else return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------
// Sorteo de grupos
// ----------------------------------------------------------------------
void Torneo::sortearGrupos() {
    // Mezclar cada bombo
    for (int b = 0; b < BOMBOS; ++b) {
        for (int i = 0; i < EQUIPOS_POR_BOMBO; ++i) {
            int j = rand() % EQUIPOS_POR_BOMBO;
            Equipo* temp = bombos[b][i];
            bombos[b][i] = bombos[b][j];
            bombos[b][j] = temp;
        }
    }
    // Asignar a grupos
    for (int bombo = 0; bombo < BOMBOS; ++bombo) {
        for (int g = 0; g < NUM_GRUPOS; ++g) {
            Equipo* equipo = bombos[bombo][g];
            int uefaCount = 0;
            for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i) {
                Equipo* e = grupos[g]->getEquipo(i);
                if (e && e->getConfederacion() == "UEFA") uefaCount++;
            }
            if (!respetaConfederaciones(equipo, grupos[g], uefaCount)) {
                for (int otro = 0; otro < NUM_GRUPOS; ++otro) {
                    if (otro == g) continue;
                    int uefaCount2 = 0;
                    for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i) {
                        Equipo* e = grupos[otro]->getEquipo(i);
                        if (e && e->getConfederacion() == "UEFA") uefaCount2++;
                    }
                    if (respetaConfederaciones(equipo, grupos[otro], uefaCount2)) {
                        Equipo* aux = bombos[bombo][otro];
                        bombos[bombo][otro] = equipo;
                        bombos[bombo][g] = aux;
                        break;
                    }
                }
            }
            grupos[g]->setEquipo(bombo, equipo);
        }
    }
    imprimirGrupos();
}

void Torneo::imprimirGrupos() {
    std::cout << "\n=== GRUPOS SORTEADOS ===\n";
    for (int g = 0; g < NUM_GRUPOS; ++g) {
        std::cout << "Grupo " << char('A'+g) << ": ";
        for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i) {
            Equipo* eq = grupos[g]->getEquipo(i);
            if (eq) std::cout << eq->getPais() << " (" << eq->getConfederacion() << ") ";
        }
        std::cout << std::endl;
    }
}

// ----------------------------------------------------------------------
// Carga de equipos desde CSV
// ----------------------------------------------------------------------
void Torneo::cargarEquipos(const std::string& archivo) {
    std::ifstream file(archivo.c_str());
    if (!file.is_open()) {
        std::cerr << "Error al abrir " << archivo << std::endl;
        return;
    }
    std::string linea;
    std::getline(file, linea);
    std::getline(file, linea);
    int idx = 0;
    while (std::getline(file, linea) && idx < NUM_EQUIPOS) {
        std::stringstream ss(linea);
        std::string token;
        int ranking, gf, gc, pg, pe, pp;
        std::string pais, dt, fed, conf;
        std::getline(ss, token, ';'); ranking = std::atoi(token.c_str());
        std::getline(ss, pais, ';');
        std::getline(ss, dt, ';');
        std::getline(ss, fed, ';');
        std::getline(ss, conf, ';');
        std::getline(ss, token, ';'); gf = std::atoi(token.c_str());
        std::getline(ss, token, ';'); gc = std::atoi(token.c_str());
        std::getline(ss, token, ';'); pg = std::atoi(token.c_str());
        std::getline(ss, token, ';'); pe = std::atoi(token.c_str());
        std::getline(ss, token, ';'); pp = std::atoi(token.c_str());

        Equipo tmp(ranking, pais, dt, fed, conf, gf, gc, pg, pe, pp);
        for (int j = 0; j < JUGADORES_POR_EQUIPO; ++j) {
            std::string nom = "nombre" + std::to_string(j+1);
            std::string ape = "apellido" + std::to_string(j+1);
            Jugador jug(nom, ape, j+1);
            tmp.setJugador(j, jug);
        }
        tmp.repartirGolesHistoricos();
        equipos[idx] = tmp;
        idx++;
    }
    numEquipos = idx;
    file.close();
    std::cout << "Cargados " << numEquipos << " equipos.\n";
}

void Torneo::conformarBombos() {
    distribuirBombos();
    std::cout << "\n=== BOMBOS ===\n";
    for (int b = 0; b < BOMBOS; ++b) {
        std::cout << "Bombo " << b+1 << ": ";
        for (int i = 0; i < EQUIPOS_POR_BOMBO; ++i)
            if (bombos[b][i]) std::cout << bombos[b][i]->getPais() << " ";
        std::cout << std::endl;
    }
}

// ----------------------------------------------------------------------
// Fase de grupos
// ----------------------------------------------------------------------
void Torneo::simularFaseGrupos(const std::string& fechaInicio) {
    for (int g = 0; g < NUM_GRUPOS; ++g) {
        grupos[g]->configurarPartidos(fechaInicio);
        grupos[g]->simularEtapa(false);
        grupos[g]->actualizarHistoricos();
        std::cout << "\n--- Grupo " << char('A'+g) << " ---\n";
        grupos[g]->imprimirPartidos();
        grupos[g]->imprimirTabla();
    }
}

// ----------------------------------------------------------------------
// Recolección de clasificados
// ----------------------------------------------------------------------
void Torneo::recolectarClasificados(Clasificado primeros[], int& pCount,
                                    Clasificado segundos[], int& sCount,
                                    Clasificado terceros[], int& tCount) {
    pCount = sCount = tCount = 0;
    for (int g = 0; g < NUM_GRUPOS; ++g) {
        Equipo *p1, *p2, *p3;
        int pts1, dif1, gf1, pts2, dif2, gf2, pts3, dif3, gf3;
        grupos[g]->obtenerClasificados(p1, p2, p3, pts1, dif1, gf1, pts2, dif2, gf2, pts3, dif3, gf3);
        primeros[pCount++] = {p1, pts1, dif1, gf1, char('A'+g), 1};
        segundos[sCount++] = {p2, pts2, dif2, gf2, char('A'+g), 2};
        terceros[tCount++] = {p3, pts3, dif3, gf3, char('A'+g), 3};
    }
}

void Torneo::ordenarTerceros(Clasificado terceros[], int& tCount) {
    for (int i = 0; i < tCount-1; ++i) {
        for (int j = i+1; j < tCount; ++j) {
            bool intercambiar = false;
            if (terceros[j].puntos > terceros[i].puntos) intercambiar = true;
            else if (terceros[j].puntos == terceros[i].puntos) {
                if (terceros[j].diferenciaGoles > terceros[i].diferenciaGoles) intercambiar = true;
                else if (terceros[j].diferenciaGoles == terceros[i].diferenciaGoles &&
                         terceros[j].golesFavor > terceros[i].golesFavor) intercambiar = true;
            }
            if (intercambiar) {
                Clasificado temp = terceros[i];
                terceros[i] = terceros[j];
                terceros[j] = temp;
            }
        }
    }
    tCount = (tCount < 8) ? tCount : 8;
}

bool Torneo::mismoGrupo(Equipo* a, Equipo* b) {
    for (int g = 0; g < NUM_GRUPOS; ++g) {
        for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i) {
            Equipo* eq = grupos[g]->getEquipo(i);
            if (eq == a) {
                for (int j = 0; j < EQUIPOS_POR_GRUPO; ++j)
                    if (grupos[g]->getEquipo(j) == b) return true;
                return false;
            }
        }
    }
    return false;
}

// ----------------------------------------------------------------------
// Generación de enfrentamientos de R16
// ----------------------------------------------------------------------
void Torneo::generarEnfrentamientosR16(Clasificado primeros[], int pCount,
                                       Clasificado segundos[], int sCount,
                                       Clasificado terceros[], int tCount,
                                       Equipo* enfrentamientos[32], int& eCount) {
    eCount = 0;
    int terceroIdx = 0;
    // 1. Cabezas de grupo vs mejores terceros (8 partidos)
    for (int i = 0; i < 8; ++i) {
        Equipo* cabeza = primeros[i].equipo;
        Equipo* tercero = terceros[terceroIdx].equipo;
        if (mismoGrupo(cabeza, tercero)) {
            for (int j = 0; j < tCount; ++j) {
                if (!mismoGrupo(cabeza, terceros[j].equipo)) {
                    tercero = terceros[j].equipo;
                    Clasificado temp = terceros[terceroIdx];
                    terceros[terceroIdx] = terceros[j];
                    terceros[j] = temp;
                    break;
                }
            }
        }
        enfrentamientos[eCount++] = cabeza;
        enfrentamientos[eCount++] = tercero;
        terceroIdx++;
    }
    // 2. Cabezas restantes vs los 4 peores segundos
    Clasificado peoresSegundos[12];
    for (int i = 0; i < sCount; ++i) peoresSegundos[i] = segundos[i];
    for (int i = 0; i < sCount-1; ++i)
        for (int j = i+1; j < sCount; ++j)
            if (peoresSegundos[j].puntos < peoresSegundos[i].puntos) {
                Clasificado temp = peoresSegundos[i];
                peoresSegundos[i] = peoresSegundos[j];
                peoresSegundos[j] = temp;
            }
    int cabezaIdx = 8;
    for (int i = 0; i < 4 && cabezaIdx < pCount; ++i) {
        Equipo* cabeza = primeros[cabezaIdx++].equipo;
        Equipo* segundo = peoresSegundos[i].equipo;
        if (mismoGrupo(cabeza, segundo)) {
            for (int j = i+1; j < sCount; ++j) {
                if (!mismoGrupo(cabeza, peoresSegundos[j].equipo)) {
                    segundo = peoresSegundos[j].equipo;
                    Clasificado temp = peoresSegundos[i];
                    peoresSegundos[i] = peoresSegundos[j];
                    peoresSegundos[j] = temp;
                    break;
                }
            }
        }
        enfrentamientos[eCount++] = cabeza;
        enfrentamientos[eCount++] = segundo;
    }
    // 3. Segundos restantes se enfrentan entre sí
    int restantes = sCount - 4;
    for (int i = 0; i < restantes; i += 2) {
        Equipo* seg1 = peoresSegundos[4 + i].equipo;
        Equipo* seg2 = peoresSegundos[4 + i + 1].equipo;
        if (mismoGrupo(seg1, seg2)) {
            for (int j = 4 + i + 2; j < sCount; ++j) {
                if (!mismoGrupo(seg1, peoresSegundos[j].equipo)) {
                    seg2 = peoresSegundos[j].equipo;
                    break;
                }
            }
        }
        enfrentamientos[eCount++] = seg1;
        enfrentamientos[eCount++] = seg2;
    }
}

// ----------------------------------------------------------------------
// Simulación de una ronda eliminatoria genérica
// ----------------------------------------------------------------------
void Torneo::simularRonda(Equipo** participantes, int numParticipantes,
                          const std::string& fecha, bool esEliminatoria,
                          Equipo** ganadores, int& numGanadores) {
    if (numParticipantes % 2 != 0) return;
    int numPartidos = numParticipantes / 2;
    Partido** partidos = new Partido*[numPartidos];
    for (int i = 0; i < numPartidos; ++i) {
        partidos[i] = new Partido(participantes[2*i], participantes[2*i+1], fecha, "00:00", "nombreSede");
        partidos[i]->simular(esEliminatoria);
        partidos[i]->actualizarHistoricos();
        partidos[i]->imprimirResultado();
        partidos[i]->imprimirGoleadores();
    }
    numGanadores = 0;
    for (int i = 0; i < numPartidos; ++i) {
        if (partidos[i]->getGolesEq1() > partidos[i]->getGolesEq2())
            ganadores[numGanadores++] = partidos[i]->getEquipo1();
        else
            ganadores[numGanadores++] = partidos[i]->getEquipo2();
        delete partidos[i];
    }
    delete[] partidos;
}

// ----------------------------------------------------------------------
// Estadísticas finales (con manejo de punteros nulos)
// ----------------------------------------------------------------------
void Torneo::calcularEstadisticasFinales(Equipo* campeon, Equipo* subcampeon,
                                         Equipo* tercero, Equipo* cuarto) {
    std::cout << "\n=== ESTADÍSTICAS FINALES ===\n";
    std::cout << "Ranking de los 4 primeros puestos:\n";
    std::cout << "1. " << campeon->getPais() << "\n";
    std::cout << "2. " << subcampeon->getPais() << "\n";
    if (tercero)
        std::cout << "3. " << tercero->getPais() << "\n";
    else
        std::cout << "3. No definido\n";
    if (cuarto)
        std::cout << "4. " << cuarto->getPais() << "\n";
    else
        std::cout << "4. No definido\n";

    // Máximo goleador del equipo campeón
    Jugador* maxGoleadorCampeon = &campeon->getJugador(0);
    for (int i = 1; i < JUGADORES_POR_EQUIPO; ++i) {
        if (campeon->getJugador(i).getGoles() > maxGoleadorCampeon->getGoles())
            maxGoleadorCampeon = &campeon->getJugador(i);
    }
    std::cout << "\nMáximo goleador del campeón (" << campeon->getPais() << "): "
              << maxGoleadorCampeon->getNombre() << " " << maxGoleadorCampeon->getApellido()
              << " - " << maxGoleadorCampeon->getGoles() << " goles\n";

    // Tres máximos goleadores del torneo
    Jugador* mejores[3] = {nullptr, nullptr, nullptr};
    for (int i = 0; i < numEquipos; ++i) {
        for (int j = 0; j < JUGADORES_POR_EQUIPO; ++j) {
            Jugador* jug = &equipos[i].getJugador(j);
            int g = jug->getGoles();
            if (!mejores[0] || g > mejores[0]->getGoles()) {
                mejores[2] = mejores[1];
                mejores[1] = mejores[0];
                mejores[0] = jug;
            } else if (!mejores[1] || g > mejores[1]->getGoles()) {
                mejores[2] = mejores[1];
                mejores[1] = jug;
            } else if (!mejores[2] || g > mejores[2]->getGoles()) {
                mejores[2] = jug;
            }
        }
    }
    std::cout << "\nTres máximos goleadores del mundial:\n";
    for (int i = 0; i < 3 && mejores[i]; ++i) {
        std::cout << i+1 << ". " << mejores[i]->getNombre() << " " << mejores[i]->getApellido()
        << " (#" << mejores[i]->getNumero() << ") - "
        << mejores[i]->getGoles() << " goles\n";
    }

    // Equipo con más goles históricos
    Equipo* maxGolesEquipo = &equipos[0];
    for (int i = 1; i < numEquipos; ++i) {
        if (equipos[i].getGolesAFavor() > maxGolesEquipo->getGolesAFavor())
            maxGolesEquipo = &equipos[i];
    }
    std::cout << "\nEquipo con más goles históricos: " << maxGolesEquipo->getPais()
              << " (" << maxGolesEquipo->getGolesAFavor() << " goles)\n";
}

// ----------------------------------------------------------------------
// Simulación completa de eliminatorias (con tercer puesto)
// ----------------------------------------------------------------------
void Torneo::simularEliminatorias(const std::string& fechaEliminatorias) {
    Clasificado primeros[12], segundos[12], terceros[12];
    int pCount, sCount, tCount;
    recolectarClasificados(primeros, pCount, segundos, sCount, terceros, tCount);
    ordenarTerceros(terceros, tCount);

    Equipo* ronda32[32];
    int numRonda32;
    generarEnfrentamientosR16(primeros, pCount, segundos, sCount, terceros, tCount, ronda32, numRonda32);

    std::cout << "\n=== DIECISEISAVOS DE FINAL ===\n";
    Equipo* ronda16[16];
    int numRonda16;
    simularRonda(ronda32, numRonda32, fechaEliminatorias, true, ronda16, numRonda16);

    std::cout << "\n=== OCTAVOS DE FINAL ===\n";
    Equipo* ronda8[8];
    int numRonda8;
    simularRonda(ronda16, numRonda16, fechaEliminatorias, true, ronda8, numRonda8);

    std::cout << "\n=== CUARTOS DE FINAL ===\n";
    Equipo* ronda4[4];
    int numRonda4;
    simularRonda(ronda8, numRonda8, fechaEliminatorias, true, ronda4, numRonda4);

    // Semifinales (manualmente para guardar perdedores)
    std::cout << "\n=== SEMIFINALES ===\n";
    Partido* semi1 = new Partido(ronda4[0], ronda4[1], fechaEliminatorias, "00:00", "nombreSede");
    Partido* semi2 = new Partido(ronda4[2], ronda4[3], fechaEliminatorias, "00:00", "nombreSede");
    semi1->simular(true);
    semi1->actualizarHistoricos();
    semi1->imprimirResultado();
    semi1->imprimirGoleadores();
    semi2->simular(true);
    semi2->actualizarHistoricos();
    semi2->imprimirResultado();
    semi2->imprimirGoleadores();

    Equipo* ganador1 = (semi1->getGolesEq1() > semi1->getGolesEq2()) ? semi1->getEquipo1() : semi1->getEquipo2();
    Equipo* perdedor1 = (ganador1 == semi1->getEquipo1()) ? semi1->getEquipo2() : semi1->getEquipo1();
    Equipo* ganador2 = (semi2->getGolesEq1() > semi2->getGolesEq2()) ? semi2->getEquipo1() : semi2->getEquipo2();
    Equipo* perdedor2 = (ganador2 == semi2->getEquipo1()) ? semi2->getEquipo2() : semi2->getEquipo1();

    delete semi1;
    delete semi2;

    // Partido por el tercer puesto
    std::cout << "\n=== PARTIDO POR EL TERCER PUESTO ===\n";
    Partido* tercerPuesto = new Partido(perdedor1, perdedor2, fechaEliminatorias, "00:00", "nombreSede");
    tercerPuesto->simular(true);
    tercerPuesto->actualizarHistoricos();
    tercerPuesto->imprimirResultado();
    tercerPuesto->imprimirGoleadores();
    Equipo* tercero = (tercerPuesto->getGolesEq1() > tercerPuesto->getGolesEq2()) ? tercerPuesto->getEquipo1() : tercerPuesto->getEquipo2();
    Equipo* cuarto = (tercero == tercerPuesto->getEquipo1()) ? tercerPuesto->getEquipo2() : tercerPuesto->getEquipo1();
    delete tercerPuesto;

    // Final
    std::cout << "\n=== FINAL ===\n";
    Partido* final = new Partido(ganador1, ganador2, fechaEliminatorias, "00:00", "nombreSede");
    final->simular(true);
    final->actualizarHistoricos();
    final->imprimirResultado();
    final->imprimirGoleadores();
    Equipo* campeon = (final->getGolesEq1() > final->getGolesEq2()) ? final->getEquipo1() : final->getEquipo2();
    Equipo* subcampeon = (campeon == final->getEquipo1()) ? final->getEquipo2() : final->getEquipo1();
    delete final;

    calcularEstadisticasFinales(campeon, subcampeon, tercero, cuarto);
}

void Torneo::generarEstadisticasFinales() {
    // Este método se deja vacío porque la funcionalidad ya está en calcularEstadisticasFinales
}
