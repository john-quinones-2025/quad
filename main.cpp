#include <iostream>
#include <vector>
#include <string>
#include "QuadTree.h"

using namespace std;

int main() {



    Frontera espacioGlobal = {0.0, 0.0, 100.0, 100.0};
    QuadTree* qtree = new QuadTree(espacioGlobal, 2);

    

    Particle p1 = {1, -50.0,  50.0, 0, 0, 1.0}; 
    Particle p2 = {2,  50.0,  50.0, 0, 0, 1.0}; 
    Particle p3 = {3, -50.0, -50.0, 0, 0, 1.0}; 
    Particle p4 = {4,  50.0, -50.0, 0, 0, 1.0}; 

    
    
    qtree->insertar(p1);
    qtree->insertar(p2);
    qtree->insertar(p3);
    qtree->insertar(p4);

    for (int i = 5; i <= 9; i++) {
        
        double nueva_x = i * 2.0;       
        double nueva_y = 10.0 - i;      
        
        Particle p_nueva = {i, nueva_x, nueva_y, 0, 0, 1.0}; 
        
        qtree->insertar(p_nueva);
    }

    

    // rango
    Frontera rangoBusqueda = {15.0, 0.0, 8.0, 8.0};
    vector<Particle> encontradas;
    int comparacionesQuadTree = 0;

    qtree->consultarRango(rangoBusqueda, encontradas, comparacionesQuadTree);

    
    cout << "cantidad de particula encontradas: " << encontradas.size() << endl;
    cout << "IDs de las particulas encontradas: ";


    for (const auto& p : encontradas) {


        cout << p.id << " ";
    }

    cout << "\ncomparaciones requeridas: " << comparacionesQuadTree << endl;

    qtree->limpiar();



    delete qtree;

    return 0;
}