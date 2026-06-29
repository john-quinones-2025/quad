#ifndef SIMULADOR_H
#define SIMULADOR_H

#include "QuadTree.h"
#include <vector>
#include <string>
#include <cmath>
#include <random>
#include <chrono>

using namespace std;

class Simulador {

    private:

        vector<Particle> particulas;
        double anchoEspacio;
        double altoEspacio;
        double radioMaximo;             // acotaremos los radios, para encontrar vecinos con radios diferentes.

        QuadTree arbol;                 // arbol reutilizado entre frames con limpiar() (#7)

        // sumidero volatil: evita que -O2 elimine las llamadas a verificanColision
        // cuyo resultado se descarta, para que los tiempos medidos sean reales.
        volatile bool sumidero;


        bool verificanColision(const Particle& p1, const Particle& p2) const;

        // respuesta fisica al choque: colision elastica con masa proporcional al area (#3)
        void resolverColision(Particle& a, Particle& b);

        // limpia y reinserta todas las particulas en el arbol miembro (reuso, #7)
        void reconstruirArbol();

        // generador mt19937: si semilla==0 usa random_device (no determinista)
        mt19937 crearGenerador(unsigned int semilla) const;

    public:

        Simulador(double ancho, double alto, int capacidadQT);

        void generarUniforme(int cantidad, double radioMin, double radioMax, double velMax, unsigned int semilla = 0);
        void generarClusters(int cantidad, int numClusters, double radioMin, double radioMax, double velMax, unsigned int semilla = 0);
        void generarAltaDensidad(int cantidad, double radioMin, double radioMax, double velMax, unsigned int semilla = 0);

        void agregarParticula(const Particle& p);
        vector<Particle>& obtenerParticulas();


        // avanza posiciones + rebote en bordes
        void actualizarFisica();

        // un paso de simulacion: fisica + deteccion y respuesta de choques via quadtree (#3, #4)
        void simularPaso();

        // paso de simulacion determinista por fuerza bruta (orden i<j), para validacion cruzada (#18)
        void simularPasoFB();


        // benchmarks puros (cuentan comparaciones y miden tiempo, sin respuesta fisica)
        void reporteColisionesQuadTree(double& tiempoMilisegundos, int& comparacionesTotales, double& candidatosPromedio);
        void reporteColisionesFuerzaBruta(double& tiempoMilisegundos, int& comparacionesTotales);

        // corre la bateria de experimentos y vuelca un CSV (#4, #5)
        void ejecutarExperimentos();

        // corre M frames imprimiendo comparaciones FB vs QT por frame (#4)
        void ejecutarSimulacion(int n, int frames);

        // vuelca a JSON el estado inicial y el estado tras `frames` pasos deterministas (#18)
        void volcarEscenarioJSON(const string& archivo, int n, int frames, unsigned int semilla);

};

#endif
