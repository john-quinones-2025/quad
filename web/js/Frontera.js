// Espejo de struct Frontera (QuadTree.h). (x, y) es el centro; ancho/alto son MITADES.
class Frontera {
    constructor(x, y, ancho, alto) {
        this.x = x;
        this.y = y;
        this.ancho = ancho; // mitad del ancho
        this.alto = alto;   // mitad del alto
    }

    contiene(px, py) {
        return (px >= this.x - this.ancho && px <= this.x + this.ancho &&
                py >= this.y - this.alto && py <= this.y + this.alto);
    }

    intersecta(rango) {
        return !(rango.x - rango.ancho > this.x + this.ancho ||
                 rango.x + rango.ancho < this.x - this.ancho ||
                 rango.y - rango.alto > this.y + this.alto ||
                 rango.y + rango.alto < this.y - this.alto);
    }
}
