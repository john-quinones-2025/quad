#ifndef QUADTREE_H
#define QUADTREE_H

#include <iostream>
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
    double alto;        //mitad del alto


    bool contiene(double px, double py)  {

        return (px >= x - ancho && px <= x + ancho &&
                py >= y - alto && py <= y + alto);
    }


    bool intersecta( Frontera rango)  {

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
        int capacidadMaxima;        // capacida maxima por nodo
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

        
        bool insertar( Particle p);
        void consultarRango( Frontera rango, vector<Particle>& encontrados, int& comparacionesQuadTree);
        void consultarRadio(double px, double py, double radioBuscado, vector<Particle>& encontrados, int& comparacionesQuadTree);
        void limpiar();
};


#endif