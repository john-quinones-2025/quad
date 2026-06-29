// Espejo de Simulador.cpp. Mantiene id == indice en this.particulas.
class Simulador {
    constructor(ancho, alto, capacidadQT) {
        this.anchoEspacio = ancho;
        this.altoEspacio = alto;
        this.capacidadQuadTree = capacidadQT;
        this.radioMaximo = 0;
        this.particulas = [];

        this.arbol = new QuadTree(
            new Frontera(ancho / 2, alto / 2, ancho / 2, alto / 2),
            capacidadQT
        );

        // metricas del ultimo frame (para el HUD)
        this.comparacionesQT = 0;
    }

    setCapacidad(capacidadQT) {
        this.capacidadQuadTree = capacidadQT;
        this.arbol = new QuadTree(
            new Frontera(this.anchoEspacio / 2, this.altoEspacio / 2, this.anchoEspacio / 2, this.altoEspacio / 2),
            capacidadQT
        );
    }

    verificanColision(p1, p2) {
        if (p1.id === p2.id) return false;
        const dx = p1.x - p2.x;
        const dy = p1.y - p2.y;
        const distanciaCuadrada = dx * dx + dy * dy;
        const sumaRadios = p1.radius + p2.radius;
        return distanciaCuadrada <= sumaRadios * sumaRadios;
    }

    // colision elastica con masa proporcional al area (espejo exacto de C++)
    resolverColision(a, b) {
        const dx = b.x - a.x;
        const dy = b.y - a.y;
        const distanciaCuadrada = dx * dx + dy * dy;
        const sumaRadios = a.radius + b.radius;

        if (distanciaCuadrada > sumaRadios * sumaRadios) return;
        if (distanciaCuadrada === 0) return;

        const distancia = Math.sqrt(distanciaCuadrada);
        const nx = dx / distancia;
        const ny = dy / distancia;

        const dvx = a.vx - b.vx;
        const dvy = a.vy - b.vy;
        const velNormal = dvx * nx + dvy * ny;

        if (velNormal <= 0) return;

        const masaA = a.radius * a.radius;
        const masaB = b.radius * b.radius;
        const impulso = (2.0 * velNormal) / (masaA + masaB);

        a.vx -= impulso * masaB * nx;
        a.vy -= impulso * masaB * ny;
        b.vx += impulso * masaA * nx;
        b.vy += impulso * masaA * ny;

        const solapamiento = 0.5 * (sumaRadios - distancia);
        a.x -= solapamiento * nx;
        a.y -= solapamiento * ny;
        b.x += solapamiento * nx;
        b.y += solapamiento * ny;
    }

    reconstruirArbol() {
        this.arbol.limpiar();
        for (const p of this.particulas) {
            this.arbol.insertar(p);
        }
    }

    // mulberry32: PRNG con semilla, para escenarios reproducibles en el navegador
    _rng(semilla) {
        let a = semilla >>> 0;
        return function () {
            a |= 0; a = (a + 0x6D2B79F5) | 0;
            let t = Math.imul(a ^ (a >>> 15), 1 | a);
            t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
            return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
        };
    }

    generarUniforme(cantidad, radioMin, radioMax, velMax, semilla = 0) {
        this.particulas = [];
        this.radioMaximo = radioMax;
        const rnd = semilla ? this._rng(semilla) : Math.random;

        for (let i = 0; i < cantidad; i++) {
            const x = radioMax + rnd() * (this.anchoEspacio - 2 * radioMax);
            const y = radioMax + rnd() * (this.altoEspacio - 2 * radioMax);
            const vx = (rnd() * 2 - 1) * velMax;
            const vy = (rnd() * 2 - 1) * velMax;
            const r = radioMin + rnd() * (radioMax - radioMin);
            this.particulas.push(new Particle(i, x, y, vx, vy, r));
        }
    }

    agregarParticula(x, y, vx, vy, radius) {
        const id = this.particulas.length;
        this.particulas.push(new Particle(id, x, y, vx, vy, radius));
        if (radius > this.radioMaximo) this.radioMaximo = radius;
    }

    reset() {
        this.particulas = [];
        this.radioMaximo = 0;
        this.arbol.limpiar();
        this.comparacionesQT = 0;
    }

    actualizarFisica() {
        for (const p of this.particulas) {
            p.x += p.vx;
            p.y += p.vy;

            if (p.x - p.radius < 0) {
                p.x = p.radius;
                p.vx *= -1;
            } else if (p.x + p.radius > this.anchoEspacio) {
                p.x = this.anchoEspacio - p.radius;
                p.vx *= -1;
            }

            if (p.y - p.radius < 0) {
                p.y = p.radius;
                p.vy *= -1;
            } else if (p.y + p.radius > this.altoEspacio) {
                p.y = this.altoEspacio - p.radius;
                p.vy *= -1;
            }
        }
    }

    // un paso de simulacion via quadtree, con respuesta de choques. Actualiza comparacionesQT.
    simularPaso() {
        this.actualizarFisica();
        this.reconstruirArbol();

        const contador = { n: 0 };

        for (let i = 0; i < this.particulas.length; i++) {
            const p = this.particulas[i];
            const rangoBusqueda = p.radius + this.radioMaximo;

            const candidatos = [];
            this.arbol.consultarRadio(p.x, p.y, rangoBusqueda, candidatos, contador);

            for (const c of candidatos) {
                if (c.id > i) {
                    this.resolverColision(this.particulas[i], this.particulas[c.id]);
                }
            }
        }

        this.comparacionesQT = contador.n;
    }

    // paso determinista por fuerza bruta (orden i<j), para validacion cruzada con C++ (#18)
    simularPasoFB() {
        this.actualizarFisica();
        for (let i = 0; i < this.particulas.length; i++) {
            for (let j = i + 1; j < this.particulas.length; j++) {
                this.resolverColision(this.particulas[i], this.particulas[j]);
            }
        }
    }

    // comparaciones equivalentes de fuerza bruta para el HUD
    comparacionesFuerzaBruta() {
        const n = this.particulas.length;
        return (n * (n - 1)) / 2;
    }
}
