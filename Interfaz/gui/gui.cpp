// ============================================================
// GUI — Sistema de Optimizacion de Riego
// Win32 API c/ Visual Styles (ComCtl32 v6)
// ============================================================

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

// ─── IDs de controles ──────────────────────────────────────
#define ID_BTN_RESOLVER   1001
#define ID_BTN_FB         1002
#define ID_BTN_VORAZ      1003
#define ID_BTN_PD         1004
#define ID_BTN_TODOS      1005
#define ID_BTN_RELOAD     1008

#define ID_FILE_EXIT      2002
#define ID_HELP_ABOUT     2003

#define ID_LIST_ARCHIVOS  3000   // ListBox con los archivos .txt
#define ID_LIST_TABLONES  3001   // ListView con los tablones del archivo seleccionado
#define ID_EDIT_RESULT    3002
#define ID_STATUSBAR      3003
#define ID_EDIT_PREVIEW   3004   // Vista previa del contenido crudo del archivo

#define ID_RADIO_FB       4001
#define ID_RADIO_VORAZ    4002
#define ID_RADIO_PD       4003
#define ID_RADIO_TODOS    4004
#define ID_RADIO_PEOR     4005

// ─── Estado global ─────────────────────────────────────────
vector<Tablon>  g_tablones;
vector<pair<string, pair<vector<int>, double>>> g_resultados;
string          g_archivoActual;      // Ruta completa del archivo seleccionado
string          g_nombreArchivo;      // Solo el nombre base (sin extension)
int             g_algoritmoSel = 0;   // 0=FB, 1=Voraz, 2=PD, 3=Todos, 4=Peor(FB)

HWND g_hListArchivos = NULL;
HWND g_hListTablones = NULL;
HWND g_hEditPreview  = NULL;
HWND g_hEdit         = NULL;
HWND g_hStatus       = NULL;
HFONT g_hFontMono    = NULL;
HFONT g_hFontUI      = NULL;
HFONT g_hFontUIBold  = NULL;
HINSTANCE g_hInst    = NULL;

// Ruta base (relativa al ejecutable) para las carpetas de datos
// Se calcula al iniciar
char g_dirEntrantes[MAX_PATH] = {};
char g_dirSalidas[MAX_PATH]   = {};

// ─── Forward declarations ─────────────────────────────────
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CrearControles(HWND hwnd);
void ComputarRutas();
void CargarListaArchivos(HWND hwnd);
void SeleccionarArchivo(HWND hwnd, const string& ruta, const string& nombre);
void LlenarListaTablones();
void MostrarPreviewArchivo(const string& ruta);
void AgregarResultado(const string& texto, bool limpiar = false);
void MostrarResultado(const string& nombre, const pair<vector<int>, double>& res);
void EjecutarAlgoritmo(HWND hwnd);
void GuardarSalidaAutomatica(HWND hwnd, const string& nombre,
                              const pair<vector<int>, double>& res);
void ActualizarEstado(HWND hwnd, const char* msg = NULL);
string BaseName(const string& ruta);
string SinExtension(const string& nombre);

// ─── Entry point ──────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {
    g_hInst = hInst;

    INITCOMMONCONTROLSEX icc = {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES
    };
    InitCommonControlsEx(&icc);

    ComputarRutas();

    const char CLASS_NAME[] = "RiegoGUI";
    WNDCLASSEX wc = {};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = CLASS_NAME;
    if (!RegisterClassEx(&wc)) return 0;

    g_hFontUI = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    g_hFontUIBold = CreateFont(15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    g_hFontMono = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    HWND hwnd = CreateWindowEx(0, CLASS_NAME,
        "Optimizacion de Riego — ADA II",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 740,
        NULL, NULL, hInst, NULL);
    if (!hwnd) return 0;

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_hFontMono)   DeleteObject(g_hFontMono);
    if (g_hFontUI)     DeleteObject(g_hFontUI);
    if (g_hFontUIBold) DeleteObject(g_hFontUIBold);
    return 0;
}

// ─── Calcular rutas de Datos/Entrantes y Datos/Salidas ────
void ComputarRutas() {
    char exePath[MAX_PATH] = {};
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // Subir 3 niveles desde el exe: gui/ -> Interfaz/ -> Proyecto/
    // Luego concatenar Datos\Entrantes y Datos\Salidas
    string path(exePath);
    // quitar nombre del exe (el exe ya está en la raíz del proyecto)
    size_t last = path.find_last_of("\\/");
    if (last != string::npos) path = path.substr(0, last);

    snprintf(g_dirEntrantes, MAX_PATH, "%s\\Datos\\Entrantes", path.c_str());
    snprintf(g_dirSalidas,   MAX_PATH, "%s\\Datos\\Salidas",   path.c_str());

    // Crear carpetas si no existen
    CreateDirectoryA(g_dirEntrantes, NULL);
    CreateDirectoryA(g_dirSalidas,   NULL);
}

// ─── Window procedure ─────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            CrearControles(hwnd);
            CargarListaArchivos(hwnd);
            break;

        case WM_SIZE: {
            RECT rc; GetClientRect(hwnd, &rc);
            int W = rc.right, H = rc.bottom;
            const int M = 10;          // margen
            const int PANEL_W = 220;   // ancho panel izquierdo
            const int GAP = 8;

            // Status bar
            SendMessage(g_hStatus, WM_SIZE, 0, 0);
            RECT rs; GetWindowRect(g_hStatus, &rs);
            int statusH = rs.bottom - rs.top;

            int innerH = H - statusH - M;

            // ── Panel izquierdo: lista de archivos ──
            // Label "Archivos de entrada:"
            HWND hLblA = GetDlgItem(hwnd, 5001);
            SetWindowPos(hLblA, NULL, M, M, PANEL_W, 18, SWP_NOZORDER);

            // Botón recargar
            HWND hReload = GetDlgItem(hwnd, ID_BTN_RELOAD);
            SetWindowPos(hReload, NULL, M, M + 22, PANEL_W, 24, SWP_NOZORDER);

            // Lista de archivos
            int listArchH = (innerH - 22 - 24 - GAP * 2) / 2;
            if (listArchH < 80) listArchH = 80;
            SetWindowPos(g_hListArchivos, NULL, M, M + 22 + 24 + GAP,
                         PANEL_W, listArchH, SWP_NOZORDER);

            // Label "Vista previa:"
            HWND hLblP = GetDlgItem(hwnd, 5002);
            int previewY = M + 22 + 24 + GAP + listArchH + GAP;
            SetWindowPos(hLblP, NULL, M, previewY, PANEL_W, 18, SWP_NOZORDER);

            // Edit preview (contenido crudo)
            int previewH = innerH - previewY - 18 - GAP - M;
            if (previewH < 60) previewH = 60;
            SetWindowPos(g_hEditPreview, NULL, M, previewY + 20,
                         PANEL_W, previewH, SWP_NOZORDER);

            // ── Panel derecho ──
            int rightX = M + PANEL_W + GAP;
            int rightW = W - rightX - M;

            // Label "Tablones cargados:"
            HWND hLblT = GetDlgItem(hwnd, 5003);
            SetWindowPos(hLblT, NULL, rightX, M, rightW, 18, SWP_NOZORDER);

            // ListView tablones
            int lvH = (innerH - 18 - GAP) * 2 / 5;
            if (lvH < 90) lvH = 90;
            SetWindowPos(g_hListTablones, NULL, rightX, M + 20,
                         rightW, lvH, SWP_NOZORDER);

            // ── Fila de algoritmos + botón Resolver ──
            int algoY = M + 20 + lvH + GAP;
            // Label "Algoritmo:"
            HWND hLblAlg = GetDlgItem(hwnd, 5004);
            SetWindowPos(hLblAlg, NULL, rightX, algoY + 4, 80, 20, SWP_NOZORDER);

            // Radio buttons
            const int radios[] = { ID_RADIO_FB, ID_RADIO_VORAZ, ID_RADIO_PD, ID_RADIO_TODOS, ID_RADIO_PEOR };
            const int radioW   = 72;
            int rx = rightX + 85;
            for (int i = 0; i < 5; i++) {
                HWND hR = GetDlgItem(hwnd, radios[i]);
                SetWindowPos(hR, NULL, rx, algoY + 2, radioW, 24, SWP_NOZORDER);
                rx += radioW + 4;
            }

            // Botón Resolver
            HWND hResol  = GetDlgItem(hwnd, ID_BTN_RESOLVER);
            int resolX   = rightX + rightW - 110;
            SetWindowPos(hResol,  NULL, resolX,  algoY, 105, 28, SWP_NOZORDER);

            // ── Edit resultados ──
            int editY = algoY + 28 + GAP;
            int editH = innerH - editY - M;
            if (editH < 80) editH = 80;
            SetWindowPos(g_hEdit, NULL, rightX, editY,
                         rightW, editH, SWP_NOZORDER);
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(30, 60, 120));
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }

        case WM_COMMAND: {
            int id   = LOWORD(wParam);
            int code = HIWORD(wParam);

            // Selección en ListBox de archivos
            if (id == ID_LIST_ARCHIVOS && code == LBN_SELCHANGE) {
                int sel = (int)SendMessageA(g_hListArchivos, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    char buf[MAX_PATH] = {};
                    SendMessageA(g_hListArchivos, LB_GETTEXT, sel, (LPARAM)buf);
                    string nombreBase(buf);
                    string rutaCompleta = string(g_dirEntrantes) + "\\" + nombreBase;
                    SeleccionarArchivo(hwnd, rutaCompleta, nombreBase);
                }
                break;
            }

            switch (id) {
                case ID_BTN_RELOAD:
                    CargarListaArchivos(hwnd);
                    break;
                case ID_FILE_EXIT:
                    DestroyWindow(hwnd);
                    break;
                case ID_HELP_ABOUT:
                    MessageBoxA(hwnd,
                        "Sistema de Optimizacion de Riego — ADA II\n\n"
                        "Algoritmos disponibles:\n"
                        "  • Fuerza Bruta (FB) — O(n!)\n"
                        "  • Voraz           — O(n log n)\n"
                        "  • Prog. Dinamica  — O(n^2 * 2^n)\n"
                        "  • Todos           — Ejecuta los tres\n\n"
                        "Los archivos de entrada van en:  Datos\\Entrantes\\\n"
                        "Los resultados se guardan en:    Datos\\Salidas\\",
                        "Acerca de", MB_OK | MB_ICONINFORMATION);
                    break;
                case ID_RADIO_FB:    g_algoritmoSel = 0; break;
                case ID_RADIO_VORAZ: g_algoritmoSel = 1; break;
                case ID_RADIO_PD:    g_algoritmoSel = 2; break;
                case ID_RADIO_TODOS: g_algoritmoSel = 3; break;
                case ID_RADIO_PEOR:  g_algoritmoSel = 4; break;
                case ID_BTN_RESOLVER: EjecutarAlgoritmo(hwnd); break;
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
    AppendMenu(hFile, MF_STRING, ID_FILE_EXIT,  "&Salir");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, "&Archivo");
    HMENU hHelp = CreatePopupMenu();
    AppendMenu(hHelp, MF_STRING, ID_HELP_ABOUT, "&Acerca de");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, "&Ayuda");
    SetMenu(hwnd, hMenu);

    // ── Labels ──
    auto mkLabel = [&](int id, const char* txt, HFONT font) {
        HWND h = CreateWindow("STATIC", txt,
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            0, 0, 10, 10, hwnd, (HMENU)(INT_PTR)id, g_hInst, NULL);
        if (font) SendMessage(h, WM_SETFONT, (WPARAM)font, TRUE);
        return h;
    };
    mkLabel(5001, "Archivos de entrada:",    g_hFontUIBold);
    mkLabel(5002, "Vista previa:",            g_hFontUIBold);
    mkLabel(5003, "Tablones del archivo:",    g_hFontUIBold);
    mkLabel(5004, "Algoritmo:",               g_hFontUIBold);

    // ── Botón Recargar lista ──
    {
        HWND h = CreateWindow("BUTTON", "↺ Recargar lista",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 10, 10, hwnd, (HMENU)ID_BTN_RELOAD, g_hInst, NULL);
        SendMessage(h, WM_SETFONT, (WPARAM)g_hFontUI, TRUE);
    }

    // ── ListBox con los archivos ──
    g_hListArchivos = CreateWindow("LISTBOX", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_SORT,
        0, 0, 10, 10, hwnd, (HMENU)ID_LIST_ARCHIVOS, g_hInst, NULL);
    SendMessage(g_hListArchivos, WM_SETFONT, (WPARAM)g_hFontUI, TRUE);

    // ── Edit preview ──
    g_hEditPreview = CreateWindow("EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY |
        ES_AUTOVSCROLL | WS_VSCROLL,
        0, 0, 10, 10, hwnd, (HMENU)ID_EDIT_PREVIEW, g_hInst, NULL);
    SendMessage(g_hEditPreview, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

    // ── ListView tablones ──
    g_hListTablones = CreateWindow(WC_LISTVIEWA, "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
        0, 0, 10, 10, hwnd, (HMENU)ID_LIST_TABLONES, g_hInst, NULL);
    ListView_SetExtendedListViewStyle(g_hListTablones,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    LV_COLUMNA lvc = {};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
    lvc.fmt  = LVCFMT_RIGHT;
    lvc.pszText = (char*)"#";   lvc.cx = 45;  ListView_InsertColumn(g_hListTablones, 0, &lvc);
    lvc.pszText = (char*)"ts (Supervivencia)"; lvc.cx = 140; ListView_InsertColumn(g_hListTablones, 1, &lvc);
    lvc.pszText = (char*)"tr (Regado)";        lvc.cx = 110; ListView_InsertColumn(g_hListTablones, 2, &lvc);
    lvc.pszText = (char*)"p (Prioridad)";      lvc.cx = 110; ListView_InsertColumn(g_hListTablones, 3, &lvc);
    lvc.pszText = (char*)"rp (Riego Perfecto)";lvc.cx = 140; ListView_InsertColumn(g_hListTablones, 4, &lvc);

    // ── Radio buttons de algoritmo ──
    struct { int id; const char* lbl; DWORD extra; } radios[] = {
        { ID_RADIO_FB,    "FB",    WS_GROUP | BS_AUTORADIOBUTTON },
        { ID_RADIO_VORAZ, "Voraz", BS_AUTORADIOBUTTON },
        { ID_RADIO_PD,    "PD",    BS_AUTORADIOBUTTON },
        { ID_RADIO_TODOS, "Todos", BS_AUTORADIOBUTTON },
        { ID_RADIO_PEOR,  "Peor",  BS_AUTORADIOBUTTON },
    };
    for (auto& r : radios) {
        HWND h = CreateWindow("BUTTON", r.lbl,
            WS_CHILD | WS_VISIBLE | r.extra,
            0, 0, 10, 10, hwnd, (HMENU)(INT_PTR)r.id, g_hInst, NULL);
        SendMessage(h, WM_SETFONT, (WPARAM)g_hFontUI, TRUE);
    }
    // Marcar FB por defecto
    SendMessage(GetDlgItem(hwnd, ID_RADIO_FB), BM_SETCHECK, BST_CHECKED, 0);

    // ── Botón Resolver ──
    {
        HWND h = CreateWindow("BUTTON", "▶  Resolver",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
            0, 0, 10, 10, hwnd, (HMENU)ID_BTN_RESOLVER, g_hInst, NULL);
        SendMessage(h, WM_SETFONT, (WPARAM)g_hFontUIBold, TRUE);
    }

    // ── Edit resultados ──
    g_hEdit = CreateWindow("EDIT",
        "Bienvenido.\r\nSeleccione un archivo de la lista, elija un algoritmo y presione Resolver.",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY |
        ES_AUTOVSCROLL | WS_VSCROLL,
        0, 0, 10, 10, hwnd, (HMENU)ID_EDIT_RESULT, g_hInst, NULL);
    SendMessage(g_hEdit, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

    // ── Status bar ──
    g_hStatus = CreateWindow(STATUSCLASSNAME,
        "Listo. Seleccione un archivo de la lista izquierda.",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd, (HMENU)ID_STATUSBAR, g_hInst, NULL);
}

// ─── Cargar lista de archivos de Datos/Entrantes ──────────
void CargarListaArchivos(HWND hwnd) {
    SendMessageA(g_hListArchivos, LB_RESETCONTENT, 0, 0);

    string patron = string(g_dirEntrantes) + "\\*.txt";
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(patron.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        char msg[MAX_PATH + 128];
        snprintf(msg, sizeof(msg),
            "No se encontraron archivos .txt en:\n%s", g_dirEntrantes);
        SetWindowTextA(g_hEditPreview, msg);
        ActualizarEstado(hwnd, "Sin archivos en Datos\\Entrantes");
        return;
    }
    int cnt = 0;
    do {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            SendMessageA(g_hListArchivos, LB_ADDSTRING, 0, (LPARAM)ffd.cFileName);
            cnt++;
        }
    } while (FindNextFileA(hFind, &ffd));
    FindClose(hFind);

    char status[256];
    snprintf(status, sizeof(status), "%d archivo(s) encontrado(s) en Datos\\Entrantes", cnt);
    ActualizarEstado(hwnd, status);
}

// ─── Seleccionar y cargar un archivo de la lista ──────────
void SeleccionarArchivo(HWND hwnd, const string& ruta, const string& nombre) {
    // Leer el archivo
    ifstream archivo(ruta);
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

    // Limpiar BOM u otros caracteres no numericos al inicio
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
        snprintf(buf, sizeof(buf), "Se esperaban %d tablones, se leyeron %zu.", n, g_tablones.size());
        MessageBoxA(hwnd, buf, "Advertencia", MB_OK | MB_ICONWARNING);
    }

    g_archivoActual = ruta;
    g_nombreArchivo = SinExtension(nombre);
    g_resultados.clear();

    LlenarListaTablones();
    MostrarPreviewArchivo(ruta);

    char status[512];
    snprintf(status, sizeof(status),
        "Cargado: %s  |  %zu tablones", nombre.c_str(), g_tablones.size());
    ActualizarEstado(hwnd, status);

    SetWindowTextA(g_hEdit,
        "Archivo cargado. Seleccione un algoritmo y presione Resolver.");
}

// ─── Mostrar contenido crudo del archivo en el preview ────
void MostrarPreviewArchivo(const string& ruta) {
    ifstream f(ruta);
    if (!f.is_open()) {
        SetWindowTextA(g_hEditPreview, "(No se pudo leer el archivo)");
        return;
    }
    string contenido, linea;
    while (getline(f, linea)) {
        contenido += linea + "\r\n";
    }
    f.close();
    SetWindowTextA(g_hEditPreview, contenido.c_str());
}

// ─── Llenar ListView con los tablones del archivo ─────────
void LlenarListaTablones() {
    ListView_DeleteAllItems(g_hListTablones);
    for (size_t i = 0; i < g_tablones.size(); i++) {
        char buf[32];
        LV_ITEMA item = {};
        item.mask    = LVIF_TEXT;
        item.iItem   = (int)i;
        snprintf(buf, sizeof(buf), "%zu", i);
        item.pszText = buf;
        ListView_InsertItem(g_hListTablones, &item);

        auto setCol = [&](int col, int val) {
            snprintf(buf, sizeof(buf), "%d", val);
            ListView_SetItemText(g_hListTablones, (int)i, col, buf);
        };
        setCol(1, g_tablones[i].getTiempoDeSupervivencia());
        setCol(2, g_tablones[i].getTiempoDeRegado());
        setCol(3, g_tablones[i].getPrioridad());
        setCol(4, g_tablones[i].getTiempoDeRiegoPerfecto());
    }
}

// ─── Agregar texto al panel de resultados ─────────────────
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
    txt += "  Orden de riego (Pi): < ";
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

// ─── Guardar salida automáticamente en Datos/Salidas ──────
void GuardarSalidaAutomatica(HWND hwnd, const string& nombre,
                              const pair<vector<int>, double>& res) {
    // Nombre: <archivo>_<algoritmo>.txt
    char rutaSalida[MAX_PATH];
    snprintf(rutaSalida, MAX_PATH, "%s\\%s_%s.txt",
             g_dirSalidas, g_nombreArchivo.c_str(), nombre.c_str());

    FILE* f = fopen(rutaSalida, "w");
    if (!f) {
        char err[MAX_PATH + 64];
        snprintf(err, sizeof(err), "No se pudo crear el archivo:\n%s", rutaSalida);
        MessageBoxA(hwnd, err, "Error al guardar", MB_OK | MB_ICONERROR);
        return;
    }
    // Formato 3.4.2: primera linea = Costo, luego cada pi en su linea
    fprintf(f, "%.0f\n", res.second);
    for (int idx : res.first)
        fprintf(f, "%d\n", idx);
    fclose(f);

    char msg[MAX_PATH + 128];
    snprintf(msg, sizeof(msg), "Resultado guardado en: Datos\\Salidas\\%s_%s.txt",
             g_nombreArchivo.c_str(), nombre.c_str());
    ActualizarEstado(hwnd, msg);
}

// ─── Ejecutar algoritmo seleccionado ──────────────────────
void EjecutarAlgoritmo(HWND hwnd) {
    if (g_tablones.empty()) {
        MessageBoxA(hwnd, "Seleccione un archivo de entrada primero.",
                    "Sin datos", MB_OK | MB_ICONWARNING);
        return;
    }

    Finca finca(g_tablones);
    SetWindowTextA(g_hStatus, "Procesando...");
    UpdateWindow(hwnd);
    AgregarResultado("", true);
    g_resultados.clear();

    if (g_algoritmoSel == 3) {
        // Todos los algoritmos
        auto rFB = finca.roFB();
        g_resultados.push_back({"FB",    rFB});
        MostrarResultado("Fuerza Bruta (roFB)", rFB);
        GuardarSalidaAutomatica(hwnd, "FB", rFB);

        auto rV  = finca.roV();
        g_resultados.push_back({"Voraz", rV});
        MostrarResultado("Voraz (roV)", rV);
        GuardarSalidaAutomatica(hwnd, "Voraz", rV);

        auto rPD = finca.roPD();
        g_resultados.push_back({"PD",    rPD});
        MostrarResultado("Programacion Dinamica (roPD)", rPD);
        GuardarSalidaAutomatica(hwnd, "PD", rPD);

        // Resumen comparativo
        string resumen = "========== RESUMEN COMPARATIVO ==========\r\n";
        char buf[128];
        for (auto& [nom, res] : g_resultados) {
            snprintf(buf, sizeof(buf), "  %-10s  Costo: %.0f\r\n", nom.c_str(), res.second);
            resumen += buf;
        }
        AgregarResultado(resumen);

        // Verificar optimalidad FB vs PD
        if (g_resultados.size() >= 2) {
            double costoFB = g_resultados[0].second.second;
            double costoPD = g_resultados[2].second.second;
            double costoV  = g_resultados[1].second.second;
            string check = "";
            check += (costoFB == costoPD)
                ? "[OK] FB y PD coinciden.\r\n"
                : "[WARN] FB y PD NO coinciden. Revise la implementacion.\r\n";
            check += (costoV == costoFB)
                ? "[OK] Voraz tambien es optimo.\r\n"
                : "[INFO] Voraz no es optimo para este caso.\r\n";
            AgregarResultado(check);
        }
    } else {
        pair<vector<int>, double> res;
        string nombre, etiqueta;
        switch (g_algoritmoSel) {
            case 0: res = finca.roFB();      nombre = "FB";    etiqueta = "Fuerza Bruta (roFB)";      break;
            case 1: res = finca.roV();       nombre = "Voraz"; etiqueta = "Voraz (roV)";              break;
            case 2: res = finca.roPD();      nombre = "PD";    etiqueta = "Prog. Dinamica (roPD)";    break;
            case 4: res = finca.roFB_peor(); nombre = "Peor";  etiqueta = "Peor Costo (roFB_peor)";  break;
        }
        g_resultados.push_back({nombre, res});
        MostrarResultado(etiqueta, res);
        GuardarSalidaAutomatica(hwnd, nombre, res);
    }
}


// ─── Utilidades ───────────────────────────────────────────
string BaseName(const string& ruta) {
    size_t pos = ruta.find_last_of("\\/");
    return (pos == string::npos) ? ruta : ruta.substr(pos + 1);
}

string SinExtension(const string& nombre) {
    size_t pos = nombre.find_last_of('.');
    return (pos == string::npos) ? nombre : nombre.substr(0, pos);
}
