# QuadTree — Simulador de colisiones de partículas

Proyecto académico (AED, UTEC) que demuestra cómo un **QuadTree** reduce el costo
de la detección de colisiones de **O(n²)** (fuerza bruta) a **≈ O(n log n)**
repartiendo las partículas en un árbol espacial.

El proyecto tiene **dos mitades que comparten la misma lógica**:

- **Backend C++** — el *benchmark* y la entrega académica. Mide comparaciones y
  tiempos de Fuerza Bruta vs QuadTree a distintos N y distribuciones, y exporta
  un CSV para graficar.
- **Frontend Web** — visualización interactiva (canvas + JS vanilla) del mismo
  algoritmo: partículas rebotando, el árbol subdividiéndose en vivo, y métricas
  en tiempo real.

Ambas implementaciones se validan cruzadamente: el C++ vuelca un escenario
determinista a `expected.json` y el JS lo reproduce comprobando **paridad bit a
bit**.

---

## Estructura del proyecto

```
quad/
├── main.cpp            # punto de entrada: corre experimentos, simulación y validación
├── Simulador.h/.cpp    # física, generadores de escenarios, benchmarks
├── QuadTree.h/.cpp     # la estructura de datos espacial (+ structs Particle y Frontera)
│
├── web/                # frontend interactivo (sin frameworks)
│   ├── index.html      # canvas + HUD + panel de controles
│   ├── style.css
│   ├── expected.json   # escenario de validación generado por el C++
│   └── js/
│       ├── Particle.js   # espejo de struct Particle
│       ├── Frontera.js   # espejo de struct Frontera
│       ├── QuadTree.js   # espejo de la clase QuadTree
│       ├── Simulador.js  # espejo de la clase Simulador
│       └── main.js       # loop de animación, interacción, HUD, validación
│
├── benchmark.csv       # (generado) resultados del benchmark C++
└── .gitignore
```

---

## Arquitectura

### Modelo de datos (compartido C++ ↔ JS)

- **`Particle`** — `{ id, x, y, vx, vy, radius }`. Invariante clave:
  **`id == índice`** dentro del vector de partículas del `Simulador`. Esto permite
  resolver colisiones modificando la partícula real a partir de un candidato.
- **`Frontera`** — caja axis-aligned definida por su **centro** `(x, y)` y sus
  **semi-dimensiones** `ancho`/`alto` (mitades). Métodos `contiene(px, py)` e
  `intersecta(rango)`.

### `QuadTree` — la estructura espacial

Árbol cuaternario: cada nodo cubre una región rectangular y guarda hasta
`capacidadMaxima` partículas. Al rebasar esa capacidad **se subdivide** en 4
hijos (NO, NE, SO, SE) y reparte sus partículas. Las partículas viven siempre en
las **hojas**.

| Método | Qué hace |
|--------|----------|
| `insertar(p)` | Inserta una partícula, subdividiendo si hace falta. |
| `consultarRango(rango, encontrados, contador)` | Query **rectangular** (primitivo canónico; útil para culling de viewport). |
| `consultarRadio(px, py, r, encontrados, contador)` | Query **circular**: filtra por distancia euclidiana. Es la que usa la detección de colisiones. |
| `limpiar()` | Vacía el árbol dejando la raíz **reutilizable** (sin reasignar su memoria). |
| `dibujar(ctx, capacidad)` *(solo JS)* | Pinta las subdivisiones y colorea hojas por densidad. |

El `contador` cuenta las comparaciones partícula-a-partícula de la query: es la
métrica central del benchmark.

### `Simulador` — física y orquestación

- **Generadores**: `generarUniforme`, `generarClusters`, `generarAltaDensidad`
  (con semilla opcional para reproducibilidad).
- **`actualizarFisica()`**: mueve cada partícula y aplica **rebote en los bordes**.
- **`resolverColision(a, b)`**: respuesta física — **colisión elástica** con masa
  proporcional al área (`r²`) y separación del solapamiento.
- **`simularPaso()`**: un frame → física + reconstrucción del árbol + detección
  por `consultarRadio` + resolución de choques.
- **Benchmarks puros**: `reporteColisionesFuerzaBruta` y
  `reporteColisionesQuadTree` cuentan comparaciones y miden tiempo.

---

## Análisis de costo computacional

Sea **n** el número de partículas, **c** la capacidad por nodo, **h** la altura
del árbol y **k** los candidatos examinados por consulta.

### Costo por operación

| Operación | Costo | Comentario |
|-----------|-------|------------|
| `Frontera::contiene` / `intersecta` | **O(1)** | Solo comparaciones aritméticas. |
| `QuadTree::insertar` | **O(h)** promedio | Desciende de la raíz a una hoja. Con distribución uniforme `h ≈ log₄(n)` → **O(log n)**. |
| Construir el árbol (n inserciones) | **O(n·h) ≈ O(n log n)** | |
| `consultarRadio` (1 partícula) | **O(h + k)** | Baja por las ramas que intersectan el círculo y revisa las hojas vecinas. |
| **Detección QuadTree (n partículas)** | **≈ O(n log n + Σk)** | A densidad fija, Σk crece **linealmente** con n. |
| **Detección Fuerza Bruta** | **O(n²)** | Exactamente `n·(n−1)/2` comparaciones. |
| Espacio | **O(n)** | Partículas + nodos del árbol (≈ O(n)). |

### Por qué el QuadTree reduce el costo

- **Fuerza bruta** compara **toda partícula contra todas**: cada una hace ≈ `n−1`
  comparaciones → `n·(n−1)/2` en total. Cuadrático.
- **QuadTree**: cada partícula solo consulta un círculo de radio
  `radio_i + radioMáximoGlobal` a su alrededor y compara contra las **pocas
  partículas de las hojas vecinas**, no contra toda la población. Sumar
  `radioMáximoGlobal` garantiza que una partícula chica **sí** detecte a una
  grande cercana.
- **La clave del ahorro**: en un espacio de tamaño fijo, al crecer `n` la
  **densidad local** (candidatos por consulta) se mantiene acotada, mientras que
  el total de pares crece como `n²`. Por eso **la ventaja se ensancha con n**
  (ver la columna *razón* abajo: 98× → 265× → 363× al pasar de 1 000 a 10 000).

### Reducción medida (de `benchmark.csv`)

Comparaciones partícula-a-partícula realizadas. Espacio 800×800, radios 2–5,
capacidad de nodo = 4:

| N | Distribución | Comp. Fuerza Bruta | Comp. QuadTree | **Razón FB/QT** | Tiempo FB | Tiempo QT |
|------:|--------------|------------------:|---------------:|----------:|---------:|---------:|
| 1 000 | Uniforme | 499 500 | 5 058 | **98.8×** | 1.3 ms | 1.0 ms |
| 1 000 | Clusters | 499 500 | 11 933 | 41.9× | 1.3 ms | 2.0 ms |
| 1 000 | Alta densidad | 499 500 | 10 359 | 48.2× | 1.1 ms | 1.4 ms |
| 5 000 | Uniforme | 12 497 500 | 47 080 | **265.5×** | 36.1 ms | 8.9 ms |
| 5 000 | Clusters | 12 497 500 | 180 557 | 69.2× | 33.8 ms | 17.1 ms |
| 5 000 | Alta densidad | 12 497 500 | 142 282 | 87.8× | 32.4 ms | 14.3 ms |
| 10 000 | Uniforme | 49 995 000 | 137 718 | **363.0×** | 145.4 ms | 22.1 ms |
| 10 000 | Clusters | 49 995 000 | 467 190 | 107.0× | 129.5 ms | 36.9 ms |
| 10 000 | Alta densidad | 49 995 000 | 471 931 | 105.9× | 129.4 ms | 38.3 ms |

> Cifras de una corrida; varían ligeramente entre ejecuciones por el azar de los
> generadores, pero el orden de magnitud es estable.

**Lecturas del experimento:**

1. **El crecimiento confirma la teoría.** En distribución uniforme, FB crece
   cuadráticamente (×100 en comparaciones de 1 k a 10 k) mientras QT crece
   ~linealmente (×27), de ahí que la razón salte de **98× a 363×**.
2. **La distribución importa.** Con *clusters* y *alta densidad* las partículas
   se apiñan, las hojas se llenan y cada consulta revisa más candidatos
   (`cand_promedio` sube de ~1.4 a ~24), así que la ventaja del QuadTree baja
   (~106× en vez de 363×). Es el **peor caso** del particionado espacial: si todo
   está en un punto, el árbol no puede separar nada y localmente degenera a O(n²).
3. **Tiempo de pared.** A N = 10 000 uniforme el QuadTree es **~6.6× más rápido**
   en reloj (22 ms vs 145 ms), pese a pagar el costo extra de **construir** el
   árbol cada frame.

### Optimizaciones de constante implementadas

Estas no cambian la complejidad asintótica pero reducen el costo real:

- **Reuso del árbol** (`limpiar()` + reinsertar en vez de `new/delete` cada
  frame): el `QuadTree` es **miembro** del `Simulador`; se evita reasignar la raíz
  y se reduce la presión sobre el asignador de memoria.
- **Paso por `const referencia`** de `Particle`/`Frontera` en el *hot path*: evita
  copiar 48 bytes por llamada en `insertar`, `verificanColision`, etc.
- **Query circular** (`consultarRadio`) en vez de rectangular: descarta los
  falsos positivos de las esquinas de la caja, reduciendo `k`.
- **Medición honesta de tiempos**: un sumidero `volatile` impide que `-O2` elimine
  las llamadas a `verificanColision` cuyo resultado se descarta (sin él, FB medía
  tiempos falsamente bajos).

### Limitación conocida

Si muchas partículas comparten **exactamente** la misma posición, una hoja nunca
logra separarlas al subdividir y el árbol recursa sin fin. Con coordenadas
`double` aleatorias esto no ocurre en la práctica, pero es el caso patológico a
tener en cuenta.

---

## Cómo iniciar el proyecto (paso a paso)

### Requisitos
- Compilador C++ con C++17 (`g++` o `clang++`).
- Python 3 (o cualquier servidor http estático) para el frontend.

### 1. Compilar y correr el backend C++

```bash
g++ -std=c++17 -O2 main.cpp Simulador.cpp QuadTree.cpp -o quadtree
./quadtree
```

Esto:
1. Imprime la tabla **Fuerza Bruta vs QuadTree** para N = 1000, 5000, 10000 en
   tres distribuciones (uniforme, clusters, alta densidad).
2. Escribe **`benchmark.csv`** (listo para graficar).
3. Corre una **simulación dinámica** mostrando comparaciones FB vs QT por frame.
4. Genera **`web/expected.json`** para la validación cruzada.

### 2. Levantar el frontend web

El `fetch` de `expected.json` no funciona con `file://`, así que hay que servir
por http:

```bash
cd web
python3 -m http.server 8000
```

Abre **http://localhost:8000** en el navegador.

### 3. Usar la visualización

| Acción | Cómo |
|--------|------|
| Añadir partícula | **Click-drag** (dirección/longitud del arrastre = velocidad). |
| Zoom | **Rueda** del mouse (centra en el cursor). |
| Pan | **Click derecho + arrastrar**. |
| Pausar / reanudar | Botón **Pausar**. |
| Reiniciar | Botón **Reset** (usa el slider *N inicial*). |
| Capacidad del nodo | Slider **Capacidad nodo** (1–16). |
| Mostrar/ocultar árbol | Checkbox **Mostrar QuadTree**. |
| Validar contra C++ | Botón **Validar vs C++** (requiere `expected.json` del paso 1). |

El **HUD** muestra en vivo: nº de partículas, FPS, comparaciones del QuadTree,
comparaciones equivalentes de fuerza bruta y la **mejora** (razón entre ambas).

---

## Notas

- `benchmark.csv` y el binario `quadtree` están en `.gitignore` (artefactos
  generados).
- El frontend es **vanilla JS** a propósito: cada clase es un espejo 1:1 del C++
  para comparar ambas implementaciones con facilidad.
