#include "Torneo.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "partido.h"
#include "Metricas.h"



static std::string extraerCampo(const std::string& linea, int& pos) {
    std::string campo;
    int n = static_cast<int>(linea.size());
    while (pos < n && linea[pos] != ';' && linea[pos] != '\r' && linea[pos] != '\n') {
        campo += linea[pos];
        pos++;
    }
    if (pos < n && linea[pos] == ';') pos++; // saltar el ';'
    return campo;
}

static std::string sumarDias(const std::string& fecha, int dias) {
    // fecha formato "DD/MM/YYYY"
    int dia  = parsearInt(fecha, 0, 2);
    int mes  = parsearInt(fecha, 3, 5);
    int anio = parsearInt(fecha, 6, 10);
    int diasPorMes[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    dia += dias;
    while (dia > diasPorMes[mes-1]) {
        dia -= diasPorMes[mes-1];
        mes++;
        if (mes > 12) { mes = 1; anio++; }
    }
    return intAString(dia, 2) + "/" + intAString(mes, 2) + "/" + intAString(anio, 4);
}

Torneo::Torneo() : numEquipos(0),
    campeon(nullptr), subcampeon(nullptr),
    terceroLugar(nullptr), cuartoLugar(nullptr),
    equiposCargados(false), bombosConfigurados(false),
    gruposSorteados(false), faseGruposSimulada(false),
    eliminatoriasSimuladas(false),
    countR16(0), countR8(0), countR4(0) {
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

Torneo::Torneo(const Torneo& otro) : numEquipos(otro.numEquipos),
    countR16(otro.countR16), countR8(otro.countR8), countR4(otro.countR4) {
    for (int i = 0; i < numEquipos; ++i)
        equipos[i] = otro.equipos[i];
    for (int i = 0; i < NUM_GRUPOS; ++i)
        grupos[i] = new Grupo(*otro.grupos[i]);
    for (int i = 0; i < BOMBOS; ++i)
        for (int j = 0; j < EQUIPOS_POR_BOMBO; ++j)
            bombos[i][j] = otro.bombos[i][j];
    for (int i = 0; i < countR16; ++i)
        participantesR16[i] = otro.participantesR16[i];
    for (int i = 0; i < countR8; ++i)
        participantesR8[i] = otro.participantesR8[i];
    for (int i = 0; i < countR4; ++i)
        participantesR4[i] = otro.participantesR4[i];
}

// Operador de asignación de Torneo
Torneo& Torneo::operator=(const Torneo& otro) {
    if (this == &otro) return *this;
    for (int i = 0; i < NUM_GRUPOS; ++i)
        delete grupos[i];
    numEquipos = otro.numEquipos;
    countR16 = otro.countR16;
    countR8 = otro.countR8;
    countR4 = otro.countR4;
    for (int i = 0; i < numEquipos; ++i)
        equipos[i] = otro.equipos[i];
    for (int i = 0; i < NUM_GRUPOS; ++i)
        grupos[i] = new Grupo(*otro.grupos[i]);
    for (int i = 0; i < BOMBOS; ++i)
        for (int j = 0; j < EQUIPOS_POR_BOMBO; ++j)
            bombos[i][j] = otro.bombos[i][j];
    for (int i = 0; i < countR16; ++i)
        participantesR16[i] = otro.participantesR16[i];
    for (int i = 0; i < countR8; ++i)
        participantesR8[i] = otro.participantesR8[i];
    for (int i = 0; i < countR4; ++i)
        participantesR4[i] = otro.participantesR4[i];
    return *this;
}


// Ordenamiento auxiliar
void Torneo::ordenarEquiposPorRanking(Equipo* lista[], int n) {
    for (int i = 0; i < n-1; ++i)
        for (int j = 0; j < n-i-1; ++j) {
            incIteraciones(1);
            if (lista[j]->getRanking() > lista[j+1]->getRanking()) {
                Equipo* temp = lista[j];
                lista[j] = lista[j+1];
                lista[j+1] = temp;
            }
        }
}

// Distribución de bombos (sorteo inicial)
void Torneo::distribuirBombos() {
    Equipo* anfitrion = nullptr;
    Equipo* otros[NUM_EQUIPOS];
    int otrosCount = 0;
    for (int i = 0; i < numEquipos; ++i) {
        incIteraciones(1);
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

// Restricción de confederaciones para el sorteo
bool Torneo::respetaConfederaciones(Equipo* equipo, Grupo* grupo) {
    std::string conf = equipo->getConfederacion();
    int countConf = 0;
    for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i) {
        incIteraciones(1);
        Equipo* eq = grupo->getEquipo(i);
        if (!eq) continue;
        if (eq->getConfederacion() == conf) {
            countConf++;
            if (conf == "UEFA" && countConf >= 2) return false;
            if (conf != "UEFA") return false;
        }
    }
    return true;
} 
// Sorteo de grupos
void Torneo::sortearGrupos() {
    bool sorteoValido = false;
    int intentos = 0;

    while (!sorteoValido && intentos < 100000) {
        intentos++;
        incIteraciones(1);

        // Limpiar grupos
        for (int g = 0; g < NUM_GRUPOS; ++g)
            for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i)
                grupos[g]->setEquipo(i, nullptr);

        // Mezclar cada bombo
        for (int b = 0; b < BOMBOS; ++b)
            for (int i = EQUIPOS_POR_BOMBO - 1; i > 0; --i) {
                int j = rand() % (i + 1);
                Equipo* temp = bombos[b][i];
                bombos[b][i] = bombos[b][j];
                bombos[b][j] = temp;
            }

        // Asignar bombo por bombo
        sorteoValido = true;
        for (int bombo = 0; bombo < BOMBOS && sorteoValido; ++bombo) {

            // Crear lista de grupos disponibles para este bombo
            int gruposDisponibles[NUM_GRUPOS];
            int numDisponibles = NUM_GRUPOS;
            for (int g = 0; g < NUM_GRUPOS; ++g)
                gruposDisponibles[g] = g;

            // Asignar cada equipo del bombo a un grupo disponible
            for (int e = 0; e < EQUIPOS_POR_BOMBO && sorteoValido; ++e) {
                Equipo* equipo = bombos[bombo][e];
                bool asignado = false;

                // Mezclar grupos disponibles para aleatorizar
                for (int i = numDisponibles - 1; i > 0; --i) {
                    int j = rand() % (i + 1);
                    int temp = gruposDisponibles[i];
                    gruposDisponibles[i] = gruposDisponibles[j];
                    gruposDisponibles[j] = temp;
                }

                // Buscar grupo válido
                for (int i = 0; i < numDisponibles; ++i) {
                    int g = gruposDisponibles[i];
                    if (respetaConfederaciones(equipo, grupos[g])) {
                        grupos[g]->setEquipo(bombo, equipo);
                        // Remover este grupo de disponibles
                        gruposDisponibles[i] = gruposDisponibles[--numDisponibles];
                        asignado = true;
                        break;
                    }
                }

                if (!asignado) sorteoValido = false;
            }
        }
    }

    if (!sorteoValido)
        std::cerr << "No se pudo generar sorteo valido despues de "
                  << intentos << " intentos\n";
    else {
        std::cout << "Sorteo valido encontrado en " << intentos
                  << " intento(s)\n";
        imprimirGrupos();
        gruposSorteados = true;
        }
        mostrarMetricas("Sorteo de grupos",
                        sizeof(int)*NUM_GRUPOS + sizeof(Equipo*)*NUM_EQUIPOS,
                        "rand() de <cstdlib>");      // otros[]
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

// Carga de equipos desde CSV
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
        incIteraciones(1);
        int ranking, gf, gc, pg, pe, pp;
        std::string pais, dt, fed, conf;
        int pos = 0;
        ranking = stringAInt(extraerCampo(linea, pos));
        pais    = extraerCampo(linea, pos);
        dt      = extraerCampo(linea, pos);
        fed     = extraerCampo(linea, pos);
        conf    = extraerCampo(linea, pos);
        gf      = stringAInt(extraerCampo(linea, pos));
        gc      = stringAInt(extraerCampo(linea, pos));
        pg      = stringAInt(extraerCampo(linea, pos));
        pe      = stringAInt(extraerCampo(linea, pos));
        pp      = stringAInt(extraerCampo(linea, pos));

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
    equiposCargados = true;
    mostrarMetricas("Carga de equipos",
                    sizeof(int)*6 + sizeof(std::string)*4,
                    "ifstream de <fstream>");
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
    bombosConfigurados = true;
    mostrarMetricas("Conformar bombos",
                    sizeof(Equipo*) * NUM_EQUIPOS,
                    "rand() de <cstdlib>");
}


// Fase de grupos
void Torneo::simularFaseGrupos(const std::string& fechaInicio) {
    int partidosPorDia[19] = {0};

    int ultimoDiaJugado[NUM_EQUIPOS];
    for (int i = 0; i < NUM_EQUIPOS; ++i) ultimoDiaJugado[i] = -99;

    int idxEquipoEnTorneo[NUM_GRUPOS][EQUIPOS_POR_GRUPO];
    for (int g = 0; g < NUM_GRUPOS; ++g) {
        for (int i = 0; i < EQUIPOS_POR_GRUPO; ++i) {
            Equipo* eq = grupos[g]->getEquipo(i);
            idxEquipoEnTorneo[g][i] = -1;
            for (int k = 0; k < numEquipos; ++k) {
                incIteraciones(1);
                if (&equipos[k] == eq) {
                    idxEquipoEnTorneo[g][i] = k;
                    break;
                }
            }
        }
    }

    for (int g = 0; g < NUM_GRUPOS; ++g) {
        incIteraciones(1);
        std::string fechas[3];

        int equiposJornada[3][4] = {
            {0, 1, 2, 3},
            {0, 2, 1, 3},
            {0, 3, 1, 2}
        };

        for (int jornada = 0; jornada < 3; ++jornada) {
            for (int dia = 0; dia < 19; ++dia) {
                if (partidosPorDia[dia] + 2 > 4) continue;
                bool diaValido = true;
                for (int e = 0; e < 4; ++e) {
                    int idx = idxEquipoEnTorneo[g][equiposJornada[jornada][e]];
                    if (idx != -1 && ultimoDiaJugado[idx] != -99) {
                        if (dia - ultimoDiaJugado[idx] < 3) {
                            diaValido = false;
                            break;
                        }
                    }
                }

                if (diaValido) {
                    fechas[jornada] = sumarDias(fechaInicio, dia);
                    partidosPorDia[dia] += 2;
                    // Actualizar último día jugado para los 4 equipos
                    for (int e = 0; e < 4; ++e) {
                        int idx = idxEquipoEnTorneo[g][equiposJornada[jornada][e]];
                        if (idx != -1) ultimoDiaJugado[idx] = dia;
                    }
                    break;
                }
            }
        }

        grupos[g]->configurarPartidos(fechas[0], fechas[1], fechas[2]);
    }

    // Simular y mostrar resultados
    for (int g = 0; g < NUM_GRUPOS; ++g) {
        grupos[g]->simularEtapa(false);
        grupos[g]->actualizarHistoricos();
        std::cout << "\n--- Grupo " << char('A'+g) << " ---\n";
        grupos[g]->imprimirPartidos();
        grupos[g]->imprimirTabla();
    }
    faseGruposSimulada = true;
    mostrarMetricas("Fase de grupos",
                    sizeof(int)*19 + sizeof(int)*NUM_EQUIPOS + sizeof(int)*NUM_GRUPOS*EQUIPOS_POR_GRUPO,
                    "rand() de <cstdlib>, pow() de <cmath>"); // idxEquipoEnTorneo
}

// Recolección de clasificados
void Torneo::recolectarClasificados(Clasificado primeros[], int& pCount,
                                    Clasificado segundos[], int& sCount,
                                    Clasificado terceros[], int& tCount) {
    pCount = sCount = tCount = 0;
    for (int g = 0; g < NUM_GRUPOS; ++g) {
        Equipo *p1, *p2, *p3;
        int pts1, dif1, gf1, pts2, dif2, gf2, pts3, dif3, gf3;
        grupos[g]->obtenerClasificados(p1, p2, p3, pts1, dif1, gf1,
                                       pts2, dif2, gf2, pts3, dif3, gf3);

        Clasificado c1;
        c1.equipo = p1; c1.puntos = pts1; c1.diferenciaGoles = dif1;
        c1.golesFavor = gf1; c1.grupo = char('A'+g); c1.posicion = 1;
        primeros[pCount++] = c1;

        Clasificado c2;
        c2.equipo = p2; c2.puntos = pts2; c2.diferenciaGoles = dif2;
        c2.golesFavor = gf2; c2.grupo = char('A'+g); c2.posicion = 2;
        segundos[sCount++] = c2;

        Clasificado c3;
        c3.equipo = p3; c3.puntos = pts3; c3.diferenciaGoles = dif3;
        c3.golesFavor = gf3; c3.grupo = char('A'+g); c3.posicion = 3;
        terceros[tCount++] = c3;
    }
}

void Torneo::ordenarTerceros(Clasificado terceros[], int& tCount) {
    for (int i = 0; i < tCount-1; ++i) {
        for (int j = i+1; j < tCount; ++j) {
            Torneo::incIteraciones(1);
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

void Torneo::contarConfederaciones(Equipo** lista, int n) {
    std::string confs[10];
    int conteo[10] = {0};
    int totalConfs = 0;

    for (int i = 0; i < n; ++i) {
        std::string conf = lista[i]->getConfederacion();
        bool encontrada = false;

        for (int j = 0; j < totalConfs; ++j) {
            if (confs[j] == conf) {
                conteo[j]++;
                encontrada = true;
                break;
            }
        }

        if (!encontrada) {
            confs[totalConfs] = conf;
            conteo[totalConfs] = 1;
            totalConfs++;
        }
    }

    int maxIdx = 0;
    for (int i = 1; i < totalConfs; ++i)
        if (conteo[i] > conteo[maxIdx])
            maxIdx = i;

    std::cout << "Confederacion con mas equipos: "
              << confs[maxIdx]
              << " (" << conteo[maxIdx] << " equipos)\n";
}

// Generación de enfrentamientos de R16
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
            bool encontrado = false;
            for (int j = 0; j < tCount; ++j) {
                if (!mismoGrupo(cabeza, terceros[j].equipo)) {
                    tercero = terceros[j].equipo;
                    Clasificado temp = terceros[terceroIdx];
                    terceros[terceroIdx] = terceros[j];
                    terceros[j] = temp;
                    encontrado = true;
                    break;
                }
            }
            if (!encontrado)
                std::cerr << "Advertencia: no se encontro tercero valido para "
                          << cabeza->getPais() << "\n";
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
            bool encontrado = false;
            for (int j = i+1; j < sCount; ++j) {
                if (!mismoGrupo(cabeza, peoresSegundos[j].equipo)) {
                    segundo = peoresSegundos[j].equipo;
                    Clasificado temp = peoresSegundos[i];
                    peoresSegundos[i] = peoresSegundos[j];
                    peoresSegundos[j] = temp;
                    encontrado = true;
                    break;
                }
            }
            if (!encontrado)
                std::cerr << "Advertencia: no se encontro segundo valido para "
                          << cabeza->getPais() << "\n";
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
            bool encontrado = false;
            for (int j = 4 + i + 2; j < sCount; ++j) {
                if (!mismoGrupo(seg1, peoresSegundos[j].equipo)) {
                    seg2 = peoresSegundos[j].equipo;
                    encontrado = true;
                    break;
                }
            }
            if (!encontrado)
                std::cerr << "Advertencia: no se encontro segundo valido para "
                          << seg1->getPais() << "\n";
        }
        enfrentamientos[eCount++] = seg1;
        enfrentamientos[eCount++] = seg2;
    }
}

// Simulación de una ronda eliminatoria genérica
void Torneo::simularRonda(Equipo** participantes, int numParticipantes,
                          const std::string& fecha, bool esEliminatoria,
                          Equipo** ganadores, int& numGanadores) {
    if (numParticipantes % 2 != 0) return;
    int numPartidos = numParticipantes / 2;
    Partido** partidos = new Partido*[numPartidos];
    for (int i = 0; i < numPartidos; ++i) {
        incIteraciones(1);
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

// Estadísticas finales (con manejo de punteros nulos)
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
        incIteraciones(1);
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
            incIteraciones(1);
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

// Simulación completa de eliminatorias (con tercer puesto)
void Torneo::simularEliminatorias(const std::string& fechaEliminatorias) {
    Clasificado primeros[12], segundos[12], terceros[12];
    int pCount, sCount, tCount;
    recolectarClasificados(primeros, pCount, segundos, sCount, terceros, tCount);
    ordenarTerceros(terceros, tCount);

    Equipo* ronda32[32];
    int numRonda32;
    generarEnfrentamientosR16(primeros, pCount, segundos, sCount, terceros, tCount, ronda32, numRonda32);

    // Imprimir las 12 tablas de clasificación
    std::cout << "\n=== TABLAS DE CLASIFICACION - FASE DE GRUPOS ===\n";
    for (int g = 0; g < NUM_GRUPOS; ++g)
        grupos[g]->imprimirTabla();

    // Imprimir emparejamientos R16 antes de simularlos
    std::cout << "\n=== PARTIDOS CONFIGURADOS PARA DIECISEISAVOS (sin simular) ===\n";
    for (int i = 0; i < numRonda32; i += 2)
        std::cout << "  " << ronda32[i]->getPais()
                  << " vs " << ronda32[i+1]->getPais() << "\n";

    std::cout << "\n=== DIECISEISAVOS DE FINAL ===\n";
    Equipo* ronda16[16];
    int numRonda16;
    simularRonda(ronda32, numRonda32, fechaEliminatorias, true, ronda16, numRonda16);
    countR16 = numRonda32;
    for (int i = 0; i < numRonda32; ++i)
        participantesR16[i] = ronda32[i];

    std::cout << "\n=== OCTAVOS DE FINAL ===\n";
    Equipo* ronda8[8];
    int numRonda8;
    simularRonda(ronda16, numRonda16, fechaEliminatorias, true, ronda8, numRonda8);
    countR8 = numRonda16;
    for (int i = 0; i < numRonda16; ++i)
        participantesR8[i] = ronda16[i];

    std::cout << "\n=== CUARTOS DE FINAL ===\n";
    Equipo* ronda4[4];
    int numRonda4;
    simularRonda(ronda8, numRonda8, fechaEliminatorias, true, ronda4, numRonda4);
    countR4 = numRonda8;
    for (int i = 0; i < numRonda8; ++i)
        participantesR4[i] = ronda8[i];

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
    Equipo* terceroLocal = (tercerPuesto->getGolesEq1() > tercerPuesto->getGolesEq2()) ?
                               tercerPuesto->getEquipo1() : tercerPuesto->getEquipo2();
    Equipo* cuartoLocal = (terceroLocal == tercerPuesto->getEquipo1()) ?
                              tercerPuesto->getEquipo2() : tercerPuesto->getEquipo1();
    delete tercerPuesto;

    // Final
    std::cout << "\n=== FINAL ===\n";
    Partido* final = new Partido(ganador1, ganador2, fechaEliminatorias, "00:00", "nombreSede");
    final->simular(true);
    final->actualizarHistoricos();
    final->imprimirResultado();
    final->imprimirGoleadores();
    Equipo* campeonLocal = (final->getGolesEq1() > final->getGolesEq2()) ?
                               final->getEquipo1() : final->getEquipo2();
    Equipo* subcampeonLocal = (campeonLocal == final->getEquipo1()) ?
                                  final->getEquipo2() : final->getEquipo1();
    delete final;
    campeon      = campeonLocal;
    subcampeon   = subcampeonLocal;
    terceroLugar = terceroLocal;
    cuartoLugar  = cuartoLocal;
    eliminatoriasSimuladas = true;

    std::cout << "\n=== CONFEDERACIONES POR ETAPA ===\n";

    std::cout << "R16:\n";
    contarConfederaciones(participantesR16, countR16);

    std::cout << "R8:\n";
    contarConfederaciones(participantesR8, countR8);

    std::cout << "R4:\n";
    contarConfederaciones(participantesR4, countR4);

    eliminatoriasSimuladas = true;
    mostrarMetricas("Eliminatorias completas",
                    sizeof(Clasificado)*12*3 + sizeof(Equipo*)*32 + sizeof(Equipo*)*16 +
                        sizeof(Equipo*)*8 + sizeof(Equipo*)*4,
                    "rand() de <cstdlib>, pow() de <cmath>");
}
    // Métricas de eficiencia
    long long Torneo::totalIteraciones = 0;

    void Torneo::incIteraciones(long long n) {
        totalIteraciones += n;
    }

    size_t Torneo::calcularMemoriaActual() {
        size_t total = 0;

        // Equipos: arreglo fijo de NUM_EQUIPOS
        // Cada Equipo tiene sus campos primitivos + puntero a Jugador[]
        total += NUM_EQUIPOS * sizeof(Equipo);
        total += NUM_EQUIPOS * JUGADORES_POR_EQUIPO * sizeof(Jugador);

        // Grupos: NUM_GRUPOS punteros, cada uno apunta a un Grupo en heap
        // Cada Grupo tiene arreglo de punteros a Equipo y puntero a Partido[]
        total += NUM_GRUPOS * sizeof(Grupo);
        total += NUM_GRUPOS * PARTIDOS_POR_GRUPO * sizeof(Partido);
        // Cada Partido tiene dos arreglos de 11 Convocados en heap
        total += NUM_GRUPOS * PARTIDOS_POR_GRUPO * 2 * 11 * sizeof(Convocado);

        // Bombos: arreglo 2D de punteros (no se duplica memoria, son alias)
        total += BOMBOS * EQUIPOS_POR_BOMBO * sizeof(Equipo*);

        // Arreglos de participantes por ronda
        total += 32 * sizeof(Equipo*);  // participantesR16
        total += 16 * sizeof(Equipo*);  // participantesR8
        total += 8  * sizeof(Equipo*);  // participantesR4

        // Variables de control del torneo
        total += sizeof(int) * 4; // numEquipos, countR16, countR8, countR4

        return total;
    }

    // Formato del archivo:
    void Torneo::guardarEstadisticasJugadores(const std::string& archivo) {
        std::ofstream file(archivo.c_str());
        file << "pais;numeroCamiseta;goles;partidosJugados;minutos;asistencias;amarillas;rojas;faltas\n";
        if (!file.is_open()) {
            std::cerr << "Error al abrir " << archivo << " para escritura\n";
            return;
        }
        for (int i = 0; i < numEquipos; ++i) {
            incIteraciones(1);
            for (int j = 0; j < JUGADORES_POR_EQUIPO; ++j) {
                incIteraciones(1);
                Jugador& jug = equipos[i].getJugador(j);
                file << equipos[i].getPais() << ";"
                     << jug.getNumero()      << ";"
                     << jug.getGoles()       << ";"
                     << jug.getPartidosJugados() << ";"
                     << jug.getMinutos()     << ";"
                     << jug.getAsistencias() << ";"
                     << jug.getAmarillas()   << ";"
                     << jug.getRojas()       << ";"
                     << jug.getFaltas()      << "\n";
            }
        }
        file.close();
        std::cout << "Estadisticas de jugadores guardadas en " << archivo << "\n";
        mostrarMetricas("Guardar estadisticas jugadores",
                        sizeof(std::string),
                        "ofstream de <fstream>");
    }

    void Torneo::cargarEstadisticasJugadores(const std::string& archivo) {
        std::ifstream file(archivo.c_str());
        std::string encabezado;
        std::getline(file, encabezado); // saltar línea de encabezado
        if (!file.is_open()) {
            std::cerr << "Error al abrir " << archivo << " para lectura\n";
            return;
        }
        std::string linea;
        while (std::getline(file, linea)) {
            incIteraciones(1);
            int pos = 0;
            std::string pais    = extraerCampo(linea, pos);
            int numero          = stringAInt(extraerCampo(linea, pos));
            int goles           = stringAInt(extraerCampo(linea, pos));
            int partidos        = stringAInt(extraerCampo(linea, pos));
            int minutos         = stringAInt(extraerCampo(linea, pos));
            int asistencias     = stringAInt(extraerCampo(linea, pos));
            int amarillas       = stringAInt(extraerCampo(linea, pos));
            int rojas           = stringAInt(extraerCampo(linea, pos));
            int faltas          = stringAInt(extraerCampo(linea, pos));

            // Buscar el equipo y jugador correspondiente
            for (int i = 0; i < numEquipos; ++i) {
                incIteraciones(1);
                if (equipos[i].getPais() == pais) {
                    for (int j = 0; j < JUGADORES_POR_EQUIPO; ++j) {
                        incIteraciones(1);
                        if (equipos[i].getJugador(j).getNumero() == numero) {
                            Jugador& jug = equipos[i].getJugador(j);
                            jug.setGoles(goles);
                            jug.setPartidosJugados(partidos);
                            jug.setMinutos(minutos);
                            jug.setAsistencias(asistencias);
                            jug.setAmarillas(amarillas);
                            jug.setRojas(rojas);
                            jug.setFaltas(faltas);
                            break;
                        }
                    }
                    break;
                }
            }
        }
        file.close();
        std::cout << "Estadisticas de jugadores cargadas desde " << archivo << "\n";
        mostrarMetricas("Cargar estadisticas jugadores",
                        sizeof(std::string),
                        "ifstream de <fstream>");
    }

    void Torneo::generarEstadisticasFinales() {
        if (!eliminatoriasSimuladas) {
            std::cout << "Debe simular las eliminatorias primero.\n";
            return;
        }
        calcularEstadisticasFinales(campeon, subcampeon, terceroLugar, cuartoLugar);
        mostrarMetricas("Generar estadisticas finales",
                        sizeof(Equipo*) * 3 +    // mejores[3]
                            sizeof(Jugador*) * 3,    // punteros a mejores goleadores
                        "");
    }

    void Torneo::mostrarMetricas(const std::string& etapa, size_t bytesLocales,
                                 const std::string& componentesExternos) {
        std::cout << "\n--- Métricas de eficiencia: " << etapa << " ---\n";
        std::cout << "Iteraciones acumuladas: " << totalIteraciones << "\n";
        size_t total = calcularMemoriaActual() + bytesLocales;
        std::cout << "Memoria estimada en uso: " << total
                  << " bytes (" << total / 1024 << " KB)\n";
        if (!componentesExternos.empty())
            std::cout << "Componentes externos: " << componentesExternos << "\n";
    }

