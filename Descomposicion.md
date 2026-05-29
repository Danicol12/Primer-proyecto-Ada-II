# Descomposición del problema — Programación dinámica (`roPD`)

Problema: encontrar el **orden de riego** de los tablones de una finca que **minimiza la penalización total** (función de costo definida en `Tablon::calcularCosto`).

---

## 1. Entrada

- Una **finca** con `n` tablones.
- Cada tablón `i` tiene:
  - `ts` — tiempo de supervivencia
  - `tr` — tiempo de regado (ocupa la manguera)
  - `p` — prioridad
  - `rp` — instante de riego perfecto

La penalización de regar un tablón depende del **momento** en que se inicia su riego (tiempo acumulado de los tablones regados antes).

---

## 2. Decisión

En cada paso debemos elegir **qué tablón regar a continuación** entre los que aún no se han regado.

Una programación óptima es una **permutación** `π` de los índices `{0, 1, …, n−1}`.

---

## 3. Subproblemas

No basta fijar “el siguiente tablón” sin recordar **cuáles ya se regaron**, porque el tiempo actual depende del conjunto regado (suma de sus `tr`).

**Subproblema:** dado un conjunto `S ⊆ {0,…,n−1}` de tablones ya regados, ¿cuál es la **penalización mínima** para regar los que faltan?

El **tiempo actual** al resolver ese subproblema es:

\[
t(S) = \sum_{i \in S} tr_i
\]

No hace falta guardar el orden en que se regó `S`: solo importa el conjunto.

---

## 4. Representación del estado (máscara de bits)

Representamos el conjunto `S` con un entero `mask` de `n` bits:

| Bit `i` | Significado        |
|---------|--------------------|
| `0`     | Tablón `i` pendiente |
| `1`     | Tablón `i` ya regado |

Ejemplo con `n = 5`: `mask = 0b01101` → regados los tablones 0, 2 y 4.

- **Estado inicial:** `mask = 0` (nadie regado).
- **Estado final:** `mask = (1 << n) - 1` (todos regados).

---

## 5. Valor óptimo y recurrencia

Sea `dp[mask]` el costo mínimo para terminar de regar los tablones que faltan, partiendo de `mask`.

**Caso base** (todos regados):

\[
dp[(1 \ll n) - 1] = 0
\]

**Caso general** (`mask` no completo). Sea `t = t(mask)` la suma de `tr` de los bits en 1. Para cada `i` con bit `i` en 0:

\[
\text{costo}(i) = \text{penalización}(i,\, t) + dp[\,mask \mid (1 \ll i)\,]
\]

\[
dp[mask] = \min_{\substack{i \,\mid\, \text{bit } i = 0}} \text{costo}(i)
\]

En código, `penalización(i, t)` es `tablones[i].calcularCosto(t)`.

**Respuesta del problema:** `dp[0]`.

---

## 6. Memoización

Hay `2^n` máscaras posibles. Usamos un arreglo:

```text
dp[mask] = -1   → aún no calculado
dp[mask] ≥ 0    → costo mínimo ya conocido
```

El algoritmo es **top-down**: la función `resolverPD(mask, …)` calcula `dp[mask]` recursivamente y reutiliza valores ya guardados.

**Complejidad:** `O(n · 2^n)` estados, cada uno prueba hasta `n` tablones.

---

## 7. Reconstrucción de la permutación óptima

Además de `dp`, guardamos `choice[mask]`: el índice del tablón que conviene regar **ahora** en el óptimo de ese estado.

Tras calcular `dp[0]`:

1. Partir con `mask = 0`.
2. Repetir `n` veces: tomar `i = choice[mask]`, añadir `i` a la permutación, actualizar `mask |= (1 << i)`.

Eso produce la programación `π` asociada al costo `dp[0]`.

---

## 8. Por qué es programación dinámica

1. **Subestructura óptima:** si el orden completo es óptimo, el suborden sobre los tablones restantes también debe ser óptimo para su máscara.
2. **Subproblemas superpuestos:** distintos órdenes pueden llegar al mismo `mask`; sin memoización se recalcula; con `dp` se calcula una vez.

Equivale al espacio de búsqueda de fuerza bruta, pero sin repetir el mismo subconjunto.

---

## 9. Funciones en el código (`Finca`)

| Función              | Rol                                              |
|----------------------|--------------------------------------------------|
| `tiempoDesdeMascara` | Calcula `t(mask)` sumando `tr` de bits en 1      |
| `resolverPD`         | Calcula `dp[mask]` con recursión + memoización   |
| `roPD`               | Inicializa tablas, resuelve y reconstruye `π`    |
| `roD`                | Alias que llama a `roPD()`                       |

---

## 10. Validación

Para `finca.txt` (5 tablones):

| Método | Costo mínimo | Permutación óptima      |
|--------|--------------|-------------------------|
| `roFB` | 160          | `< 1, 2, 3, 4, 0 >`   |
| `roPD` | 160          | `< 1, 2, 3, 4, 0 >`   |

Si coinciden costo y permutación con fuerza bruta en varias entradas pequeñas, la DP está correcta.
