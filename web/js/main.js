// ============================================================================
// main.js — loop de animacion, render, interaccion y HUD (#13-#19)
// ============================================================================

const MUNDO = 800; // lado del espacio de simulacion (igual que el C++)

const canvas = document.getElementById('lienzo');
const ctx = canvas.getContext('2d');

let sim = new Simulador(MUNDO, MUNDO, 4);
sim.generarUniforme(300, 2, 6, 2);

// estado de UI
let pausado = false;
let mostrarArbol = true;

// camara: world = (screen - offset) / zoom
const camara = { x: 0, y: 0, zoom: 1 };

// fps
let ultimoT = performance.now();
let fps = 0;

// ---------------------------------------------------------------------------
// Canvas responsive
// ---------------------------------------------------------------------------
function ajustarCanvas() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    encajarMundo();
}

// centra y escala el mundo para que quepa en el canvas
function encajarMundo() {
    const escala = Math.min(canvas.width, canvas.height) / MUNDO * 0.9;
    camara.zoom = escala;
    camara.x = (canvas.width - MUNDO * escala) / 2;
    camara.y = (canvas.height - MUNDO * escala) / 2;
}

window.addEventListener('resize', ajustarCanvas);

function pantallaAMundo(sx, sy) {
    return {
        x: (sx - camara.x) / camara.zoom,
        y: (sy - camara.y) / camara.zoom
    };
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------
function render() {
    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.fillStyle = '#0a0e14';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    ctx.setTransform(camara.zoom, 0, 0, camara.zoom, camara.x, camara.y);

    // borde del mundo
    ctx.strokeStyle = 'rgba(255,255,255,0.2)';
    ctx.lineWidth = 1;
    ctx.strokeRect(0, 0, MUNDO, MUNDO);

    if (mostrarArbol) {
        sim.reconstruirArbol();
        sim.arbol.dibujar(ctx, sim.capacidadQuadTree);
    }

    // particulas
    ctx.fillStyle = '#ffd166';
    for (const p of sim.particulas) {
        ctx.beginPath();
        ctx.arc(p.x, p.y, p.radius, 0, Math.PI * 2);
        ctx.fill();
    }

    // previsualizacion del arrastre (velocidad inicial)
    if (arrastrando.activo) {
        const m0 = pantallaAMundo(arrastrando.x0, arrastrando.y0);
        const m1 = pantallaAMundo(arrastrando.x1, arrastrando.y1);
        ctx.strokeStyle = '#ff6b6b';
        ctx.lineWidth = 2 / camara.zoom;
        ctx.beginPath();
        ctx.moveTo(m0.x, m0.y);
        ctx.lineTo(m1.x, m1.y);
        ctx.stroke();
    }

    ctx.setTransform(1, 0, 0, 1, 0, 0);
}

// ---------------------------------------------------------------------------
// HUD (#16)
// ---------------------------------------------------------------------------
function actualizarHUD() {
    const n = sim.particulas.length;
    const compFB = sim.comparacionesFuerzaBruta();
    const compQT = sim.comparacionesQT;
    const razon = compQT > 0 ? (compFB / compQT).toFixed(1) : '-';

    document.getElementById('hud-n').textContent = n;
    document.getElementById('hud-fps').textContent = fps.toFixed(0);
    document.getElementById('hud-qt').textContent = compQT.toLocaleString();
    document.getElementById('hud-fb').textContent = compFB.toLocaleString();
    document.getElementById('hud-razon').textContent = razon + '×';
}

// ---------------------------------------------------------------------------
// Loop (#13)
// ---------------------------------------------------------------------------
function frame(t) {
    const dt = t - ultimoT;
    ultimoT = t;
    fps = 1000 / dt;

    if (!pausado) {
        sim.simularPaso();
    }
    render();
    actualizarHUD();
    requestAnimationFrame(frame);
}

// ---------------------------------------------------------------------------
// Interaccion: click-drag para anadir (#14), zoom/pan (#15)
// ---------------------------------------------------------------------------
const arrastrando = { activo: false, x0: 0, y0: 0, x1: 0, y1: 0 };
const paneo = { activo: false, x: 0, y: 0 };

canvas.addEventListener('mousedown', (e) => {
    if (e.button === 0) {
        arrastrando.activo = true;
        arrastrando.x0 = arrastrando.x1 = e.clientX;
        arrastrando.y0 = arrastrando.y1 = e.clientY;
    } else if (e.button === 2) {
        paneo.activo = true;
        paneo.x = e.clientX;
        paneo.y = e.clientY;
    }
});

canvas.addEventListener('mousemove', (e) => {
    if (arrastrando.activo) {
        arrastrando.x1 = e.clientX;
        arrastrando.y1 = e.clientY;
    }
    if (paneo.activo) {
        camara.x += e.clientX - paneo.x;
        camara.y += e.clientY - paneo.y;
        paneo.x = e.clientX;
        paneo.y = e.clientY;
    }
});

canvas.addEventListener('mouseup', (e) => {
    if (e.button === 0 && arrastrando.activo) {
        arrastrando.activo = false;
        const m0 = pantallaAMundo(arrastrando.x0, arrastrando.y0);
        const m1 = pantallaAMundo(arrastrando.x1, arrastrando.y1);
        // velocidad = vector de arrastre escalado (click simple => velocidad pequena aleatoria)
        let vx = (m1.x - m0.x) * 0.1;
        let vy = (m1.y - m0.y) * 0.1;
        if (vx === 0 && vy === 0) {
            vx = (Math.random() * 2 - 1) * 2;
            vy = (Math.random() * 2 - 1) * 2;
        }
        const r = 3 + Math.random() * 4;
        if (m0.x > 0 && m0.x < MUNDO && m0.y > 0 && m0.y < MUNDO) {
            sim.agregarParticula(m0.x, m0.y, vx, vy, r);
        }
    }
    if (e.button === 2) paneo.activo = false;
});

canvas.addEventListener('contextmenu', (e) => e.preventDefault());

canvas.addEventListener('wheel', (e) => {
    e.preventDefault();
    const factor = e.deltaY < 0 ? 1.1 : 1 / 1.1;
    // zoom alrededor del cursor: mantener el punto del mundo bajo el cursor fijo
    const mundoAntes = pantallaAMundo(e.clientX, e.clientY);
    camara.zoom *= factor;
    camara.x = e.clientX - mundoAntes.x * camara.zoom;
    camara.y = e.clientY - mundoAntes.y * camara.zoom;
}, { passive: false });

// ---------------------------------------------------------------------------
// Controles (#17)
// ---------------------------------------------------------------------------
const btnPausa = document.getElementById('btn-pausa');
btnPausa.addEventListener('click', () => {
    pausado = !pausado;
    btnPausa.textContent = pausado ? 'Reanudar' : 'Pausar';
});

document.getElementById('btn-reset').addEventListener('click', () => {
    const n = parseInt(document.getElementById('slider-n').value, 10);
    sim.reset();
    sim.generarUniforme(n, 2, 6, 0);
});

const sliderCap = document.getElementById('slider-cap');
sliderCap.addEventListener('input', () => {
    const cap = parseInt(sliderCap.value, 10);
    document.getElementById('val-cap').textContent = cap;
    sim.setCapacidad(cap);
});

const sliderN = document.getElementById('slider-n');
sliderN.addEventListener('input', () => {
    document.getElementById('val-n').textContent = sliderN.value;
});

document.getElementById('toggle-arbol').addEventListener('change', (e) => {
    mostrarArbol = e.target.checked;
});

// ---------------------------------------------------------------------------
// Validacion cruzada C++ <-> JS (#18). Requiere servir por http (fetch).
// ---------------------------------------------------------------------------
document.getElementById('btn-validar').addEventListener('click', async () => {
    const salida = document.getElementById('validacion-salida');
    salida.textContent = 'Cargando expected.json...';
    try {
        const resp = await fetch('expected.json');
        if (!resp.ok) throw new Error('HTTP ' + resp.status);
        const data = await resp.json();

        const s = new Simulador(data.ancho, data.alto, 4);
        s.particulas = data.inicial.map(p => new Particle(p.id, p.x, p.y, p.vx, p.vy, p.radius));
        s.radioMaximo = Math.max(...data.inicial.map(p => p.radius));

        for (let f = 0; f < data.frames; f++) {
            s.simularPasoFB();
        }

        let maxErr = 0;
        for (let i = 0; i < s.particulas.length; i++) {
            const a = s.particulas[i];
            const b = data.esperado[i];
            maxErr = Math.max(maxErr, Math.abs(a.x - b.x), Math.abs(a.y - b.y));
        }

        const ok = maxErr < 1e-6;
        salida.textContent = (ok ? '✅ PARIDAD OK' : '❌ DIFERENCIA') +
            ` — error maximo = ${maxErr.toExponential(3)} sobre ${s.particulas.length} particulas, ${data.frames} frames`;
        salida.style.color = ok ? '#06d6a0' : '#ff6b6b';
    } catch (err) {
        salida.textContent = 'No se pudo validar: ' + err.message +
            ' (sirve la carpeta con un servidor http y corre el binario C++ para generar expected.json)';
        salida.style.color = '#ff6b6b';
    }
});

// ---------------------------------------------------------------------------
// arranque
// ---------------------------------------------------------------------------
ajustarCanvas();
requestAnimationFrame(frame);
