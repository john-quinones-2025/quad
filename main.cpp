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




    miSimulador.ejecutarExperimentos();


    return 0;
}