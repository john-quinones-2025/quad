#include "Simulador.h"


// constuimos el espacio del simulado, y la capcidad del quadtree 
Simulador::Simulador(double ancho, double alto, int capacidadQT) {

    anchoEspacio = ancho;
    altoEspacio = alto;
    capacidadQuadTree = capacidadQT;
    radioMaximo = 0.0;
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

// generador de particula uniformemente
void Simulador::generarUniforme(int cantidad, double radioMin, double radioMax, double velMax) {

    particulas.clear();
    radioMaximo = radioMax; // guardamos el radio maximo

    random_device rd;
    mt19937 gen(rd());

    uniform_real_distribution<> disX(radioMax, anchoEspacio - radioMax);
    uniform_real_distribution<> disY(radioMax, altoEspacio - radioMax);
    uniform_real_distribution<> disR(radioMin, radioMax);
    uniform_real_distribution<> disV(-velMax, velMax);

    for (int i = 0; i < cantidad; i++) {

        Particle p = {i, disX(gen), disY(gen), disV(gen), disV(gen), disR(gen)};
        particulas.push_back(p);
    }
}



void Simulador::generarClusters(int cantidad, int numClusters, double radioMin, double radioMax, double velMax) {

    particulas.clear();
    radioMaximo = radioMax;

    random_device rd;
    mt19937 gen(rd());

    uniform_real_distribution<> disCentroX(anchoEspacio * 0.1, anchoEspacio * 0.9);
    uniform_real_distribution<> disCentroY(altoEspacio * 0.1, altoEspacio * 0.9);
    uniform_real_distribution<> disR(radioMin, radioMax);
    uniform_real_distribution<> disV(-velMax, velMax);

    // creamos centros aleatorios para los clusters
    vector<pair<double, double>> centros;
    for (int i = 0; i < numClusters; i++) {
        centros.push_back({disCentroX(gen), disCentroY(gen)});
    }

    // distrbucion normal alrededor de un cluster
    normal_distribution<> disDispersion(0, anchoEspacio * 0.05);

    for (int i = 0; i < cantidad; i++) {

        int clusterAsignado = i % numClusters; // repartimos equitativamente
        double cx = centros[clusterAsignado].first;
        double cy = centros[clusterAsignado].second;

        double px = cx + disDispersion(gen);
        double py = cy + disDispersion(gen);

        // aseguramos que no se salgan del mapa
        px = max(radioMax, min(px, anchoEspacio - radioMax));
        py = max(radioMax, min(py, altoEspacio - radioMax));

        Particle p = {i, px, py, disV(gen), disV(gen), disR(gen)};
        particulas.push_back(p);

    }
}


void Simulador::generarAltaDensidad(int cantidad, double radioMin, double radioMax, double velMax) {


    particulas.clear();
    radioMaximo = radioMax;

    random_device rd;
    mt19937 gen(rd());
    
    // pondremos el 90 porciento de particulas concentradas en el centro
    normal_distribution<> disCentroX(anchoEspacio / 2, anchoEspacio * 0.1);
    normal_distribution<> disCentroY(altoEspacio / 2, altoEspacio * 0.1);
    
    // pondremos el 10 porciento partiulas esparcidas
    uniform_real_distribution<> disBajaX(radioMax, anchoEspacio - radioMax);
    uniform_real_distribution<> disBajaY(radioMax, altoEspacio - radioMax);
    
    uniform_real_distribution<> disR(radioMin, radioMax);
    uniform_real_distribution<> disV(-velMax, velMax);

    for (int i = 0; i < cantidad; i++) {

        double px, py;

        // 90% apretadas
        if (i < cantidad * 0.90) { 

            px = disCentroX(gen);
            py = disCentroY(gen);

        } else { 

            // 10% sueltas
            px = disBajaX(gen);
            py = disBajaY(gen);
        }

        px = max(radioMax, min(px, anchoEspacio - radioMax));
        py = max(radioMax, min(py, altoEspacio - radioMax));

        Particle p = {i, px, py, disV(gen), disV(gen), disR(gen)};
        particulas.push_back(p);

    }
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
        double rangoBusqueda = particulas[i].radius+ radioMaximo;
        Frontera areaConsulta = {particulas[i].x, particulas[i].y, rangoBusqueda , rangoBusqueda};

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



void Simulador::reporteColisionesQuadTree(double& tiempoMilisegundos, int& comparacionesTotales, double& candidatosPromedio) {

    comparacionesTotales = 0;
    int totalCandidatosRevisados = 0; // para el promedio que pide el profe
    
    auto inicio = chrono::high_resolution_clock::now();

    Frontera limiteGlobal = {anchoEspacio / 2, altoEspacio / 2, anchoEspacio / 2, altoEspacio / 2};
    QuadTree qt(limiteGlobal, capacidadQuadTree);


    for (int i = 0; i < particulas.size(); i++) {

        qt.insertar(particulas[i]);
    }

    for (int i = 0; i < particulas.size(); i++) {

        double rangoBusqueda = particulas[i].radius + radioMaximo;
        Frontera areaConsulta = {particulas[i].x, particulas[i].y, rangoBusqueda, rangoBusqueda};

        vector<Particle> vecinosCandidatos;
        qt.consultarRango(areaConsulta, vecinosCandidatos, comparacionesTotales);
        
        totalCandidatosRevisados += vecinosCandidatos.size();

        for (int j = 0; j < vecinosCandidatos.size(); j++) {

            verificanColision(particulas[i], vecinosCandidatos[j]);
        }
    }

    auto fin = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duracion = fin - inicio;
    tiempoMilisegundos = duracion.count();
    
    // calculo del promedio de candidatos por objeto
    if (particulas.size() > 0) {
        candidatosPromedio = (double)totalCandidatosRevisados / particulas.size();
    } else {
        candidatosPromedio = 0;
    }
}

// reporte fuerza bruta
void Simulador::reporteColisionesFuerzaBruta(double& tiempoMilisegundos, int& comparacionesTotales) {

    comparacionesTotales = 0;
    
    auto inicio = chrono::high_resolution_clock::now();

    for (int i = 0; i < particulas.size(); i++) {

        for (int j = i + 1; j < particulas.size(); j++) {

            comparacionesTotales++; 
            verificanColision(particulas[i], particulas[j]);
        }
    }

    auto fin = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duracion = fin - inicio;
    tiempoMilisegundos = duracion.count();

}



void Simulador::ejecutarExperimentos() {
    vector<int> tamanos = {1000, 5000, 10000}; 
    
    cout << "EXPERIMENTOS:\n";
 

    for (int n : tamanos) {
        cout << "............EVALUANDO " << n << " OBJETOS ..........\n";
        
        double tiempoFB, tiempoQT, candPromedio;
        int compFB, compQT;

        // uniforme
        generarUniforme(n, 2.0, 5.0, 1.0);
        reporteColisionesFuerzaBruta(tiempoFB, compFB);
        reporteColisionesQuadTree(tiempoQT, compQT, candPromedio);

        cout << "Distribucion Uniforme:\n";
        cout << "  - Fuerza Bruta : " << tiempoFB << " ms | Comparaciones: " << compFB << "\n";
        cout << "  - QuadTree     : " << tiempoQT << " ms | Comparaciones: " << compQT << "\n";
        cout << "  - Promedio de candidatos revisados por objeto: " << candPromedio << "\n\n";
        

        // clusters
        int numeroDeClusters = 5; // grupos de 5
        generarClusters(n, numeroDeClusters, 2.0, 5.0, 1.0);
        reporteColisionesFuerzaBruta(tiempoFB, compFB);
        reporteColisionesQuadTree(tiempoQT, compQT, candPromedio);

        cout << "Distribucion Clusters " << numeroDeClusters << " grupos:\n";
        cout << "  - Fuerza Bruta : " << tiempoFB << " ms | Comparaciones: " << compFB << "\n";
        cout << "  - QuadTree     : " << tiempoQT << " ms | Comparaciones: " << compQT << "\n";
        cout << "  - Promedio de candidatos revisados por objeto: " << candPromedio << "\n\n";


        // alta densidad
        generarAltaDensidad(n, 2.0, 5.0, 1.0);
        reporteColisionesFuerzaBruta(tiempoFB, compFB);
        reporteColisionesQuadTree(tiempoQT, compQT, candPromedio);

        cout << "Distribucion Alta Densidad:\n";
        cout << "  - Fuerza Bruta : " << tiempoFB << " ms | Comparaciones: " << compFB << "\n";
        cout << "  - QuadTree     : " << tiempoQT << " ms | Comparaciones: " << compQT << "\n";
        cout << "  - Promedio de candidatos revisados por objeto: " << candPromedio << "\n\n";



    }




}





