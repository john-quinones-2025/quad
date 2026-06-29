#ifndef SIMULADOR_H
#define SIMULADOR_H

#include "QuadTree.h"
#include <vector>
#include <cmath>
#include <random>
#include <chrono>

using namespace std;

class Simulador {

    private:

        vector<Particle> particulas;
        double anchoEspacio;
        double altoEspacio;
        int capacidadQuadTree;
        double radioMaximo;             // acotaremos los radios, para encontrar vecinos con radios diferentes.

        
        bool verificanColision(Particle p1, Particle p2);

    public:

        Simulador(double ancho, double alto, int capacidadQT);

        void generarUniforme(int cantidad, double radioMin, double radioMax, double velMax);
        void generarClusters(int cantidad, int numClusters, double radioMin, double radioMax, double velMax);
        void generarAltaDensidad(int cantidad, double radioMin, double radioMax, double velMax);

        void agregarParticula(Particle p);

        vector<Particle>& obtenerParticulas();

        
        void actualizarFisica();

        
        int detectarColisionesFuerzaBruta();

        
        int detectarColisionesQuadTree();

        void reporteColisionesQuadTree(double& tiempoMilisegundos, int& comparacionesTotales, double& candidatosPromedio);
        void reporteColisionesFuerzaBruta(double& tiempoMilisegundos, int& comparacionesTotales);
        void ejecutarExperimentos();

};

#endif