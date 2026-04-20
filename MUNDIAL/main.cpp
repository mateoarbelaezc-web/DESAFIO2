#include "Torneo.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    srand(time(nullptr));
    Torneo mundial;
    mundial.cargarEquipos("selecciones_clasificadas_mundial.csv");
    mundial.conformarBombos();
    mundial.sortearGrupos();
    mundial.simularFaseGrupos("20/06/2026");
    mundial.simularEliminatorias("10/07/2026");
    return 0;
}
