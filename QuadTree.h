#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>

using namespace std;

struct Particle{

    int id;
    double x, y;
    double vx, vy;
    double radius;
};


struct Frontera{


    double x, y;
    double ancho;       // mitad del ancho
    double alto;        // mitad del alto


    bool contiene(double px, double py) const {

        return (px >= x - ancho && px <= x + ancho &&
                py >= y - alto && py <= y + alto);
    }


    bool intersecta(const Frontera& rango) const {

        return !(rango.x - rango.ancho > x + ancho ||
                 rango.x + rango.ancho < x - ancho ||
                 rango.y - rango.alto > y + alto ||
                 rango.y + rango.alto < y - alto);
    }

};


class QuadTree {


    private:


    //atributos de quadtree

        Frontera limite;            // los limites
        size_t capacidadMaxima;     // capacidad maxima por nodo
        vector<Particle> particulas;        // vector guarda partiuclas en el nodo
        bool dividido;              // bandera si fue dividido el nodo



        //punteros
        QuadTree* noroeste;
        QuadTree* noreste;
        QuadTree* suroeste;
        QuadTree* sureste;



        void subdividir();

    public:



        QuadTree(Frontera frontera, int capacidad);
        ~QuadTree();


        bool insertar(const Particle& p);

        // Query rectangular: primitivo canonico del quadtree (util para culling de viewport).
        void consultarRango(const Frontera& rango, vector<Particle>& encontrados, int& comparacionesQuadTree) const;

        // Query circular: filtra por distancia euclidiana. Es el correcto para colisiones por radio.
        void consultarRadio(double px, double py, double radioBuscado, vector<Particle>& encontrados, int& comparacionesQuadTree) const;

        void limpiar();
};


#endif
