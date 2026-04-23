#include "Torneo.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    srand(time(nullptr));
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
    Torneo mundial;
    int opcion = 0;

    while (opcion != 7) {
        std::cout << "\n===== MENU UdeAWorldCup =====\n";
        std::cout << "1. Cargar equipos\n";
        std::cout << "2. Conformar bombos\n";
        std::cout << "3. Sortear grupos\n";
        std::cout << "4. Simular fase de grupos\n";
        std::cout << "5. Simular eliminatorias\n";
        std::cout << "6. Generar estadisticas finales\n";
        std::cout << "7. Salir\n";
        std::cout << "Opcion: ";

        if (!(std::cin >> opcion)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "Entrada invalida, ingrese un numero del 1 al 7.\n";
            opcion = 0;
            continue;
        }

        switch (opcion) {
        case 1:
            mundial.cargarEquipos("selecciones_clasificadas_mundial.csv");
            break;
        case 2:
            if (!mundial.isEquiposCargados())
                std::cout << "Primero debe cargar los equipos (opcion 1).\n";
            else
                mundial.conformarBombos();
            break;
        case 3:
            if (!mundial.isBombosConfigurados())
                std::cout << "Primero debe conformar los bombos (opcion 2).\n";
            else
                mundial.sortearGrupos();
            break;
        case 4:
            if (!mundial.isGruposSorteados())
                std::cout << "Primero debe sortear los grupos (opcion 3).\n";
            else
                mundial.simularFaseGrupos("20/06/2026");
            break;
        case 5:
            if (!mundial.isFaseGruposSimulada())
                std::cout << "Primero debe simular la fase de grupos (opcion 4).\n";
            else
                mundial.simularEliminatorias("10/07/2026");
            break;
        case 6:
            if (!mundial.isEliminatoriasSimuladas())
                std::cout << "Primero debe simular las eliminatorias (opcion 5).\n";
            else
                mundial.generarEstadisticasFinales();
            break;
        case 7:
            std::cout << "Saliendo...\n";
            break;
        default:
            std::cout << "Opcion invalida, ingrese un numero del 1 al 7.\n";
        }
    }
    return 0;
}
