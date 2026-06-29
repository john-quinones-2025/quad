// Espejo de struct Particle (QuadTree.h). id == indice en el vector de Simulador.
class Particle {
    constructor(id, x, y, vx, vy, radius) {
        this.id = id;
        this.x = x;
        this.y = y;
        this.vx = vx;
        this.vy = vy;
        this.radius = radius;
    }
}
