#include <iostream>
#include "Simulador.h"

using namespace std;

int main() {

    // plano de 800x800
    double anchoEspacio = 800.0;
    double altoEspacio = 800.0;

    int capacidadQuadTree = 4;

    cout << "Dimensiones del espacio: " << anchoEspacio << " x " << altoEspacio << endl;
    cout << "Capacidad maxima por nodo: " << capacidadQuadTree << "\n" << endl;

    Simulador miSimulador(anchoEspacio, altoEspacio, capacidadQuadTree);

    // 1) Benchmark FB vs QT a distintos N y distribuciones -> consola + benchmark.csv (#4, #5)
    miSimulador.ejecutarExperimentos();

    // 2) Simulacion dinamica: ejercita actualizarFisica y compara FB vs QT por frame (#4)
    miSimulador.ejecutarSimulacion(2000, 10);

    // 3) Escenario determinista para validacion cruzada con el frontend JS (#18)
    miSimulador.volcarEscenarioJSON("web/expected.json", 200, 60, 12345);

    return 0;
}
