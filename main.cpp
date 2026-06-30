#include <iostream>
#include <string>
#include "Simulador.h"

using namespace std;


// Test de correctness de las dos queries del QuadTree (rango rectangular y radio circular).
// Inserta una particula en cada esquina de un mapa 200x200 y verifica que cada query
// recupere exactamente la esperada.
void validarConsultasEspaciales() {

    string linea(80, '=');
    cout << linea << "\n";
    cout << "    VALIDANDO METODOS DE CONSULTA ESPACIAL\n";
    cout << linea << "\n";
    cout << "Particulas insertadas en las 4 esquinas de un mapa 200x200.\n\n";

    // mapa 200x200: centro (100,100), semi-dimensiones 100. Convencion y hacia arriba.
    Frontera limite = {100, 100, 100, 100};
    QuadTree qt(limite, 4);

    Particle p1 = {1,  10, 190, 0, 0, 3};   // superior izquierda
    Particle p2 = {2, 190, 190, 0, 0, 3};   // superior derecha
    Particle p3 = {3,  10,  10, 0, 0, 3};   // inferior izquierda
    Particle p4 = {4, 190,  10, 0, 0, 3};   // inferior derecha
    qt.insertar(p1);
    qt.insertar(p2);
    qt.insertar(p3);
    qt.insertar(p4);

    int comparaciones = 0;

    // 1) Rango rectangular: caja chica alrededor de la esquina superior izquierda.
    Frontera caja = {10, 190, 20, 20};
    vector<Particle> r1;
    qt.consultarRango(caja, r1, comparaciones);

    cout << "1. Prueba de Rango Rectangular (Buscando en la esquina superior izquierda):\n";
    cout << "   - Esperado: 1 particula (ID 1)\n";
    cout << "   - Encontrado: " << r1.size() << " particula(s)\n";
    if (!r1.empty()) cout << "   - ID de la particula encontrada: " << r1[0].id << "\n";
    cout << "\n";

    // 2) Radio circular: circulo cerca de la esquina inferior derecha.
    comparaciones = 0;
    vector<Particle> r2;
    qt.consultarRadio(190, 10, 30, r2, comparaciones);

    cout << "2. Prueba de Radio Circular (Buscando cerca de la esquina inferior derecha):\n";
    cout << "   - Esperado: 1 particula (ID 4)\n";
    cout << "   - Encontrado: " << r2.size() << " particula(s)\n";
    if (!r2.empty()) cout << "   - ID de la particula encontrada: " << r2[0].id << "\n";
    cout << linea << "\n\n";
}


int main() {

    // plano de 800x800
    double anchoEspacio = 800.0;
    double altoEspacio = 800.0;

    int capacidadQuadTree = 4;

    cout << "Dimensiones del espacio: " << anchoEspacio << " x " << altoEspacio << endl;
    cout << "Capacidad maxima por nodo: " << capacidadQuadTree << "\n" << endl;

    // 0) Validacion de correctness de las queries del QuadTree
    validarConsultasEspaciales();

    Simulador miSimulador(anchoEspacio, altoEspacio, capacidadQuadTree);

    // 1) Benchmark FB vs QT a distintos N y distribuciones -> consola + benchmark.csv (#4, #5)
    miSimulador.ejecutarExperimentos();

    // 2) Simulacion dinamica: ejercita actualizarFisica y compara FB vs QT por frame (#4)
    miSimulador.ejecutarSimulacion(2000, 10);

    // 3) Escenario determinista para validacion cruzada con el frontend JS (#18)
    miSimulador.volcarEscenarioJSON("web/expected.json", 200, 60, 12345);

    return 0;
}
