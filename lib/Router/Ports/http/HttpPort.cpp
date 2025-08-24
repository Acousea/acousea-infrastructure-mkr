#include "HttpPort.hpp"
#include <Logger/Logger.h>

#if defined(__linux__)
  #include <curl/curl.h>
#endif

HttpPort::HttpPort(std::string rxUrl, std::string txUrl, long timeoutMs)
    : IPort(PortType::SBDPort), rxUrl(std::move(rxUrl)), txUrl(std::move(txUrl)), timeoutMs(timeoutMs) {}

void HttpPort::init() {
#if defined(__linux__)
    CURLcode rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (rc != CURLE_OK) {
        Logger::logError("MockHTTPPort::init() -> curl_global_init failed");
        return;
    }
    initialized = true;
    Logger::logInfo("MockHTTPPort::init() -> OK (libcurl)");
#else
    Logger::logError("MockHTTPPort solo soportado en Linux en esta implementación.");
#endif
}

void HttpPort::send(const std::vector<uint8_t>& data) {
#if defined(__linux__)
    if (!initialized) { Logger::logError("MockHTTPPort::send() -> not initialized"); return; }

    CURL* curl = curl_easy_init();
    if (!curl) { Logger::logError("MockHTTPPort::send() -> curl_easy_init failed"); return; }

    struct curl_slist* hdrs = nullptr;
    hdrs = curl_slist_append(hdrs, "Content-Type: application/octet-stream");

    curl_easy_setopt(curl, CURLOPT_URL, txUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reinterpret_cast<const char*>(data.data()));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(data.size()));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);

    Logger::logInfo("MockHTTPPort::send() -> " + Logger::vectorToHexString(data));

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        Logger::logError(std::string("MockHTTPPort::send() -> CURL error: ") + curl_easy_strerror(rc));
    } else {
        long code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        if (code / 100 != 2) {
            Logger::logError("MockHTTPPort::send() -> HTTP status " + std::to_string(code));
        }
    }

    curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
#else
    (void)data;
#endif
}

#if defined(__linux__)
static size_t write_vec_cb(char* ptr, size_t sz, size_t nm, void* userdata) {
    auto* vec = static_cast<std::vector<uint8_t>*>(userdata);
    size_t bytes = sz * nm;
    vec->insert(vec->end(), reinterpret_cast<uint8_t*>(ptr), reinterpret_cast<uint8_t*>(ptr) + bytes);
    return bytes;
}
#endif

bool HttpPort::available() {
    if (!receivedRawPackets.empty()) return true;
#if defined(__linux__)
    (void)fetchOne(); // simple: intentamos traer 1 mensaje si no hay nada
#endif
    return !receivedRawPackets.empty();
}

std::vector<std::vector<uint8_t>> HttpPort::read() {
#if defined(__linux__)
    if (receivedRawPackets.empty()) {
        (void)fetchOne();
    }
#endif
    std::vector<std::vector<uint8_t>> out;
    out.reserve(receivedRawPackets.size());
    for (auto& pkt : receivedRawPackets) out.push_back(std::move(pkt));
    receivedRawPackets.clear();
    return out;
}

bool HttpPort::fetchOne() {
#if defined(__linux__)
    if (!initialized) return false;

    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::vector<uint8_t> body;
    curl_easy_setopt(curl, CURLOPT_URL, rxUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_vec_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        Logger::logError(std::string("MockHTTPPort::fetchOne() -> CURL error: ") + curl_easy_strerror(rc));
        curl_easy_cleanup(curl);
        return false;
    }

    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(curl);

    if (code == 204) {
        // nada que leer
        return false;
    }
    if (code / 100 != 2) {
        Logger::logError("MockHTTPPort::fetchOne() -> HTTP status " + std::to_string(code));
        return false;
    }

    if (!body.empty()) {
        if (receivedRawPackets.size() >= MAX_QUEUE_SIZE) {
            Logger::logInfo("MockHTTPPort::fetchOne(): queue full, dropping oldest");
            receivedRawPackets.pop_front();
        }
        Logger::logInfo("MockHTTPPort::fetchOne() -> received " + std::to_string(body.size()) + " bytes");
        receivedRawPackets.push_back(std::move(body));
        return true;
    }
    return false;
#else
    return false;
#endif
}
