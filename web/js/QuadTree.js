// Espejo de QuadTree.cpp. Mismos nombres de metodo para validar paridad con el C++.
class QuadTree {
    constructor(frontera, capacidad) {
        this.limite = frontera;
        this.capacidadMaxima = capacidad;
        this.particulas = [];
        this.dividido = false;

        this.noroeste = null;
        this.noreste = null;
        this.suroeste = null;
        this.sureste = null;
    }

    subdividir() {
        const x = this.limite.x;
        const y = this.limite.y;
        const w = this.limite.ancho;
        const h = this.limite.alto;

        const f_no = new Frontera(x - w / 2, y + h / 2, w / 2, h / 2);
        const f_ne = new Frontera(x + w / 2, y + h / 2, w / 2, h / 2);
        const f_so = new Frontera(x - w / 2, y - h / 2, w / 2, h / 2);
        const f_se = new Frontera(x + w / 2, y - h / 2, w / 2, h / 2);

        this.noroeste = new QuadTree(f_no, this.capacidadMaxima);
        this.noreste  = new QuadTree(f_ne, this.capacidadMaxima);
        this.suroeste = new QuadTree(f_so, this.capacidadMaxima);
        this.sureste  = new QuadTree(f_se, this.capacidadMaxima);

        this.dividido = true;

        for (const p of this.particulas) {
            if (this.noroeste.insertar(p)) continue;
            if (this.noreste.insertar(p)) continue;
            if (this.suroeste.insertar(p)) continue;
            if (this.sureste.insertar(p)) continue;
        }

        this.particulas = [];
    }

    insertar(p) {
        if (!this.limite.contiene(p.x, p.y)) {
            return false;
        }

        if (!this.dividido) {
            if (this.particulas.length < this.capacidadMaxima) {
                this.particulas.push(p);
                return true;
            } else {
                this.subdividir();
            }
        }

        if (this.noroeste.insertar(p)) return true;
        if (this.noreste.insertar(p)) return true;
        if (this.suroeste.insertar(p)) return true;
        if (this.sureste.insertar(p)) return true;

        return false;
    }

    // query rectangular (primitivo canonico, util para culling de viewport)
    consultarRango(rango, encontrados, contador) {
        if (!this.limite.intersecta(rango)) {
            return;
        }

        if (!this.dividido) {
            for (const p of this.particulas) {
                contador.n++;
                if (rango.contiene(p.x, p.y)) {
                    encontrados.push(p);
                }
            }
        } else {
            this.noroeste.consultarRango(rango, encontrados, contador);
            this.noreste.consultarRango(rango, encontrados, contador);
            this.suroeste.consultarRango(rango, encontrados, contador);
            this.sureste.consultarRango(rango, encontrados, contador);
        }
    }

    // query circular: filtra por distancia euclidiana (correcto para colisiones por radio)
    consultarRadio(px, py, radioBuscado, encontrados, contador) {
        const rango = new Frontera(px, py, radioBuscado, radioBuscado);

        if (!this.limite.intersecta(rango)) {
            return;
        }

        if (!this.dividido) {
            for (const p of this.particulas) {
                contador.n++;
                const dx = p.x - px;
                const dy = p.y - py;
                const distanciaCuadrada = dx * dx + dy * dy;
                if (distanciaCuadrada <= radioBuscado * radioBuscado) {
                    encontrados.push(p);
                }
            }
        } else {
            this.noroeste.consultarRadio(px, py, radioBuscado, encontrados, contador);
            this.noreste.consultarRadio(px, py, radioBuscado, encontrados, contador);
            this.suroeste.consultarRadio(px, py, radioBuscado, encontrados, contador);
            this.sureste.consultarRadio(px, py, radioBuscado, encontrados, contador);
        }
    }

    limpiar() {
        this.particulas = [];
        if (this.dividido) {
            this.noroeste.limpiar();
            this.noreste.limpiar();
            this.suroeste.limpiar();
            this.sureste.limpiar();
            this.noroeste = null;
            this.noreste = null;
            this.suroeste = null;
            this.sureste = null;
            this.dividido = false;
        }
    }

    // dibuja las lineas de subdivision; colorea hojas por densidad (#11, #19)
    dibujar(ctx, capacidad) {
        const x0 = this.limite.x - this.limite.ancho;
        const y0 = this.limite.y - this.limite.alto;
        const w = this.limite.ancho * 2;
        const h = this.limite.alto * 2;

        if (!this.dividido) {
            // relleno por densidad: mas particulas -> mas opaco
            const dens = Math.min(1, this.particulas.length / capacidad);
            if (dens > 0) {
                ctx.fillStyle = `rgba(0, 220, 255, ${0.05 + dens * 0.25})`;
                ctx.fillRect(x0, y0, w, h);
            }
        }

        ctx.strokeStyle = 'rgba(0, 220, 255, 0.25)';
        ctx.lineWidth = 0.5;
        ctx.strokeRect(x0, y0, w, h);

        if (this.dividido) {
            this.noroeste.dibujar(ctx, capacidad);
            this.noreste.dibujar(ctx, capacidad);
            this.suroeste.dibujar(ctx, capacidad);
            this.sureste.dibujar(ctx, capacidad);
        }
    }
}
