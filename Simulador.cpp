#include "Simulador.h"


// constuimos el espacio del simulado, y la capcidad del quadtree 
Simulador::Simulador(double ancho, double alto, int capacidadQT) {

    anchoEspacio = ancho;
    altoEspacio = alto;
    capacidadQuadTree = capacidadQT;
}


// agregamos particulas al simulador
void Simulador::agregarParticula(Particle p) {
    particulas.push_back(p);
}

// obtenemos el vector particulas
vector<Particle>& Simulador::obtenerParticulas() {
    return particulas;
}



// para verificar si choco o no entre dos particulas de diferentes radios.
bool Simulador::verificanColision(Particle p1, Particle p2) {
    // Para no comparar una particula consigo misma
    if (p1.id == p2.id) return false;

    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    double distanciaCuadrada = (dx * dx) + (dy * dy);
    
    double sumaRadios = p1.radius + p2.radius;
    
    return distanciaCuadrada <= (sumaRadios * sumaRadios);
}


// actualizamos posiciones y rebotes en los bordes.
void Simulador::actualizarFisica() {


    for (int i = 0; i < particulas.size(); i++) {

        // movemos al particula
        particulas[i].x += particulas[i].vx;
        particulas[i].y += particulas[i].vy;

        // si choca a la izquierda, y si choca a la derecha
        if (particulas[i].x - particulas[i].radius < 0) {

            particulas[i].x = particulas[i].radius;
            particulas[i].vx *= -1;

        } else if (particulas[i].x + particulas[i].radius > anchoEspacio) {

            particulas[i].x = anchoEspacio - particulas[i].radius;
            particulas[i].vx *= -1;
        }

        // si choca abajo y si choca arriba
        if (particulas[i].y - particulas[i].radius < 0) {

            particulas[i].y = particulas[i].radius;
            particulas[i].vy *= -1;

        } else if (particulas[i].y + particulas[i].radius > altoEspacio) {
            particulas[i].y = altoEspacio - particulas[i].radius;
            particulas[i].vy *= -1;
        }
    }
}


// fuerza bruta, verificamos todos los choques entre pares de particulas.
int Simulador::detectarColisionesFuerzaBruta() {

    int comparaciones = 0;
    
    for (int i = 0; i < particulas.size(); i++) {

        for (int j = i + 1; j < particulas.size(); j++) {

            comparaciones++; //contador
            verificanColision(particulas[i], particulas[j]);
        }
    }


    return comparaciones;
}

// usamos quadtrees para la deteccion de choques
int Simulador::detectarColisionesQuadTree() {

    int comparaciones = 0;
    
    // definimos el limiteGlobal del quadtree
    Frontera limiteGlobal = {anchoEspacio / 2, altoEspacio / 2, anchoEspacio / 2, altoEspacio / 2};
    QuadTree qt(limiteGlobal, capacidadQuadTree);

    // insertamos todas las particulas en ese frame
    for (int i = 0; i < particulas.size(); i++) {
        qt.insertar(particulas[i]);
    }

    // consultamos los choques por cada particula
    for (int i = 0; i < particulas.size(); i++) {

        // creamos una frontera de busqueda para atrapar a los posibles candidatos a que hayan chocado
        // utilizamos una caja con el radio del doble por mientras, luego arreglamos para cuando
        // una particula chica se detecte el choque con una grande.
        double r = particulas[i].radius;
        Frontera areaConsulta = {particulas[i].x, particulas[i].y, r * 2, r * 2};

        vector<Particle> vecinosCandidatos;
        
        // el quadtree nos llena el vector y nos suma las comparaciones 
        qt.consultarRango(areaConsulta, vecinosCandidatos, comparaciones);


        // verificamos choque entres estos pocos candidatos
        for (int j = 0; j < vecinosCandidatos.size(); j++) {
            
            verificanColision(particulas[i], vecinosCandidatos[j]);
        }
    }

    return comparaciones;
}