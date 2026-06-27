#ifndef SIMULADOR_H
#define SIMULADOR_H

#include "QuadTree.h"
#include <vector>
#include <cmath>

using namespace std;

class Simulador {

    private:

        vector<Particle> particulas;
        double anchoEspacio;
        double altoEspacio;
        int capacidadQuadTree;

        
        bool verificanColision(Particle p1, Particle p2);

    public:

        Simulador(double ancho, double alto, int capacidadQT);

        void agregarParticula(Particle p);

        vector<Particle>& obtenerParticulas();

        
        void actualizarFisica();

        
        int detectarColisionesFuerzaBruta();

        
        int detectarColisionesQuadTree();
};

#endif