/* ═══════════════════════════════════════════════════════════
   Optimizacion de Riego — ADA  |  Frontend Logic
   ═══════════════════════════════════════════════════════════ */

(function() {
    'use strict';

    // ─── State ─────────────────────────────────────────────
    let tablones = [];
    let resultados = {};        // { clave: { nombre, costo, permutacion } }
    let lastAlgo = null;        // ultimo algoritmo ejecutado ("fb", "voraz", "pd", "todos")
    let startTime = null;
    let timerInterval = null;

    // ─── DOM References ────────────────────────────────────
    const $ = id => document.getElementById(id);
    const fileInput      = $('fileInput');
    const btnLoadLabel   = $('btnLoadLabel');
    const emptyState     = $('emptyState');
    const tableWrap      = $('tableWrap');
    const dataBody       = $('dataBody');
    const fileInfo       = $('fileInfo');
    const fileInfoText   = $('fileInfoText');
    const resultsEmpty   = $('resultsEmpty');
    const resultsCont    = $('resultsContainer');
    const cardActions    = $('cardActions');
    const statusMsg      = $('statusMsg');
    const statusTimer    = $('statusTimer');
    const verifyModal    = $('verifyModal');
    const modalBody      = $('modalBody');
    const modalClose     = $('modalClose');
    const modalCloseBtn  = $('modalCloseBtn');

    // ─── Helpers ───────────────────────────────────────────

    function setStatus(msg, isError) {
        statusMsg.textContent = msg;
        statusMsg.style.color = isError ? 'var(--red)' : '';
        if (isError) setTimeout(() => statusMsg.style.color = '', 3000);
    }

    function setTimer(text) {
        statusTimer.textContent = text;
    }

    function showError(msg) {
        setStatus('❌ ' + msg, true);
    }

    function showLoading(show) {
        // Simple loading indicator on buttons
        const btns = document.querySelectorAll('.btn-accent, #btnVerify, #btnDownload');
        btns.forEach(b => b.disabled = show);
        if (show) setStatus('⏳ Procesando...');
    }

    function startTimer() {
        if (timerInterval) clearInterval(timerInterval);
        startTime = Date.now();
        timerInterval = setInterval(() => {
            const sec = Math.floor((Date.now() - startTime) / 1000);
            const m = Math.floor(sec / 60);
            const s = sec % 60;
            setTimer(`⏱ ${m}:${s.toString().padStart(2, '0')}`);
        }, 1000);
    }

    function stopTimer() {
        if (timerInterval) {
            clearInterval(timerInterval);
            timerInterval = null;
        }
    }

    function apiPost(path, body) {
        return fetch(path, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(body)
        }).then(r => r.json());
    }

    function apiPostText(path, text) {
        return fetch(path, {
            method: 'POST',
            headers: { 'Content-Type': 'text/plain; charset=utf-8' },
            body: text
        }).then(r => r.json());
    }

    function apiGet(path) {
        return fetch(path).then(r => r.json());
    }

    // ─── Render Data Table ─────────────────────────────────

    function renderTable(data) {
        tablones = data;
        emptyState.style.display = 'none';
        tableWrap.style.display = '';
        fileInfo.style.display = '';

        dataBody.innerHTML = '';
        data.forEach((t, i) => {
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td>${i}</td>
                <td>${t.ts}</td>
                <td>${t.tr}</td>
                <td>${t.p}</td>
                <td>${t.rp}</td>
            `;
            dataBody.appendChild(tr);
        });

        fileInfoText.textContent = `✅ ${data.length} tablones cargados`;
        setStatus(`📂 ${data.length} tablones cargados correctamente`);
    }

    function showEmptyData() {
        tablones = [];
        emptyState.style.display = '';
        tableWrap.style.display = 'none';
        fileInfo.style.display = 'none';
    }

    // ─── Render Results ────────────────────────────────────

    function renderResult(clave, nombre, costo, perm, isOptimal) {
        resultados[clave] = { nombre, costo, perm };
        resultsEmpty.style.display = 'none';
        cardActions.style.display = '';

        // Check if card already exists, update or create
        let card = document.querySelector(`.result-card[data-clave="${clave}"]`);
        const isNew = !card;

        if (!card) {
            card = document.createElement('div');
            card.className = 'result-card';
            card.dataset.clave = clave;
        }

        card.className = 'result-card' + (isOptimal ? ' optimal' : '');
        card.innerHTML = `
            <div class="algo-name">${nombre}</div>
            <div class="algo-cost"><span class="label">Costo:</span> ${costo.toFixed(0)}</div>
            <div class="algo-perm">Pi: &lt; ${perm.join(', ')} &gt;</div>
            ${isOptimal ? '<div class="optimal-badge">✅ Óptimo</div>' : ''}
            ${isOptimal === false ? '<div class="suboptimal-badge">⚠️ Subóptimo</div>' : ''}
        `;

        if (isNew) {
            resultsCont.appendChild(card);
            // Animate in
            requestAnimationFrame(() => card.style.opacity = '1');
            card.style.opacity = '0';
            requestAnimationFrame(() => card.style.transition = 'opacity 0.3s ease');
            requestAnimationFrame(() => card.style.opacity = '1');
        }
    }

    function renderOptimalitySummary(allOptimal) {
        // Remove existing summary
        const old = document.querySelector('.optimality-summary');
        if (old) old.remove();

        const div = document.createElement('div');
        div.className = 'optimality-summary ' + (allOptimal ? 'ok' : 'warn');
        div.textContent = allOptimal
            ? '✅ TODOS los algoritmos coinciden en el mismo costo óptimo'
            : '⚠️ Los algoritmos NO coinciden — revisá las diferencias';
        resultsCont.appendChild(div);
    }

    function renderResultsAll(results) {
        resultsEmpty.style.display = 'none';
        cardActions.style.display = '';
        resultsCont.innerHTML = '';
        resultados = {};

        results.forEach(r => {
            renderResult(r.clave, r.algoritmo, r.costo, r.permutacion, true);
        });

        if (results.length > 1) {
            renderOptimalitySummary(results.every(r => r.costo === results[0].costo));
        }

        setStatus(`✅ ${results.length} algoritmos ejecutados`);
    }

    function clearResults() {
        resultados = {};
        lastAlgo = null;
        resultsCont.innerHTML = '';
        resultsEmpty.style.display = '';
        cardActions.style.display = 'none';
    }

    // ─── File Loading ──────────────────────────────────────

    fileInput.addEventListener('change', async (e) => {
        const file = e.target.files[0];
        if (!file) return;

        try {
            showLoading(true);
            const text = await file.text();
            const res = await apiPostText('/api/load', text);

            if (!res.ok) {
                showError(res.error || 'Error al cargar archivo');
                showEmptyData();
                clearResults();
                return;
            }

            renderTable(res.tablones);
            clearResults();
            setStatus(`📂 ${res.n} tablones cargados desde ${file.name}`);
        } catch (err) {
            showError('Error de conexión: ' + err.message);
        } finally {
            showLoading(false);
            fileInput.value = '';
        }
    });

    // ─── Run Algorithm ─────────────────────────────────────

    document.querySelectorAll('[data-algo]').forEach(btn => {
        btn.addEventListener('click', async () => {
            const algo = btn.dataset.algo;
            if (tablones.length === 0) {
                showError('Cargá un archivo primero');
                return;
            }

            try {
                showLoading(true);
                startTimer();
                const res = await apiPost('/api/run', { algoritmo: algo });

                if (!res.ok) {
                    showError(res.error || 'Error al ejecutar');
                    return;
                }

                if (algo === 'todos') {
                    renderResultsAll(res.resultados);
                    lastAlgo = 'fb'; // default for verify/download
                } else {
                    renderResult(algo, res.algoritmo, res.costo, res.permutacion, true);
                    lastAlgo = algo;
                    // Clear optimality summary if exists
                    const old = document.querySelector('.optimality-summary');
                    if (old) old.remove();
                    setStatus(`✅ ${res.algoritmo}: Costo ${res.costo.toFixed(0)}`);
                }
            } catch (err) {
                showError('Error de conexión: ' + err.message);
            } finally {
                showLoading(false);
                stopTimer();
            }
        });
    });

    // ─── Verify ────────────────────────────────────────────

    $('btnVerify').addEventListener('click', async () => {
        const algo = lastAlgo || Object.keys(resultados)[0];
        if (!algo || !resultados[algo]) {
            showError('Ejecutá un algoritmo primero');
            return;
        }

        try {
            showLoading(true);
            const res = await apiPost('/api/verify', { algoritmo: algo });

            if (!res.ok) {
                showError(res.error || 'Error al verificar');
                return;
            }

            // Build modal content
            let html = `<p style="margin-bottom:12px;color:var(--text-secondary)">
                Verificando: <strong style="color:var(--accent)">${resultados[algo].nombre}</strong>
                — Costo esperado: <strong>${res.costo_esperado.toFixed(0)}</strong></p>`;

            res.pasos.forEach(p => {
                html += `<div class="step-card">
                    <div class="step-title">Paso ${p.paso} — Tablón #${p.indice}</div>
                    <div class="step-detail">
                        ts=${p.ts}, tr=${p.tr}, p=${p.p}, rp=${p.rp} |
                        t=${p.tiempo}<br>
                        <strong>Rama:</strong> <code>${p.rama}</code><br>
                        <strong>Fórmula:</strong> <code>${p.formula}</code>
                    </div>
                    <div class="step-cost">Costo parcial: ${p.costo_paso}</div>
                    <div class="step-cost acum">Costo acumulado: ${p.costo_acum.toFixed(0)}</div>
                </div>`;
            });

            html += `<div class="verify-result ${res.coincide ? 'ok' : 'fail'}">
                ${res.coincide
                    ? `✅ VERIFICACIÓN EXITOSA — Costo calculado: ${res.total.toFixed(0)} = Costo esperado: ${res.costo_esperado.toFixed(0)}`
                    : `❌ ERROR — Costo calculado: ${res.total.toFixed(0)} ≠ Costo esperado: ${res.costo_esperado.toFixed(0)}`
                }
            </div>`;

            modalBody.innerHTML = html;
            verifyModal.style.display = '';
        } catch (err) {
            showError('Error de conexión: ' + err.message);
        } finally {
            showLoading(false);
        }
    });

    // ─── Download ──────────────────────────────────────────

    $('btnDownload').addEventListener('click', async () => {
        const algo = lastAlgo || Object.keys(resultados)[0];
        if (!algo || !resultados[algo]) {
            showError('Ejecutá un algoritmo primero');
            return;
        }

        try {
            const resp = await fetch('/api/download', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ algoritmo: algo })
            });
            const text = await resp.text();

            const blob = new Blob([text], { type: 'text/plain' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `resultado_${algo}.txt`;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);

            setStatus(`💾 Resultado descargado: resultado_${algo}.txt`);
        } catch (err) {
            showError('Error de conexión: ' + err.message);
        }
    });

    // ─── Clear ─────────────────────────────────────────────

    $('btnClear').addEventListener('click', () => {
        clearResults();
        setStatus('🗑️ Resultados limpiados');
    });

    // ─── Modal Controls ────────────────────────────────────

    function closeModal() {
        verifyModal.style.display = 'none';
        modalBody.innerHTML = '';
    }

    modalClose.addEventListener('click', closeModal);
    modalCloseBtn.addEventListener('click', closeModal);
    verifyModal.addEventListener('click', (e) => {
        if (e.target === verifyModal) closeModal();
    });
    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape') closeModal();
    });

    // ─── Initial Status ────────────────────────────────────

    // Check server status on load
    apiGet('/api/status').then(res => {
        if (res.ok) {
            setStatus('💡 Servidor conectado. Cargá un archivo para comenzar.');
            if (res.n > 0) {
                // Server already has data loaded
                renderTable(res.tablones);
            }
        }
    }).catch(() => {
        setStatus('❌ No se pudo conectar con el servidor', true);
    });

})();
