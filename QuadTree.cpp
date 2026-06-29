#include "QuadTree.h"



// constructor
QuadTree::QuadTree(Frontera frontera, int capacidad) {

    limite = frontera;
    capacidadMaxima = (size_t)capacidad;
    dividido = false;

    noroeste = nullptr;
    noreste = nullptr;
    suroeste = nullptr;
    sureste = nullptr;

}

// destructor
QuadTree::~QuadTree() {


    if (dividido) {

        delete noroeste;
        delete noreste;
        delete suroeste;
        delete sureste;
    }

}



// subdivide el nodo en 4 regiones
void QuadTree::subdividir() {


    // calculamos la dimension de los hijos
    double x = limite.x;
    double y = limite.y;
    double w = limite.ancho;
    double h = limite.alto;

    Frontera f_no = {x - w/2, y + h/2, w/2, h/2};
    Frontera f_ne = {x + w/2, y + h/2, w/2, h/2};
    Frontera f_so = {x - w/2, y - h/2, w/2, h/2};
    Frontera f_se = {x + w/2, y - h/2, w/2, h/2};


    // creamos los quadTrees
    noroeste = new QuadTree(f_no, (int)capacidadMaxima);
    noreste  = new QuadTree(f_ne, (int)capacidadMaxima);
    suroeste = new QuadTree(f_so, (int)capacidadMaxima);
    sureste  = new QuadTree(f_se, (int)capacidadMaxima);


    // prendemos la bandera
    dividido = true;

    // hacemos que las particulas del padre vengan a los nodos hijos segun su posicion
    for (const auto& p : particulas) {

        if (noroeste->insertar(p)) continue;
        if (noreste->insertar(p)) continue;
        if (suroeste->insertar(p)) continue;
        if (sureste->insertar(p)) continue;

    }

    particulas.clear();
}

// insertamos particulas, si se rebalsa, redistribuimos las partiuclas del nodo padre en los nodos hijos, por ultimo
// la particula que rebalsa, tambien lo ponemos en uno del los nodos hijos.
bool QuadTree::insertar(const Particle& p) {


    // no esta en el limite retornamos false
    if (!limite.contiene(p.x, p.y)) {

        return false;
    }

    // si el nodo no fue dividido y todavia tiene capacidad pusheamos, de lo contrario estamos en el limite y subdividimos.
    if (!dividido) {

        if (particulas.size() < capacidadMaxima) {

            particulas.push_back(p);
            return true;

        } else {
            subdividir();
        }
    }


    // si llegamos aqui entonces rebalso y tambien reubicamos la particula en el nodo hijo
    if (noroeste->insertar(p)) return true;
    if (noreste->insertar(p)) return true;
    if (suroeste->insertar(p)) return true;
    if (sureste->insertar(p)) return true;


    // si ocurre un inconcordancia matematica retornamos false, como si no se hubiera insertado
    return false;
}



// consultamos los nodos que estan en el rango rectangular, guardamos los encontrados y las comparaciones.
void QuadTree::consultarRango(const Frontera& rango, vector<Particle>& encontrados, int& comparacionesQuadTree) const {

    // si no esta en el rango , no buscamos
    if (!limite.intersecta(rango)) {
        return;
    }

    // si es hoja entonces buscamso ahi. todos las particulas estan en la hojas.
    if (!dividido) {

        for (const auto& p : particulas) {

            // anotamos las comparaciones
            comparacionesQuadTree++;

            // verificamos si se encuentra en el rango y pusheamos
            if (rango.contiene(p.x, p.y)) {

                encontrados.push_back(p);
            }
        }
    }
    // si llegamos aqui entonces no es hoja  y seguimos bajando por los nodos hijos y cuando llegemos a hoja se consulta
    else {


        noroeste->consultarRango(rango, encontrados, comparacionesQuadTree);
        noreste->consultarRango(rango, encontrados, comparacionesQuadTree);
        suroeste->consultarRango(rango, encontrados, comparacionesQuadTree);
        sureste->consultarRango(rango, encontrados, comparacionesQuadTree);
    }
}


// consultamos las particulas dentro de un circulo (px, py, radioBuscado).
// Filtra por distancia euclidiana, evitando los falsos positivos de las esquinas de una caja.
void QuadTree::consultarRadio(double px, double py, double radioBuscado, vector<Particle>& encontrados, int& comparacionesQuadTree) const {

    // creamos una caja que cubra el circulo buscado para el test de interseccion con el cuadrante
    Frontera rango = {px, py, radioBuscado, radioBuscado};

    // si la caja no intersecta con este cuadrante, lo descartamos
    if (!limite.intersecta(rango)) {

        return;
    }

    if (!dividido) {

        for (const auto& p : particulas) {

            comparacionesQuadTree++;

            // verificamos si la particula esta dentro del circulo
            double dx = p.x - px;
            double dy = p.y - py;
            double distanciaCuadrada = (dx * dx) + (dy * dy);

            if (distanciaCuadrada <= (radioBuscado * radioBuscado)) {

                encontrados.push_back(p);

            }
        }
    }
    else {

        noroeste->consultarRadio(px, py, radioBuscado, encontrados, comparacionesQuadTree);
        noreste->consultarRadio(px, py, radioBuscado, encontrados, comparacionesQuadTree);
        suroeste->consultarRadio(px, py, radioBuscado, encontrados, comparacionesQuadTree);
        sureste->consultarRadio(px, py, radioBuscado, encontrados, comparacionesQuadTree);
    }
}


// limpiamos todo el quadtree dejando el root reutilizable (sin reasignar memoria del root).
void QuadTree::limpiar() {


    particulas.clear();

    if (dividido) {

        noroeste->limpiar();
        noreste->limpiar();
        suroeste->limpiar();
        sureste->limpiar();

        delete noroeste; noroeste = nullptr;
        delete noreste; noreste = nullptr;
        delete suroeste; suroeste = nullptr;
        delete sureste; sureste = nullptr;

        dividido = false;
    }
}
