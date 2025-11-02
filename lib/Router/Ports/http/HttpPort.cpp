#include "HttpPort.hpp"
#include <Logger/Logger.h>

#if defined(__linux__)
#include <curl/curl.h>
#include <cctype>
#include <sstream>

static size_t write_vec_cb(char* ptr, size_t sz, size_t nm, void* userdata)
{
    auto* vec = static_cast<std::vector<uint8_t>*>(userdata);
    size_t bytes = sz * nm;
    vec->insert(vec->end(), reinterpret_cast<uint8_t*>(ptr), reinterpret_cast<uint8_t*>(ptr) + bytes);
    return bytes;
}

static size_t write_str_cb(char* ptr, size_t sz, size_t nm, void* userdata)
{
    auto* str = static_cast<std::string*>(userdata);
    size_t bytes = sz * nm;
    str->append(ptr, bytes);
    return bytes;
}
#endif

HttpPort::HttpPort(std::string baseUrl, std::string imei, long timeoutMs, int pollMax)
    : IPort(PortType::SBDPort),
      baseUrl_(std::move(baseUrl)),
      imei_(std::move(imei)),
      timeoutMs_(timeoutMs),
      pollMax_(pollMax > 0 ? pollMax : 1)
{
}

void HttpPort::init()
{
#if defined(__linux__)
    CURLcode rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (rc != CURLE_OK)
    {
        Logger::logError("HttpPort::init() -> curl_global_init failed");
        return;
    }
    initialized_ = true;
    Logger::logInfo("HttpPort::init() -> OK (libcurl)");
#else
    Logger::logError("HttpPort solo soportado en Linux en esta implementación.");
#endif
}

bool HttpPort::send(const std::vector<uint8_t>& data)
{
#if defined(__linux__)
    if (!initialized_) {
        Logger::logError("HttpPort::send() -> not initialized");
        return false;
    }

    const std::string url = baseUrl_ + "/enqueue_mo";
    const std::string dataHex = bytesToHex(data);
    Logger::logInfo("HttpPort::send() -> MO hex: " + dataHex);

    CURL* curl = curl_easy_init();
    if (!curl) {
        Logger::logError("HttpPort::send() -> curl_easy_init failed");
        return false;
    }

    struct curl_slist* hdrs = nullptr;
    char* imeiEsc = nullptr;

    bool success = false; // para control centralizado del resultado

    do {
        imeiEsc = curl_easy_escape(curl, imei_.c_str(), static_cast<int>(imei_.size()));
        if (!imeiEsc) {
            Logger::logError("HttpPort::send() -> curl_easy_escape failed");
            break;
        }

        std::ostringstream form;
        form << "imei=" << imeiEsc << "&data=" << dataHex;
        const std::string body = form.str();

        hdrs = curl_slist_append(hdrs, "Content-Type: application/x-www-form-urlencoded");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs_);

        CURLcode rc = curl_easy_perform(curl);
        if (rc != CURLE_OK) {
            Logger::logError(std::string("HttpPort::send() -> CURL error: ") + curl_easy_strerror(rc));
            break;
        }

        long code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        if (code / 100 != 2) {
            Logger::logError("HttpPort::send() -> HTTP status " + std::to_string(code));
            break;
        }

        // éxito total
        Logger::logInfo("HttpPort::send() -> HTTP OK (" + std::to_string(code) + ")");
        success = true;

    } while (false);

    if (imeiEsc) curl_free(imeiEsc);
    if (hdrs) curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);

    return success;

#else
    (void)data;
    Logger::logError("HttpPort::send() not supported on this platform");
    return false;
#endif
}


bool HttpPort::available()
{
    if (!receivedRawPackets.empty()) return true;
#if defined(__linux__)
    (void)fetchOne(); // intenta traer 1 si está vacío
#endif
    return !receivedRawPackets.empty();
}

std::vector<std::vector<uint8_t>> HttpPort::read()
{
#if defined(__linux__)
    if (receivedRawPackets.empty())
    {
        (void)fetchOne();
    }
#endif
    std::vector<std::vector<uint8_t>> out;
    out.reserve(receivedRawPackets.size());
    for (auto& pkt : receivedRawPackets) out.push_back(std::move(pkt));
    receivedRawPackets.clear();
    return out;
}

bool HttpPort::fetchOne()
{
#if defined(__linux__)
    if (!initialized_) return false;

    // GET /modem/poll?imei=...&max=pollMax_
    std::ostringstream u;
    u << baseUrl_ << "/modem/poll?imei=" << imei_ << "&max=" << pollMax_;

    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string resp;
    curl_easy_setopt(curl, CURLOPT_URL, u.str().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs_);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_str_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK)
    {
        Logger::logError(std::string("HttpPort::fetchOne() -> CURL error: ") + curl_easy_strerror(rc));
        curl_easy_cleanup(curl);
        return false;
    }

    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(curl);

    if (code / 100 != 2)
    {
        Logger::logError("HttpPort::fetchOne() -> HTTP status " + std::to_string(code));
        return false;
    }

    // Respuesta esperada: JSON array. Ej: [] o [{"id":1,"data_hex":"A1B2","created_at":"..."}]
    // Parser mínimo: busca el primer "data_hex":"...".
    auto posKey = resp.find("\"data_hex\"");
    if (posKey == std::string::npos)
    {
        // array vacío o formato inesperado -> no hay nada
        return false;
    }
    // Busca ':' y la primera comilla que abre el valor
    auto posColon = resp.find(':', posKey);
    if (posColon == std::string::npos) return false;
    auto posQuote1 = resp.find('"', posColon + 1);
    if (posQuote1 == std::string::npos) return false;
    auto posQuote2 = resp.find('"', posQuote1 + 1);
    if (posQuote2 == std::string::npos) return false;

    std::string hex = resp.substr(posQuote1 + 1, posQuote2 - (posQuote1 + 1));

    // normaliza a minúsculas
    for (auto& c : hex) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    std::vector<uint8_t> bytes;
    if (!hexToBytes(hex, bytes))
    {
        Logger::logError("HttpPort::fetchOne() -> invalid hex in data_hex");
        return false;
    }

    if (receivedRawPackets.size() >= MAX_QUEUE_SIZE)
    {
        Logger::logInfo("HttpPort::fetchOne(): queue full, dropping oldest");
        receivedRawPackets.pop_front();
    }
    Logger::logInfo("HttpPort::fetchOne() -> RX " + std::to_string(bytes.size()) + " bytes (MT)");
    receivedRawPackets.push_back(std::move(bytes));
    return true;
#else
    return false;
#endif
}


std::string HttpPort::bytesToHex(const std::vector<uint8_t>& in)
{
    static const char* digits = "0123456789abcdef";
    std::string out;
    out.resize(in.size() * 2);
    for (size_t i = 0; i < in.size(); ++i)
    {
        out[2 * i] = digits[(in[i] >> 4) & 0xF];
        out[2 * i + 1] = digits[in[i] & 0xF];
    }
    return out;
}

bool HttpPort::hexToBytes(const std::string& hex, std::vector<uint8_t>& out)
{
    if (hex.size() % 2 != 0) return false;
    out.clear();
    out.reserve(hex.size() / 2);
    auto hexVal = [](char c) -> int
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };
    for (size_t i = 0; i < hex.size(); i += 2)
    {
        int hi = hexVal(hex[i]);
        int lo = hexVal(hex[i + 1]);
        if (hi < 0 || lo < 0) return false;
        out.push_back(static_cast<uint8_t>((hi << 4) | lo));
    }
    return true;
}
