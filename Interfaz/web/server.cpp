// ============================================================
// Servidor HTTP embebido — Interfaz Web Moderna
// Optimizacion de Riego — ADA
// ============================================================
// Compilar:
//   g++ -std=c++17 -Wall -Wextra Interfaz/web/server.cpp
//       Objetos/Finca.cpp Objetos/Tablon.cpp
//       -o riego.exe -lws2_32 -lcomdlg32
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <shellapi.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>

// Incluir con ruta relativa desde donde se compila (raiz del proyecto)
#include "../../Objetos/Finca.h"
#include "../../Objetos/Tablon.h"

using namespace std;

// ─── Config ────────────────────────────────────────────────
#define PORT 9174
#define BUFSIZE 65536
#define WEB_DIR "Interfaz/web"

// ─── Estado global ─────────────────────────────────────────
static vector<Tablon> g_tablones;
static map<string, pair<vector<int>, double>> g_resultados;
static string g_archivoActual;
static mutex g_mutex;

// ─── Helpers de string ─────────────────────────────────────

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static string jsonEscape(const string& s) {
    string r;
    r.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '"') r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else if (c == '\r') r += "\\r";
        else if (c == '\t') r += "\\t";
        else r += c;
    }
    return r;
}

static string jsonStr(const string& s) {
    return "\"" + jsonEscape(s) + "\"";
}

static string jsonNum(double n) {
    long long ni = (long long)n;
    if (n == (double)ni) return to_string(ni);
    return to_string(n);
}

// ─── HTTP response builder ─────────────────────────────────

static string httpResponse(const string& body, const string& contentType, int statusCode) {
    const char* statusText;
    switch (statusCode) {
        case 200: statusText = "OK"; break;
        case 400: statusText = "Bad Request"; break;
        case 404: statusText = "Not Found"; break;
        case 500: statusText = "Internal Server Error"; break;
        default:  statusText = "Unknown"; break;
    }

    char header[512];
    int n = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n",
        statusCode, statusText, contentType.c_str(), body.size());
    if (n < 0 || (size_t)n >= sizeof(header))
        return "HTTP/1.1 500 Error\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    return string(header) + body;
}

static string json(const string& body, int code = 200) { return httpResponse(body, "application/json; charset=utf-8", code); }
static string jsonOk(const string& extra = "") {
    return json("{\"ok\":true" + (extra.empty() ? "" : "," + extra) + "}");
}
static string jsonErr(const string& msg) {
    return json("{\"ok\":false,\"error\":" + jsonStr(msg) + "}", 400);
}

// ─── Ruta del ejecutable ───────────────────────────────────

static string getExeDir() {
    char path[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return ".";
    string full(path);
    size_t pos = full.find_last_of("\\/");
    return (pos != string::npos) ? full.substr(0, pos) : ".";
}

// ─── Parseo de body JSON (minimo) ──────────────────────────
// Solo necesitamos extraer valores planos tipo {"clave":"valor"} o {"clave":numero}

static string jsonExtractString(const string& json, const string& key) {
    string search = "\"" + key + "\":\"";
    size_t p = json.find(search);
    if (p == string::npos) return "";
    p += search.size();
    string r;
    while (p < json.size() && json[p] != '"') {
        if (json[p] == '\\' && p + 1 < json.size()) {
            if (json[p+1] == '"') r += '"';
            else if (json[p+1] == '\\') r += '\\';
            else if (json[p+1] == 'n') r += '\n';
            else r += json[p+1];
            p += 2;
        } else {
            r += json[p++];
        }
    }
    return r;
}

// ─── Servir archivos estaticos ─────────────────────────────

static string getMimeType(const string& path) {
    size_t dot = path.find_last_of('.');
    if (dot == string::npos) return "application/octet-stream";
    string ext = path.substr(dot);
    if (ext == ".html") return "text/html; charset=utf-8";
    if (ext == ".css")  return "text/css; charset=utf-8";
    if (ext == ".js")   return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".png")  return "image/png";
    if (ext == ".ico")  return "image/x-icon";
    if (ext == ".svg")  return "image/svg+xml";
    return "application/octet-stream";
}

static string readFile(const string& path) {
    ifstream f(path, ios::binary | ios::ate);
    if (!f.is_open()) return "";
    streamsize sz = f.tellg();
    f.seekg(0);
    string content((size_t)sz, '\0');
    f.read(&content[0], sz);
    return content;
}

static string serveFile(const string& exeDir, const string& urlPath) {
    // Seguridad: no permitir subir de directorio
    if (urlPath.find("..") != string::npos)
        return httpResponse("Forbidden", "text/plain", 403);

    string relPath = urlPath.empty() || urlPath == "/" ? "/index.html" : urlPath;
    string filePath = exeDir + "\\" + WEB_DIR + relPath;
    // Reemplazar / por backslash
    for (char& c : filePath) if (c == '/') c = '\\';

    string content = readFile(filePath);
    if (content.empty()) {
        char buf[256];
        snprintf(buf, sizeof(buf), "404 Not Found: %s", urlPath.c_str());
        return httpResponse(buf, "text/plain", 404);
    }

    string mime = getMimeType(relPath);
    return httpResponse(content, mime, 200);
}

// ═══════════════════════════════════════════════════════════
// API Handlers
// ═══════════════════════════════════════════════════════════

// ─── GET /api/status ───────────────────────────────────────
static string apiStatus() {
    lock_guard<mutex> lock(g_mutex);
    string r = "\"n\":" + to_string(g_tablones.size());
    r += ",\"archivo\":" + jsonStr(g_archivoActual);
    return jsonOk(r);
}

// ─── POST /api/load ────────────────────────────────────────
static string apiLoad(const string& body) {
    lock_guard<mutex> lock(g_mutex);
    g_tablones.clear();
    g_resultados.clear();
    g_archivoActual = "";

    // body es el contenido del archivo (texto plano)
    // Strip BOM if present
    string data = body;
    if (data.size() >= 3 && (unsigned char)data[0] == 0xEF && 
        (unsigned char)data[1] == 0xBB && (unsigned char)data[2] == 0xBF)
        data = data.substr(3);

    istringstream ss(data);
    string linea;

    // Leer n
    if (!getline(ss, linea))
        return jsonErr("Archivo vacio");
    linea = trim(linea);
    int n;
    try { n = stoi(linea); } catch (...) { return jsonErr("Primera linea debe ser un numero entero"); }
    if (n <= 0) return jsonErr("Numero de tablones invalido");

    // Leer tablones
    int leidos = 0;
    while (getline(ss, linea) && leidos < n) {
        linea = trim(linea);
        if (linea.empty()) continue;
        for (char& c : linea) if (c == ',') c = ' ';
        int ts, tr, p, rp;
        istringstream sl(linea);
        if (sl >> ts >> tr >> p >> rp) {
            g_tablones.emplace_back(ts, tr, p, rp);
            leidos++;
        }
    }

    if (g_tablones.empty())
        return jsonErr("No se pudo leer ningun tablon valido");

    // Construir JSON de respuesta
    string json = "\"n\":" + to_string(g_tablones.size());
    json += ",\"tablones\":[";
    for (size_t i = 0; i < g_tablones.size(); i++) {
        if (i > 0) json += ",";
        const auto& t = g_tablones[i];
        json += "{";
        json += "\"i\":" + to_string(i) + ",";
        json += "\"ts\":" + to_string(t.getTiempoDeSupervivencia()) + ",";
        json += "\"tr\":" + to_string(t.getTiempoDeRegado()) + ",";
        json += "\"p\":" + to_string(t.getPrioridad()) + ",";
        json += "\"rp\":" + to_string(t.getTiempoDeRiegoPerfecto());
        json += "}";
    }
    json += "]";

    g_archivoActual = "datos.cargados";
    return jsonOk(json);
}

// ─── POST /api/run ─────────────────────────────────────────
static string apiRun(const string& body) {
    lock_guard<mutex> lock(g_mutex);
    if (g_tablones.empty()) return jsonErr("No hay datos cargados");

    string algo = jsonExtractString(body, "algoritmo");
    Finca finca(g_tablones);

    if (algo == "todos") {
        string r = "\"resultados\":[";
        
        auto fb = finca.roFB();
        g_resultados["fb"] = fb;
        r += "{\"algoritmo\":" + jsonStr("Fuerza Bruta (roFB)");
        r += ",\"costo\":" + jsonNum(fb.second);
        r += ",\"clave\":\"fb\"";
        r += ",\"permutacion\":[";
        for (size_t i = 0; i < fb.first.size(); i++) {
            if (i > 0) r += ",";
            r += to_string(fb.first[i]);
        }
        r += "]},";

        auto vz = finca.roV();
        g_resultados["voraz"] = vz;
        r += "{\"algoritmo\":" + jsonStr("Voraz (roV)");
        r += ",\"costo\":" + jsonNum(vz.second);
        r += ",\"clave\":\"voraz\"";
        r += ",\"permutacion\":[";
        for (size_t i = 0; i < vz.first.size(); i++) {
            if (i > 0) r += ",";
            r += to_string(vz.first[i]);
        }
        r += "]},";

        auto pd = finca.roPD();
        g_resultados["pd"] = pd;
        r += "{\"algoritmo\":" + jsonStr("PD (roPD)");
        r += ",\"costo\":" + jsonNum(pd.second);
        r += ",\"clave\":\"pd\"";
        r += ",\"permutacion\":[";
        for (size_t i = 0; i < pd.first.size(); i++) {
            if (i > 0) r += ",";
            r += to_string(pd.first[i]);
        }
        r += "]},";

        auto rd = finca.roD();
        g_resultados["rd"] = rd;
        r += "{\"algoritmo\":" + jsonStr("roD (wrapper)");
        r += ",\"costo\":" + jsonNum(rd.second);
        r += ",\"clave\":\"rd\"";
        r += ",\"permutacion\":[";
        for (size_t i = 0; i < rd.first.size(); i++) {
            if (i > 0) r += ",";
            r += to_string(rd.first[i]);
        }
        r += "]}";

        r += "]";

        // Optimalidad
        double ref = fb.second;
        bool ok = (vz.second == ref && pd.second == ref && rd.second == ref);
        r += ",\"optimalidad\":" + string(ok ? "true" : "false");

        return jsonOk(r);
    }

    // Individual
    pair<vector<int>, double> res;
    string nombre, clave;
    if (algo == "fb") {
        res = finca.roFB();   nombre = "Fuerza Bruta (roFB)"; clave = "fb";
    } else if (algo == "voraz") {
        res = finca.roV();    nombre = "Voraz (roV)";         clave = "voraz";
    } else if (algo == "pd") {
        res = finca.roPD();   nombre = "PD (roPD)";           clave = "pd";
    } else {
        return jsonErr("Algoritmo desconocido: use fb, voraz, pd, o todos");
    }

    g_resultados[clave] = res;

    string r = jsonStr("algoritmo") + ":" + jsonStr(nombre);
    r += "," + jsonStr("clave") + ":" + jsonStr(clave);
    r += "," + jsonStr("costo") + ":" + jsonNum(res.second);
    r += "," + jsonStr("permutacion") + ":[";
    for (size_t i = 0; i < res.first.size(); i++) {
        if (i > 0) r += ",";
        r += to_string(res.first[i]);
    }
    r += "]";
    return jsonOk(r);
}

// ─── POST /api/verify ──────────────────────────────────────
static string apiVerify(const string& body) {
    lock_guard<mutex> lock(g_mutex);
    if (g_tablones.empty()) return jsonErr("No hay datos cargados");

    string algo = jsonExtractString(body, "algoritmo");
    if (algo.empty()) algo = "fb";

    auto it = g_resultados.find(algo);
    if (it == g_resultados.end())
        return jsonErr("Ejecute el algoritmo " + algo + " primero");

    const auto& [perm, costoEsperado] = it->second;

    string r = "\"algoritmo\":" + jsonStr(algo);
    r += ",\"costo_esperado\":" + jsonNum(costoEsperado);
    r += ",\"pasos\":[";

    double costoAcum = 0;
    int tiempo = 0;

    for (size_t paso = 0; paso < perm.size(); paso++) {
        if (paso > 0) r += ",";
        int idx = perm[paso];
        const Tablon& t = g_tablones[idx];

        int ts = t.getTiempoDeSupervivencia();
        int tr = t.getTiempoDeRegado();
        int p  = t.getPrioridad();
        int rp = t.getTiempoDeRiegoPerfecto();

        int costoPaso = 0;
        string rama, formula;

        if (rp == tiempo) {
            costoPaso = ts - (tiempo + tr);
            rama = "rp == tiempo";
            char buf[128];
            snprintf(buf, sizeof(buf), "%d - (%d + %d) = %d", ts, tiempo, tr, costoPaso);
            formula = buf;
        } else if ((ts - tr) >= tiempo) {
            costoPaso = 2 * (ts - (tiempo + tr));
            rama = "(ts - tr) >= tiempo";
            char buf[128];
            snprintf(buf, sizeof(buf), "2 * (%d - (%d + %d)) = %d", ts, tiempo, tr, costoPaso);
            formula = buf;
        } else {
            costoPaso = 2 * p * ((tiempo + tr) - ts);
            rama = "(ts - tr) < tiempo";
            char buf[128];
            snprintf(buf, sizeof(buf), "2 * %d * ((%d + %d) - %d) = %d", p, tiempo, tr, ts, costoPaso);
            formula = buf;
        }

        costoAcum += costoPaso;
        tiempo += tr;

        r += "{";
        r += "\"paso\":" + to_string(paso + 1) + ",";
        r += "\"indice\":" + to_string(idx) + ",";
        r += "\"ts\":" + to_string(ts) + ",";
        r += "\"tr\":" + to_string(tr) + ",";
        r += "\"p\":" + to_string(p) + ",";
        r += "\"rp\":" + to_string(rp) + ",";
        r += "\"tiempo\":" + to_string(tiempo - tr) + ",";
        r += "\"rama\":" + jsonStr(rama) + ",";
        r += "\"formula\":" + jsonStr(formula) + ",";
        r += "\"costo_paso\":" + to_string(costoPaso) + ",";
        r += "\"costo_acum\":" + jsonNum(costoAcum);
        r += "}";
    }

    r += "]";
    r += ",\"total\":" + jsonNum(costoAcum);
    r += ",\"coincide\":" + string((costoAcum == costoEsperado) ? "true" : "false");

    return jsonOk(r);
}

// ─── POST /api/save ────────────────────────────────────────
static string apiSave(const string& body) {
    lock_guard<mutex> lock(g_mutex);
    string algo = jsonExtractString(body, "algoritmo");
    string archivo = jsonExtractString(body, "archivo");
    if (algo.empty()) algo = "fb";
    if (archivo.empty()) archivo = "salida.txt";

    auto it = g_resultados.find(algo);
    if (it == g_resultados.end())
        return jsonErr("Ejecute el algoritmo " + algo + " primero");

    const auto& [perm, costo] = it->second;
    FILE* f = fopen(archivo.c_str(), "w");
    if (!f) return jsonErr("No se pudo crear el archivo");

    fprintf(f, "%.0f\n", costo);
    for (int idx : perm) fprintf(f, "%d\n", idx);
    fclose(f);

    string r = "\"mensaje\":" + jsonStr("Guardado en " + archivo);
    return jsonOk(r);
}

// ─── POST /api/download ────────────────────────────────────
static string apiDownload(const string& body) {
    lock_guard<mutex> lock(g_mutex);
    string algo = jsonExtractString(body, "algoritmo");
    if (algo.empty()) algo = "fb";

    auto it = g_resultados.find(algo);
    if (it == g_resultados.end())
        return jsonErr("Ejecute el algoritmo " + algo + " primero");

    const auto& [perm, costo] = it->second;
    char buf[64];
    snprintf(buf, sizeof(buf), "%.0f\n", costo);
    string content = buf;
    for (int idx : perm) {
        snprintf(buf, sizeof(buf), "%d\n", idx);
        content += buf;
    }

    return httpResponse(content, "text/plain; charset=utf-8", 200);
}

// ═══════════════════════════════════════════════════════════
// HTTP Request parsing + routing
// ═══════════════════════════════════════════════════════════

static string routeRequest(const string& exeDir, const string& /*method*/, const string& path, const string& body) {
    // ── API routes ──
    if (path == "/api/status")   return apiStatus();
    if (path == "/api/load")     return apiLoad(body);
    if (path == "/api/run")      return apiRun(body);
    if (path == "/api/verify")   return apiVerify(body);
    if (path == "/api/save")     return apiSave(body);
    if (path == "/api/download") return apiDownload(body);

    // ── Static files ──
    return serveFile(exeDir, path);
}

// ═══════════════════════════════════════════════════════════
// Cliente: leer request y responder
// ═══════════════════════════════════════════════════════════

static void handleClient(SOCKET client, const string& exeDir) {
    char buf[BUFSIZE];
    int received = recv(client, buf, sizeof(buf) - 1, 0);
    if (received <= 0) { closesocket(client); return; }
    buf[received] = '\0';

    // Parsear request line
    string req(buf);
    size_t endLine = req.find("\r\n");
    if (endLine == string::npos) { closesocket(client); return; }

    string requestLine = req.substr(0, endLine);
    size_t sp1 = requestLine.find(' ');
    if (sp1 == string::npos) { closesocket(client); return; }
    size_t sp2 = requestLine.find(' ', sp1 + 1);
    if (sp2 == string::npos) { closesocket(client); return; }

    string method = requestLine.substr(0, sp1);
    string path = requestLine.substr(sp1 + 1, sp2 - sp1 - 1);

    // Parsear headers para encontrar Content-Length
    size_t headerEnd = req.find("\r\n\r\n");
    if (headerEnd == string::npos) { closesocket(client); return; }

    string body_part;
    string headersStr = req.substr(endLine + 2, headerEnd - endLine - 2);

    // Buscar Content-Length
    int contentLength = 0;
    size_t clPos = headersStr.find("Content-Length:");
    if (clPos == string::npos) clPos = headersStr.find("content-length:");
    if (clPos != string::npos) {
        string clStr = headersStr.substr(clPos + 15);
        clStr = trim(clStr);
        contentLength = atoi(clStr.c_str());
    }

    body_part = req.substr(headerEnd + 4);
    // Si el body es mas largo de lo que recibimos en el primer paquete, leer el resto
    while ((int)body_part.size() < contentLength) {
        int n = recv(client, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = '\0';
        body_part += buf;
    }

    // Ruteo
    string response;
    try {
        response = routeRequest(exeDir, method, path, body_part);
    } catch (const exception& e) {
        response = jsonErr(string("Error interno: ") + e.what());
    }

    send(client, response.c_str(), (int)response.size(), 0);
    closesocket(client);
}

// ═══════════════════════════════════════════════════════════
// Abrir navegador
// ═══════════════════════════════════════════════════════════

static void openBrowser(const string& url) {
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

// ═══════════════════════════════════════════════════════════
// Entry point
// ═══════════════════════════════════════════════════════════

int main() {
    SetConsoleTitleA("Servidor de Riego — ADA");

    string exeDir = getExeDir();
    printf("Directorio del ejecutable: %s\n", exeDir.c_str());
    printf("Sirviendo archivos desde: %s\\%s\n", exeDir.c_str(), WEB_DIR);

    // Inicializar Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "Error: WSAStartup fallo\n");
        return 1;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        fprintf(stderr, "Error: No se pudo crear el socket\n");
        WSACleanup();
        return 1;
    }

    // Reutilizar direccion
    int reuse = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // solo localhost
    addr.sin_port = htons(PORT);

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) != 0) {
        fprintf(stderr, "Error: No se pudo bindear al puerto %d\n", PORT);
        closesocket(server);
        WSACleanup();
        return 1;
    }

    if (listen(server, SOMAXCONN) != 0) {
        fprintf(stderr, "Error: listen fallo\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    string url = "http://localhost:" + to_string(PORT);
    printf("\n");
    printf("  =========================================\n");
    printf("    Optimizacion de Riego — ADA\n");
    printf("    Servidor iniciado en:\n");
    printf("    >>  %s  <<\n", url.c_str());
    printf("  =========================================\n");
    printf("\n");
    printf("  Presione Ctrl+C para detener el servidor.\n\n");

    // Abrir navegador
    openBrowser(url);

    // Aceptar conexiones
    while (true) {
        SOCKET client = accept(server, NULL, NULL);
        if (client == INVALID_SOCKET) continue;

        // Thread por conexion
        thread(handleClient, client, exeDir).detach();
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
