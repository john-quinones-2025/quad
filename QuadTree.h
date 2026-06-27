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


    bool intersecta(const Frontera& rango)  {

        return !(rango.x - rango.ancho > x + ancho ||
                 rango.x + rango.ancho < x - ancho ||
                 rango.y - rango.alto > y + alto ||
                 rango.y + rango.alto < y - alto);
    }
};


#endif