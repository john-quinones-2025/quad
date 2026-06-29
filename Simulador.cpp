#include "Simulador.h"
#include <iostream>
#include <fstream>
#include <algorithm>


// constuimos el espacio del simulado, y la capcidad del quadtree.
// el arbol miembro se construye una sola vez con el limite global y se reutiliza cada frame.
Simulador::Simulador(double ancho, double alto, int capacidadQT)
    : anchoEspacio(ancho),
      altoEspacio(alto),
      radioMaximo(0.0),
      arbol(Frontera{ancho / 2, alto / 2, ancho / 2, alto / 2}, capacidadQT),
      sumidero(false) {
}


// generador mt19937: semilla==0 -> no determinista; semilla fija -> reproducible (para validacion)
mt19937 Simulador::crearGenerador(unsigned int semilla) const {
    if (semilla == 0) {
        random_device rd;
        return mt19937(rd());
    }
    return mt19937(semilla);
}


// agregamos particulas al simulador
void Simulador::agregarParticula(const Particle& p) {
    particulas.push_back(p);
}

// obtenemos el vector particulas
vector<Particle>& Simulador::obtenerParticulas() {
    return particulas;
}



// para verificar si choco o no entre dos particulas de diferentes radios.
bool Simulador::verificanColision(const Particle& p1, const Particle& p2) const {
    // Para no comparar una particula consigo misma
    if (p1.id == p2.id) return false;

    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    double distanciaCuadrada = (dx * dx) + (dy * dy);

    double sumaRadios = p1.radius + p2.radius;

    return distanciaCuadrada <= (sumaRadios * sumaRadios);
}


// respuesta fisica: colision elastica entre dos circulos con masa proporcional al area (r^2).
void Simulador::resolverColision(Particle& a, Particle& b) {

    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double distanciaCuadrada = (dx * dx) + (dy * dy);
    double sumaRadios = a.radius + b.radius;

    // no se tocan, o estan exactamente encima (evitamos division por cero)
    if (distanciaCuadrada > sumaRadios * sumaRadios) return;
    if (distanciaCuadrada == 0.0) return;

    double distancia = sqrt(distanciaCuadrada);

    // normal del contacto (de a hacia b)
    double nx = dx / distancia;
    double ny = dy / distancia;

    // velocidad relativa proyectada sobre la normal
    double dvx = a.vx - b.vx;
    double dvy = a.vy - b.vy;
    double velNormal = dvx * nx + dvy * ny;

    // si ya se estan separando no aplicamos impulso
    if (velNormal <= 0) return;

    double masaA = a.radius * a.radius;
    double masaB = b.radius * b.radius;

    // impulso elastico 1D sobre la normal, repartido por masa
    double impulso = (2.0 * velNormal) / (masaA + masaB);

    a.vx -= impulso * masaB * nx;
    a.vy -= impulso * masaB * ny;
    b.vx += impulso * masaA * nx;
    b.vy += impulso * masaA * ny;

    // separamos el solapamiento para que no queden pegadas
    double solapamiento = 0.5 * (sumaRadios - distancia);
    a.x -= solapamiento * nx;
    a.y -= solapamiento * ny;
    b.x += solapamiento * nx;
    b.y += solapamiento * ny;
}


// limpia el arbol miembro y reinserta todas las particulas del frame actual (#7)
void Simulador::reconstruirArbol() {
    arbol.limpiar();
    for (const auto& p : particulas) {
        arbol.insertar(p);
    }
}


// generador uniforme
void Simulador::generarUniforme(int cantidad, double radioMin, double radioMax, double velMax, unsigned int semilla) {

    particulas.clear();
    radioMaximo = radioMax; // guardamos el radio maximo

    mt19937 gen = crearGenerador(semilla);

    uniform_real_distribution<> disX(radioMax, anchoEspacio - radioMax);
    uniform_real_distribution<> disY(radioMax, altoEspacio - radioMax);
    uniform_real_distribution<> disR(radioMin, radioMax);
    uniform_real_distribution<> disV(-velMax, velMax);

    for (int i = 0; i < cantidad; i++) {

        Particle p = {i, disX(gen), disY(gen), disV(gen), disV(gen), disR(gen)};
        particulas.push_back(p);
    }
}



void Simulador::generarClusters(int cantidad, int numClusters, double radioMin, double radioMax, double velMax, unsigned int semilla) {

    particulas.clear();
    radioMaximo = radioMax;

    mt19937 gen = crearGenerador(semilla);

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


void Simulador::generarAltaDensidad(int cantidad, double radioMin, double radioMax, double velMax, unsigned int semilla) {


    particulas.clear();
    radioMaximo = radioMax;

    mt19937 gen = crearGenerador(semilla);

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


    for (size_t i = 0; i < particulas.size(); i++) {

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


// un paso completo de simulacion usando el quadtree: fisica + deteccion + respuesta (#3, #4)
void Simulador::simularPaso() {

    actualizarFisica();

    reconstruirArbol();

    int comparaciones = 0; // descartado aqui; el conteo formal lo hacen los reportes

    for (size_t i = 0; i < particulas.size(); i++) {

        double rangoBusqueda = particulas[i].radius + radioMaximo;

        vector<Particle> vecinosCandidatos;
        arbol.consultarRadio(particulas[i].x, particulas[i].y, rangoBusqueda, vecinosCandidatos, comparaciones);

        for (const auto& c : vecinosCandidatos) {

            // resolvemos cada par una sola vez (id == indice en el vector)
            if (c.id > (int)i) {
                resolverColision(particulas[i], particulas[c.id]);
            }
        }
    }
}


// paso determinista por fuerza bruta (orden i<j), usado para la validacion cruzada con JS (#18)
void Simulador::simularPasoFB() {

    actualizarFisica();

    for (size_t i = 0; i < particulas.size(); i++) {
        for (size_t j = i + 1; j < particulas.size(); j++) {
            resolverColision(particulas[i], particulas[j]);
        }
    }
}


// usamos el quadtree para la deteccion de choques. benchmark puro: cuenta y mide tiempo.
void Simulador::reporteColisionesQuadTree(double& tiempoMilisegundos, int& comparacionesTotales, double& candidatosPromedio) {

    comparacionesTotales = 0;
    int totalCandidatosRevisados = 0; // para el promedio que pide el profe

    auto inicio = chrono::high_resolution_clock::now();

    // construir el arbol forma parte del costo del metodo quadtree
    reconstruirArbol();

    for (size_t i = 0; i < particulas.size(); i++) {

        double rangoBusqueda = particulas[i].radius + radioMaximo;

        vector<Particle> vecinosCandidatos;
        arbol.consultarRadio(particulas[i].x, particulas[i].y, rangoBusqueda, vecinosCandidatos, comparacionesTotales);

        totalCandidatosRevisados += (int)vecinosCandidatos.size();

        for (const auto& c : vecinosCandidatos) {
            sumidero ^= verificanColision(particulas[i], c);
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

// reporte fuerza bruta. benchmark puro: cuenta y mide tiempo.
void Simulador::reporteColisionesFuerzaBruta(double& tiempoMilisegundos, int& comparacionesTotales) {

    comparacionesTotales = 0;

    auto inicio = chrono::high_resolution_clock::now();

    for (size_t i = 0; i < particulas.size(); i++) {

        for (size_t j = i + 1; j < particulas.size(); j++) {

            comparacionesTotales++;
            sumidero ^= verificanColision(particulas[i], particulas[j]);
        }
    }

    auto fin = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duracion = fin - inicio;
    tiempoMilisegundos = duracion.count();

}



void Simulador::ejecutarExperimentos() {
    vector<int> tamanos = {1000, 5000, 10000};

    // CSV para graficar (#5)
    ofstream csv("benchmark.csv");
    csv << "N,distribucion,comp_FB,comp_QT,tiempo_FB_ms,tiempo_QT_ms,cand_promedio,razon_comp\n";

    cout << "EXPERIMENTOS:\n";


    for (int n : tamanos) {
        cout << "............EVALUANDO " << n << " OBJETOS ..........\n";

        double tiempoFB, tiempoQT, candPromedio;
        int compFB, compQT;

        // helper local para reportar a consola y CSV
        auto reportar = [&](const string& nombre) {
            reporteColisionesFuerzaBruta(tiempoFB, compFB);
            reporteColisionesQuadTree(tiempoQT, compQT, candPromedio);

            double razon = (compQT > 0) ? (double)compFB / compQT : 0.0;

            cout << nombre << ":\n";
            cout << "  - Fuerza Bruta : " << tiempoFB << " ms | Comparaciones: " << compFB << "\n";
            cout << "  - QuadTree     : " << tiempoQT << " ms | Comparaciones: " << compQT << "\n";
            cout << "  - Promedio de candidatos revisados por objeto: " << candPromedio << "\n";
            cout << "  - Razon FB/QT (comparaciones): " << razon << "\n\n";

            csv << n << "," << nombre << "," << compFB << "," << compQT << ","
                << tiempoFB << "," << tiempoQT << "," << candPromedio << "," << razon << "\n";
        };

        generarUniforme(n, 2.0, 5.0, 1.0);
        reportar("Uniforme");

        int numeroDeClusters = 5; // grupos de 5
        generarClusters(n, numeroDeClusters, 2.0, 5.0, 1.0);
        reportar("Clusters_5");

        generarAltaDensidad(n, 2.0, 5.0, 1.0);
        reportar("AltaDensidad");
    }

    csv.close();
    cout << "CSV escrito en benchmark.csv\n\n";
}


// corre M frames imprimiendo comparaciones FB vs QT por frame, ejercitando la fisica (#4)
void Simulador::ejecutarSimulacion(int n, int frames) {

    generarUniforme(n, 2.0, 5.0, 2.0);

    cout << "SIMULACION DINAMICA (" << n << " objetos, " << frames << " frames):\n";
    cout << "frame,comp_FB,comp_QT,razon\n";

    for (int f = 0; f < frames; f++) {

        double tiempoFB, tiempoQT, candPromedio;
        int compFB, compQT;

        reporteColisionesFuerzaBruta(tiempoFB, compFB);
        reporteColisionesQuadTree(tiempoQT, compQT, candPromedio);

        double razon = (compQT > 0) ? (double)compFB / compQT : 0.0;
        cout << f << "," << compFB << "," << compQT << "," << razon << "\n";

        // avanzamos un paso con respuesta fisica de choques
        simularPaso();
    }
    cout << "\n";
}


// vuelca a JSON el estado inicial y el final tras `frames` pasos deterministas (#18)
void Simulador::volcarEscenarioJSON(const string& archivo, int n, int frames, unsigned int semilla) {

    generarUniforme(n, 2.0, 5.0, 2.0, semilla);

    ofstream out(archivo);

    auto volcarParticulas = [&](const vector<Particle>& ps) {
        out << "[";
        for (size_t i = 0; i < ps.size(); i++) {
            const Particle& p = ps[i];
            out << "{\"id\":" << p.id
                << ",\"x\":" << p.x << ",\"y\":" << p.y
                << ",\"vx\":" << p.vx << ",\"vy\":" << p.vy
                << ",\"radius\":" << p.radius << "}";
            if (i + 1 < ps.size()) out << ",";
        }
        out << "]";
    };

    out.precision(17);
    out << "{\n";
    out << "  \"ancho\": " << anchoEspacio << ",\n";
    out << "  \"alto\": " << altoEspacio << ",\n";
    out << "  \"frames\": " << frames << ",\n";
    out << "  \"semilla\": " << semilla << ",\n";

    out << "  \"inicial\": ";
    volcarParticulas(particulas);
    out << ",\n";

    // avanzamos de forma determinista (fuerza bruta, orden i<j) para que JS reproduzca exacto
    for (int f = 0; f < frames; f++) {
        simularPasoFB();
    }

    out << "  \"esperado\": ";
    volcarParticulas(particulas);
    out << "\n}\n";

    out.close();
    cout << "Escenario de validacion escrito en " << archivo << "\n";
}
