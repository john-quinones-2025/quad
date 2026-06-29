// ============================================================================
// main.js — loop de animacion, render, interaccion y HUD (#13-#19)
// ============================================================================

const MUNDO = 800; // lado del espacio de simulacion (igual que el C++)

const canvas = document.getElementById('lienzo');
const ctx = canvas.getContext('2d');

let sim = new Simulador(MUNDO, MUNDO, 4);

// estado de UI
let pausado = false;
let mostrarArbol = true;
let iniciado = false;       // no simulamos hasta que se elija una modalidad
let modoTest = false;       // modo validacion de consultas espaciales (estatico)
let testData = null;        // { caja, circulo, rectIds, circIds, encontrados }

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

// centra y escala el mundo (del tamaño del sim actual) para que quepa en el canvas
function encajarMundo() {
    const lado = Math.max(sim.anchoEspacio, sim.altoEspacio);
    const escala = Math.min(canvas.width, canvas.height) / lado * 0.9;
    camara.zoom = escala;
    camara.x = (canvas.width - sim.anchoEspacio * escala) / 2;
    camara.y = (canvas.height - sim.altoEspacio * escala) / 2;
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
    ctx.strokeRect(0, 0, sim.anchoEspacio, sim.altoEspacio);

    if (mostrarArbol) {
        sim.reconstruirArbol();
        // en modo test mostramos solo las lineas del arbol para no tapar las regiones de consulta
        sim.arbol.dibujar(ctx, sim.capacidadQuadTree, modoTest);
    }

    // en modo test dibujamos las regiones de consulta debajo de las particulas
    if (modoTest && testData) renderTest();

    // particulas (resaltadas si fueron encontradas por alguna consulta del test)
    for (const p of sim.particulas) {
        ctx.fillStyle = (modoTest && testData && testData.encontrados.has(p.id)) ? '#06d6a0' : '#ffd166';
        ctx.beginPath();
        ctx.arc(p.x, p.y, p.radius, 0, Math.PI * 2);
        ctx.fill();

        // etiqueta de ID en modo test
        if (modoTest) {
            ctx.fillStyle = '#fff';
            ctx.font = `${6}px sans-serif`;
            ctx.fillText('ID ' + p.id, p.x + p.radius + 2, p.y - p.radius - 2);
        }
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

// dibuja las regiones de consulta del modo test
function renderTest() {
    const z = camara.zoom;
    const fs = 11 / z;
    ctx.font = `${fs}px sans-serif`;
    ctx.lineWidth = 2.5 / z;
    ctx.setLineDash([6 / z, 4 / z]);

    // caja rectangular (consultarRango)
    const c = testData.caja;
    ctx.fillStyle = 'rgba(77, 171, 247, 0.30)';
    ctx.fillRect(c.x - c.ancho, c.y - c.alto, c.ancho * 2, c.alto * 2);
    ctx.strokeStyle = '#4dabf7';
    ctx.strokeRect(c.x - c.ancho, c.y - c.alto, c.ancho * 2, c.alto * 2);

    // circulo (consultarRadio)
    const o = testData.circulo;
    ctx.beginPath();
    ctx.arc(o.x, o.y, o.r, 0, Math.PI * 2);
    ctx.fillStyle = 'rgba(255, 169, 77, 0.30)';
    ctx.fill();
    ctx.strokeStyle = '#ffa94d';
    ctx.stroke();

    ctx.setLineDash([]);

    // etiquetas de cada consulta
    ctx.fillStyle = '#4dabf7';
    ctx.fillText('consultarRango', c.x - c.ancho, c.y + c.alto + fs * 1.3);
    ctx.fillStyle = '#ffa94d';
    ctx.fillText('consultarRadio', o.x - o.r, o.y - o.r - fs * 0.5);

    // anillo blanco en las particulas encontradas
    ctx.strokeStyle = '#ffffff';
    ctx.lineWidth = 2 / z;
    for (const p of sim.particulas) {
        if (testData.encontrados.has(p.id)) {
            ctx.beginPath();
            ctx.arc(p.x, p.y, p.radius + 4 / z + 2, 0, Math.PI * 2);
            ctx.stroke();
        }
    }
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

    if (iniciado && !pausado) {
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
        if (!modoTest && m0.x > 0 && m0.x < sim.anchoEspacio && m0.y > 0 && m0.y < sim.altoEspacio) {
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

const sliderCap = document.getElementById('slider-cap');
sliderCap.addEventListener('input', () => {
    const cap = parseInt(sliderCap.value, 10);
    document.getElementById('val-cap').textContent = cap;
    sim.setCapacidad(cap);
});

document.getElementById('toggle-arbol').addEventListener('change', (e) => {
    mostrarArbol = e.target.checked;
});

// ---------------------------------------------------------------------------
// Pantalla de inicio con modalidades
// ---------------------------------------------------------------------------
const overlay = document.getElementById('overlay');
const clustersWrap = document.getElementById('cfg-clusters-wrap');
let modoSeleccionado = 'uniforme';

document.querySelectorAll('.modo').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.modo').forEach(b => b.classList.remove('seleccionado'));
        btn.classList.add('seleccionado');
        modoSeleccionado = btn.dataset.modo;
        clustersWrap.style.display = (modoSeleccionado === 'clusters') ? 'flex' : 'none';
        // el test usa un escenario fijo de 4 partículas; N no aplica
        document.getElementById('cfg-n-wrap').style.display = (modoSeleccionado === 'test') ? 'none' : 'flex';
    });
});

function aplicarModalidad() {
    const cap = parseInt(sliderCap.value, 10) || 4;

    if (modoSeleccionado === 'test') {
        configurarTest(cap);
        overlay.classList.add('oculto');
        return;
    }

    // modos dinamicos: mundo 800x800
    modoTest = false;
    testData = null;
    document.getElementById('test-info').style.display = 'none';

    const n = Math.max(1, parseInt(document.getElementById('cfg-n').value, 10) || 1000);
    const grupos = Math.max(1, parseInt(document.getElementById('cfg-clusters').value, 10) || 5);

    sim = new Simulador(MUNDO, MUNDO, cap);
    // radios 2-6, velocidad maxima 2 (igual que el resto del proyecto)
    if (modoSeleccionado === 'clusters') {
        sim.generarClusters(n, grupos, 2, 6, 2);
    } else if (modoSeleccionado === 'densidad') {
        sim.generarAltaDensidad(n, 2, 6, 2);
    } else {
        sim.generarUniforme(n, 2, 6, 2);
    }

    iniciado = true;
    pausado = false;
    btnPausa.textContent = 'Pausar';
    encajarMundo();
    overlay.classList.add('oculto');
}

// Monta el escenario de validacion de consultas: 4 esquinas de un mapa 200x200,
// query rectangular en la esquina superior izquierda y circular en la inferior derecha.
function configurarTest(cap) {
    sim = new Simulador(200, 200, cap);
    sim.particulas = [
        new Particle(1,  10,  10, 0, 0, 7),  // superior izquierda
        new Particle(2, 190,  10, 0, 0, 7),  // superior derecha
        new Particle(3,  10, 190, 0, 0, 7),  // inferior izquierda
        new Particle(4, 190, 190, 0, 0, 7),  // inferior derecha
    ];
    sim.radioMaximo = 7;
    sim.reconstruirArbol();

    const caja = new Frontera(10, 10, 20, 20);          // esquina superior izquierda
    const rect = [];
    sim.arbol.consultarRango(caja, rect, { n: 0 });

    const circulo = { x: 190, y: 190, r: 30 };           // esquina inferior derecha
    const circ = [];
    sim.arbol.consultarRadio(circulo.x, circulo.y, circulo.r, circ, { n: 0 });

    const rectIds = rect.map(p => p.id);
    const circIds = circ.map(p => p.id);
    testData = {
        caja, circulo, rectIds, circIds,
        encontrados: new Set([...rectIds, ...circIds])
    };

    modoTest = true;
    iniciado = false;   // escenario estatico, sin fisica
    pausado = true;
    encajarMundo();
    mostrarInfoTest();
}

function mostrarInfoTest() {
    const okR = testData.rectIds.length === 1 && testData.rectIds[0] === 1;
    const okC = testData.circIds.length === 1 && testData.circIds[0] === 4;
    const el = document.getElementById('test-info');
    el.innerHTML =
        '<div class="ti-tit">Validación de consultas espaciales</div>' +
        '<div class="ti-fila"><b style="color:#4dabf7">1. Rango rectangular</b> — esquina superior izquierda<br>' +
        'Esperado: ID 1 · Encontrado: ' + (testData.rectIds.join(', ') || '—') + ' ' + (okR ? '✅' : '❌') + '</div>' +
        '<div class="ti-fila"><b style="color:#ffa94d">2. Radio circular</b> — esquina inferior derecha<br>' +
        'Esperado: ID 4 · Encontrado: ' + (testData.circIds.join(', ') || '—') + ' ' + (okC ? '✅' : '❌') + '</div>';
    el.style.display = 'block';
}

document.getElementById('btn-iniciar').addEventListener('click', aplicarModalidad);

document.getElementById('btn-nuevo').addEventListener('click', () => {
    overlay.classList.remove('oculto');
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
