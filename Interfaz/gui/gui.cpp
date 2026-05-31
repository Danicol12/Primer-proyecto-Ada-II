// ============================================================
// GUI — Sistema de Optimizacion de Riego
// Win32 API c/ Visual Styles (ComCtl32 v6)
// ============================================================

// Habilitar estilos visuales modernos de Windows (MSVC)
#if defined(_MSC_VER)
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "../../Objetos/Finca.h"

using namespace std;

// ─── Constants ─────────────────────────────────────────────

#define ID_BTN_LOAD     1001
#define ID_BTN_FB       1002
#define ID_BTN_VORAZ    1003
#define ID_BTN_PD       1004
#define ID_BTN_TODOS    1005
#define ID_BTN_VERIFY   1006
#define ID_BTN_SAVE     1007
#define ID_FILE_OPEN    2001
#define ID_FILE_EXIT    2002
#define ID_HELP_ABOUT   2003
#define ID_LIST_TABLONES 3001
#define ID_EDIT_RESULT  3002
#define ID_STATUSBAR    3003

// ─── Global state ─────────────────────────────────────────

vector<Tablon> g_tablones;
vector<pair<string, pair<vector<int>, double>>> g_resultados;
string g_archivoActual;
HWND g_hList = NULL;
HWND g_hEdit = NULL;
HWND g_hStatus = NULL;
HFONT g_hFontMono = NULL;   // Consolas (resultados)
HFONT g_hFontUI = NULL;     // Segoe UI (botones, labels)
HINSTANCE g_hInst = NULL;

// ─── Forward declarations ─────────────────────────────────

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CrearControles(HWND hwnd);
void CargarArchivo(HWND hwnd);
void LlenarLista();
void AgregarResultado(const string& texto, bool limpiar = false);
void EjecutarAlgoritmo(HWND hwnd, int tipo);
void VerificarCostoGUI(HWND hwnd);
void GuardarSalidaGUI(HWND hwnd);
void ActualizarEstado(HWND hwnd, const char* msg = NULL);

// ─── Entry point ──────────────────────────────────────────

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {
    g_hInst = hInst;

    INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES };
    InitCommonControlsEx(&icc);

    const char CLASS_NAME[] = "RiegoGUI";

    WNDCLASSEX wc = {};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassEx(&wc)) return 0;

    // Fuentes
    g_hFontUI = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    g_hFontMono = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Optimizacion de Riego — ADA",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 960, 720,
                               NULL, NULL, hInst, NULL);
    if (!hwnd) return 0;

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_hFontMono) DeleteObject(g_hFontMono);
    if (g_hFontUI)   DeleteObject(g_hFontUI);
    return 0;
}

// ─── Window procedure ─────────────────────────────────────

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            CrearControles(hwnd);
            break;

        case WM_SIZE: {
            RECT rc; GetClientRect(hwnd, &rc);
            int w = rc.right, h = rc.bottom;
            int margen = 12;
            int y = 10;

            // Status bar
            SendMessage(g_hStatus, WM_SIZE, 0, 0);

            // ── Header ──
            y = 8;

            // ── Load button ──
            HWND hLoad = GetDlgItem(hwnd, ID_BTN_LOAD);
            SetWindowPos(hLoad, NULL, margen, y, 130, 30, SWP_NOZORDER);

            // ── ListView ──
            int listH = (h - 220) * 2 / 5;
            if (listH < 100) listH = 100;
            int listY = y + 40;
            SetWindowPos(g_hList, NULL, margen, listY, w - 2*margen, listH, SWP_NOZORDER);

            // ── Buttons ──
            int btnY = listY + listH + 10;
            int btnW = 95, btnH = 30;
            int gap = 6;
            int totalBtnW = 6 * btnW + 5 * gap;
            int btnStart = max(margen, (w - totalBtnW) / 2);

            const int btns[] = { ID_BTN_FB, ID_BTN_VORAZ, ID_BTN_PD,
                                 ID_BTN_TODOS, ID_BTN_VERIFY, ID_BTN_SAVE };
            for (int i = 0; i < 6; i++) {
                HWND hBtn = GetDlgItem(hwnd, btns[i]);
                SetWindowPos(hBtn, NULL, btnStart + i*(btnW+gap), btnY, btnW, btnH, SWP_NOZORDER);
            }

            // ── Edit / resultados ──
            int editY = btnY + btnH + 10;
            int editH = h - editY - 30;
            if (editH < 80) editH = 80;
            SetWindowPos(g_hEdit, NULL, margen, editY, w - 2*margen, editH, SWP_NOZORDER);
            break;
        }

        case WM_CTLCOLORSTATIC: {
            // Header text color
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(0, 80, 160));
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            switch (id) {
                case ID_FILE_OPEN:
                case ID_BTN_LOAD:
                    CargarArchivo(hwnd);
                    break;
                case ID_FILE_EXIT:
                    DestroyWindow(hwnd);
                    break;
                case ID_HELP_ABOUT:
                    MessageBoxA(hwnd,
                        "Sistema de Optimizacion de Riego — ADA\n\n"
                        "Algoritmos:\n"
                        "  • Fuerza Bruta (roFB)\n"
                        "  • Voraz (roV)\n"
                        "  • Programacion Dinamica (roPD)\n"
                        "  • roD (wrapper)\n\n"
                        "Desarrollado para el curso de ADA\n"
                        "Semestre 6 — Ingenieria de Sistemas",
                        "Acerca de", MB_OK | MB_ICONINFORMATION);
                    break;
                case ID_BTN_FB:    EjecutarAlgoritmo(hwnd, 0); break;
                case ID_BTN_VORAZ: EjecutarAlgoritmo(hwnd, 1); break;
                case ID_BTN_PD:    EjecutarAlgoritmo(hwnd, 2); break;
                case ID_BTN_TODOS: EjecutarAlgoritmo(hwnd, 3); break;
                case ID_BTN_VERIFY: VerificarCostoGUI(hwnd); break;
                case ID_BTN_SAVE:  GuardarSalidaGUI(hwnd); break;
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ─── Crear controles ──────────────────────────────────────

void CrearControles(HWND hwnd) {
    // ── Menu ──
    HMENU hMenu = CreateMenu();
    HMENU hFile = CreatePopupMenu();
    AppendMenu(hFile, MF_STRING, ID_FILE_OPEN, "&Abrir archivo\tCtrl+O");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, ID_FILE_EXIT, "&Salir");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, "&Archivo");

    HMENU hHelp = CreatePopupMenu();
    AppendMenu(hHelp, MF_STRING, ID_HELP_ABOUT, "&Acerca de");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, "&Ayuda");

    SetMenu(hwnd, hMenu);

    // ── Load button ──
    HWND hBtn = CreateWindow("BUTTON", "Cargar archivo",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        12, 10, 130, 30, hwnd, (HMENU)ID_BTN_LOAD, g_hInst, NULL);
    if (g_hFontUI) SendMessage(hBtn, WM_SETFONT, (WPARAM)g_hFontUI, TRUE);

    // ── ListView ──
    g_hList = CreateWindow(WC_LISTVIEWA, "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
        12, 50, 600, 180, hwnd, (HMENU)ID_LIST_TABLONES, g_hInst, NULL);

    LV_COLUMNA lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_RIGHT;

    lvc.pszText = (char*)"#";    lvc.cx = 40;  ListView_InsertColumn(g_hList, 0, &lvc);
    lvc.pszText = (char*)"ts";   lvc.cx = 70;  ListView_InsertColumn(g_hList, 1, &lvc);
    lvc.pszText = (char*)"tr";   lvc.cx = 70;  ListView_InsertColumn(g_hList, 2, &lvc);
    lvc.pszText = (char*)"p";    lvc.cx = 60;  ListView_InsertColumn(g_hList, 3, &lvc);
    lvc.pszText = (char*)"rp";   lvc.cx = 70;  ListView_InsertColumn(g_hList, 4, &lvc);

    ListView_SetExtendedListViewStyle(g_hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    // ── Algorithm buttons ──
    struct { int id; const char* label; } btns[] = {
        { ID_BTN_FB, "FB" }, { ID_BTN_VORAZ, "Voraz" }, { ID_BTN_PD, "PD" },
        { ID_BTN_TODOS, "Todos" }, { ID_BTN_VERIFY, "Verificar" }, { ID_BTN_SAVE, "Guardar" }
    };
    for (auto& b : btns) {
        HWND h = CreateWindow("BUTTON", b.label,
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 95, 30, hwnd, (HMENU)(INT_PTR)b.id, g_hInst, NULL);
        if (g_hFontUI) SendMessage(h, WM_SETFONT, (WPARAM)g_hFontUI, TRUE);
    }

    // ── Results edit ──
    g_hEdit = CreateWindow("EDIT", "Bienvenido.\nCargue un archivo para comenzar.",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
        12, 250, 900, 350, hwnd, (HMENU)ID_EDIT_RESULT, g_hInst, NULL);
    if (g_hFontMono) SendMessage(g_hEdit, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

    // ── Status bar ──
    g_hStatus = CreateWindow(STATUSCLASSNAME, "Listo. Cargue un archivo .txt con los datos de la finca.",
                             WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                             0, 0, 0, 0, hwnd, (HMENU)ID_STATUSBAR, g_hInst, NULL);
}

// ─── File loading ─────────────────────────────────────────

void CargarArchivo(HWND hwnd) {
    OPENFILENAMEA ofn = {};
    char szFile[260] = {};

    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hwnd;
    ofn.lpstrFile       = szFile;
    ofn.nMaxFile        = sizeof(szFile);
    ofn.lpstrFilter     = "Archivos de texto (*.txt)\0*.txt\0Todos los archivos (*.*)\0*.*\0";
    ofn.nFilterIndex    = 1;
    ofn.lpstrTitle      = "Seleccionar archivo de finca";
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileNameA(&ofn)) {
        try {
            ifstream archivo(ofn.lpstrFile);
            if (!archivo.is_open()) {
                MessageBoxA(hwnd, "No se pudo abrir el archivo.", "Error", MB_OK | MB_ICONERROR);
                return;
            }

            int n;
            string linea;
            if (!getline(archivo, linea)) {
                MessageBoxA(hwnd, "El archivo esta vacio.", "Error", MB_OK | MB_ICONERROR);
                return;
            }

            while (!linea.empty() && (linea[0] < '0' || linea[0] > '9'))
                linea.erase(0, 1);

            stringstream(linea) >> n;
            if (n <= 0) {
                MessageBoxA(hwnd, "Numero de tablones invalido.", "Error", MB_OK | MB_ICONERROR);
                return;
            }

            g_tablones.clear();
            int leidos = 0;
            while (getline(archivo, linea) && leidos < n) {
                if (linea.empty()) continue;
                for (char& c : linea) if (c == ',') c = ' ';

                int ts, tr, p, rp;
                stringstream sl(linea);
                if (sl >> ts >> tr >> p >> rp) {
                    g_tablones.emplace_back(ts, tr, p, rp);
                    leidos++;
                }
            }
            archivo.close();

            if (g_tablones.size() != (size_t)n) {
                char buf[128];
                snprintf(buf, sizeof(buf),
                    "Se esperaban %d tablones, se leyeron %zu.", n, g_tablones.size());
                MessageBoxA(hwnd, buf, "Advertencia", MB_OK | MB_ICONWARNING);
            }

            g_archivoActual = ofn.lpstrFile;
            g_resultados.clear();
            LlenarLista();

            char status[512];
            snprintf(status, sizeof(status),
                "OK: %zu tablones cargados desde %s", g_tablones.size(), ofn.lpstrFile);
            ActualizarEstado(hwnd, status);

            SetWindowTextA(g_hEdit, "Datos cargados correctamente.\r\nSeleccione un algoritmo para ejecutar.");
        }
        catch (const exception& e) {
            MessageBoxA(hwnd, e.what(), "Error", MB_OK | MB_ICONERROR);
        }
    }
}

void LlenarLista() {
    ListView_DeleteAllItems(g_hList);
    for (size_t i = 0; i < g_tablones.size(); i++) {
        char buf[16];
        LV_ITEMA item = {};
        item.mask = LVIF_TEXT;

        snprintf(buf, sizeof(buf), "%zu", i);
        item.pszText = buf;
        item.iItem = (int)i;
        ListView_InsertItem(g_hList, &item);

        auto setCol = [&](int col, int val) {
            snprintf(buf, sizeof(buf), "%d", val);
            ListView_SetItemText(g_hList, (int)i, col, buf);
        };
        setCol(1, g_tablones[i].getTiempoDeSupervivencia());
        setCol(2, g_tablones[i].getTiempoDeRegado());
        setCol(3, g_tablones[i].getPrioridad());
        setCol(4, g_tablones[i].getTiempoDeRiegoPerfecto());
    }
}

// ─── Result display ───────────────────────────────────────

void AgregarResultado(const string& texto, bool limpiar) {
    if (limpiar) {
        SetWindowTextA(g_hEdit, texto.c_str());
    } else {
        int len = GetWindowTextLengthA(g_hEdit);
        SendMessageA(g_hEdit, EM_SETSEL, len, len);
        string s = texto + "\r\n";
        SendMessageA(g_hEdit, EM_REPLACESEL, 0, (LPARAM)s.c_str());
    }
}

void MostrarResultado(const string& nombre, const pair<vector<int>, double>& res) {
    char buf[1024];
    string txt = "--- " + nombre + " ---\r\n";
    snprintf(buf, sizeof(buf), "  Costo (CR): %.0f\r\n", res.second);
    txt += buf;
    txt += "  Programacion (Pi): < ";
    for (size_t i = 0; i < res.first.size(); i++) {
        txt += to_string(res.first[i]);
        if (i + 1 < res.first.size()) txt += ", ";
    }
    txt += " >\r\n\r\n";
    AgregarResultado(txt);
}

void ActualizarEstado(HWND, const char* msg) {
    if (msg) {
        SetWindowTextA(g_hStatus, msg);
    } else {
        char buf[512];
        snprintf(buf, sizeof(buf),
            "Archivo: %s  |  Tablones: %zu",
            g_archivoActual.empty() ? "(ninguno)" : g_archivoActual.c_str(),
            g_tablones.size());
        SetWindowTextA(g_hStatus, buf);
    }
}

// ─── Algorithm execution ──────────────────────────────────

void EjecutarAlgoritmo(HWND hwnd, int tipo) {
    if (g_tablones.empty()) {
        MessageBoxA(hwnd, "Cargue un archivo primero.", "Sin datos", MB_OK | MB_ICONWARNING);
        return;
    }

    Finca finca(g_tablones);
    SetWindowTextA(g_hStatus, "Procesando...");
    UpdateWindow(hwnd);
    AgregarResultado("", true);

    if (tipo == 3) {
        // Todos
        g_resultados.clear();
        auto rFB  = finca.roFB(); g_resultados.push_back({"Fuerza Bruta (roFB)", rFB});
        MostrarResultado("Fuerza Bruta (roFB)", rFB);

        auto rV   = finca.roV();  g_resultados.push_back({"Voraz (roV)", rV});
        MostrarResultado("Voraz (roV)", rV);

        auto rPD  = finca.roPD(); g_resultados.push_back({"PD (roPD)", rPD});
        MostrarResultado("Programacion Dinamica (roPD)", rPD);

        auto rD   = finca.roD();  g_resultados.push_back({"roD (wrapper)", rD});
        MostrarResultado("roD (wrapper)", rD);

        // Resumen
        string resumen = "========== RESUMEN COMPARATIVO ==========\r\n";
        char buf[128];
        for (auto& [nom, res] : g_resultados) {
            snprintf(buf, sizeof(buf), "  %-30s  Costo: %.0f\r\n", nom.c_str(), res.second);
            resumen += buf;
        }
        AgregarResultado(resumen);

        // Optimalidad
        if (g_resultados.size() >= 2) {
            double ref = g_resultados[0].second.second;
            bool ok = true;
            for (size_t i = 1; i < g_resultados.size(); i++)
                if (g_resultados[i].second.second != ref) { ok = false; break; }

            if (ok)
                AgregarResultado("[OK] TODOS los algoritmos coinciden en el mismo costo.");
            else
                AgregarResultado("[WARN] Los algoritmos NO coinciden. Revise diferencias arriba.");
        }
    } else {
        pair<vector<int>, double> res;
        string nombre;
        switch (tipo) {
            case 0: res = finca.roFB(); nombre = "Fuerza Bruta (roFB)"; break;
            case 1: res = finca.roV();  nombre = "Voraz (roV)";        break;
            case 2: res = finca.roPD(); nombre = "PD (roPD)";          break;
        }
        g_resultados.push_back({nombre, res});
        MostrarResultado(nombre, res);
    }

    ActualizarEstado(hwnd);
}

// ─── Verify cost step by step ─────────────────────────────

void VerificarCostoGUI(HWND hwnd) {
    if (g_resultados.empty()) {
        MessageBoxA(hwnd, "No hay resultados.\nEjecute un algoritmo primero (FB, Voraz o PD).",
                    "Verificar", MB_OK | MB_ICONWARNING);
        return;
    }

    auto& [nombre, res] = g_resultados.back();

    AgregarResultado("", true);
    char buf[4096];
    string txt = "========== VERIFICACION DE COSTO ==========\r\n\r\n";
    txt += "Resultado: " + nombre + "\r\n";
    txt += "Permutacion: < ";
    for (size_t i = 0; i < res.first.size(); i++) {
        txt += to_string(res.first[i]);
        if (i + 1 < res.first.size()) txt += ", ";
    }
    txt += " >\r\n\r\n";
    AgregarResultado(txt);

    double costoAcum = 0;
    int tiempo = 0;

    for (size_t paso = 0; paso < res.first.size(); paso++) {
        int idx = res.first[paso];
        const Tablon& t = g_tablones[idx];

        int ts = t.getTiempoDeSupervivencia();
        int tr = t.getTiempoDeRegado();
        int p  = t.getPrioridad();
        int rp = t.getTiempoDeRiegoPerfecto();

        snprintf(buf, sizeof(buf),
            "Paso %zu: Tablon %d (ts=%d, tr=%d, p=%d, rp=%d)\r\n  t = %d\r\n",
            paso+1, idx, ts, tr, p, rp, tiempo);
        AgregarResultado(buf);

        int costoPaso = 0;
        if (rp == tiempo) {
            costoPaso = ts - (tiempo + tr);
            snprintf(buf, sizeof(buf),
                "  -> rp == t : Costo = ts - (t + tr)\r\n"
                "     = %d - (%d + %d) = %d\r\n",
                ts, tiempo, tr, costoPaso);
        } else if ((ts - tr) >= tiempo) {
            costoPaso = 2 * (ts - (tiempo + tr));
            snprintf(buf, sizeof(buf),
                "  -> (ts - tr) >= t : Costo = 2 * (ts - (t + tr))\r\n"
                "     = 2 * (%d - (%d + %d)) = %d\r\n",
                ts, tiempo, tr, costoPaso);
        } else {
            costoPaso = 2 * p * ((tiempo + tr) - ts);
            snprintf(buf, sizeof(buf),
                "  -> (ts - tr) < t : Costo = 2 * p * ((t + tr) - ts)\r\n"
                "     = 2 * %d * ((%d + %d) - %d) = %d\r\n",
                p, tiempo, tr, ts, costoPaso);
        }
        AgregarResultado(buf);

        costoAcum += costoPaso;
        tiempo += tr;

        snprintf(buf, sizeof(buf),
            "  Costo acumulado: %.0f\r\n  t += tr -> t = %d\r\n\r\n",
            costoAcum, tiempo);
        AgregarResultado(buf);
    }

    if (costoAcum == res.second) {
        snprintf(buf, sizeof(buf),
            "========== VERIFICACION EXITOSA ==========\r\n"
            "Costo calculado: %.0f  =  Costo esperado: %.0f\r\n"
            "Los valores coinciden correctamente.\r\n",
            costoAcum, res.second);
    } else {
        snprintf(buf, sizeof(buf),
            "========== ERROR ==========\r\n"
            "Costo calculado: %.0f  !=  Costo esperado: %.0f\r\n"
            "Revise la implementacion del algoritmo.\r\n",
            costoAcum, res.second);
    }
    AgregarResultado(buf);
    ActualizarEstado(hwnd);
}

// ─── Save output ──────────────────────────────────────────

void GuardarSalidaGUI(HWND hwnd) {
    if (g_resultados.empty()) {
        MessageBoxA(hwnd, "No hay resultados.\nEjecute un algoritmo primero.",
                    "Guardar", MB_OK | MB_ICONWARNING);
        return;
    }

    auto& [nombre, res] = g_resultados.back();
    OPENFILENAMEA ofn = {};
    char szFile[260] = "salida.txt";

    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hwnd;
    ofn.lpstrFile       = szFile;
    ofn.nMaxFile        = sizeof(szFile);
    ofn.lpstrFilter     = "Archivos de texto (*.txt)\0*.txt\0\0";
    ofn.lpstrDefExt     = "txt";
    ofn.lpstrTitle      = "Guardar resultado como...";
    ofn.Flags           = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

    if (GetSaveFileNameA(&ofn)) {
        FILE* f = fopen(ofn.lpstrFile, "w");
        if (!f) {
            MessageBoxA(hwnd, "No se pudo crear el archivo.", "Error", MB_OK | MB_ICONERROR);
            return;
        }
        // Formato 3.4.2
        fprintf(f, "%.0f\n", res.second);
        for (int idx : res.first)
            fprintf(f, "%d\n", idx);
        fclose(f);

        char buf[256];
        snprintf(buf, sizeof(buf), "Resultado guardado correctamente en:\n%s", ofn.lpstrFile);
        MessageBoxA(hwnd, buf, "Guardado", MB_OK | MB_ICONINFORMATION);
        ActualizarEstado(hwnd, buf);
    }
}
