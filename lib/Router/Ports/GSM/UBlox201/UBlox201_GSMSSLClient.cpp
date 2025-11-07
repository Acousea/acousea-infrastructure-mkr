#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)

#include "UBlox201_GSMSSLClient.hpp"
#include <Arduino.h>

#include <vector>
#include <string>

inline CertType certTypeFromInt(int type)
{
    switch (type)
    {
    case 0: return CertType::RootCA;
    case 1: return CertType::ClientCert;
    case 2: return CertType::PrivateKey;
    default: return CertType::All;
    }
}

inline CertType certTypeFromString(const char* typeStr)
{
    if (!typeStr) return CertType::Unknown;

    if (strcmp(typeStr, "CA") == 0) return CertType::RootCA;
    if (strcmp(typeStr, "CC") == 0) return CertType::ClientCert;
    if (strcmp(typeStr, "PK") == 0) return CertType::PrivateKey;
    return CertType::Unknown;
}


/**
 * Normaliza una cadena en el mismo buffer:
 * - Convierte CRLF → LF
 * - Elimina comillas
 * - Evita líneas vacías repetidas
 */
void normalizeOutputCrlfAndQuotationMarks(char* input)
{
    LOG_INFO("normalizeCrlf() -> Normalizing input of size %zu", input ? strlen(input) : 0);

    if (!input) return;

    size_t w = 0;
    bool lastWasNewline = false;

    for (size_t r = 0; input[r]; ++r)
    {
        const char c = input[r];

        if (c == '\r' || c == '"') continue; // ignorar CR y comillas

        if (c == '\n')
        {
            if (lastWasNewline) continue;
            lastWasNewline = true;
        }
        else
        {
            lastWasNewline = false;
        }

        input[w++] = c;
    }
    input[w] = '\0';
}

/**
 * Divide un texto por líneas y devuelve el número de líneas no vacías encontradas.
 * Cada puntero apunta dentro del mismo buffer modificado (se insertan '\0').
 */
size_t splitByNewlines(char* text, const char* linesOut[], const size_t maxLines)
{
    LOG_INFO("splitByNewlines() -> Splitting text into lines (up to %zu lines)", maxLines);
    if (!text || !linesOut) return 0;

    size_t count = 0;
    const char* token = strtok(text, "\n");

    while (token && count < maxLines)
    {
        if (strlen(token) > 0)
        {
            linesOut[count++] = token;
        }
        token = strtok(nullptr, "\n");
    }

    return count;
}

/**
 * Parsea una línea tipo CSV y llena una estructura StoredCert.
 * Ejemplo de línea: "CA,myCert,CommonName,2025-12-31"
 */
StoredCert buildStoredCert(const char* line)
{
    StoredCert cert{};
    if (!line) return cert;

    const char* parts[4] = {nullptr, nullptr, nullptr, nullptr};
    size_t partIndex = 0;

    // Creamos una copia temporal para usar strtok
    char buffer[256];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* token = strtok(buffer, ",");
    while (token && partIndex < 4)
    {
        parts[partIndex++] = token;
        token = strtok(nullptr, ",");
    }

    // Copiar partes al struct
    strncpy(cert.type, parts[0] ? parts[0] : "", sizeof(cert.type) - 1);
    strncpy(cert.internalName, parts[1] ? parts[1] : "", sizeof(cert.internalName) - 1);
    strncpy(cert.commonName, parts[2] ? parts[2] : "", sizeof(cert.commonName) - 1);
    strncpy(cert.expiration, parts[3] ? parts[3] : "", sizeof(cert.expiration) - 1);

    LOG_INFO("buildStoredCert() -> Type: %s, Name: %s, CN: %s, Exp: %s",
             cert.type, cert.internalName, cert.commonName, cert.expiration);

    return cert;
}


bool UBlox201_GSMSSLClient::updateCerts(
    const GSMRootCert* rootCerts, const size_t rootCertsSize, const char* testBroker /* = "google.com" */
)
{
    LOG_CLASS_INFO(" -> Loading new trusted root CAs...");
    LOG_CLASS_INFO("updateCerts() -> Number of root certs to load: %zu", rootCertsSize);
    this->eraseTrustedRoot(); // Erase the currently existing roots
    this->setUserRoots(rootCerts, rootCertsSize);
    for (size_t i = 0; i < rootCertsSize; i++)
    {
        const GSMRootCert& rootCert = rootCerts[i];
        this->importTrustedRoot(rootCert.data, rootCert.name, rootCert.size);
        this->setTrustedRoot(rootCert.name);
        LOG_CLASS_INFO(" -> Trusted root CA added: %s", rootCert.name);
    }

    LOG_CLASS_INFO(" -> Verifying by connecting to %s:443", testBroker);
    if (!this->connect(testBroker, 443))
    {
        LOG_CLASS_ERROR("updateCerts()  CERTIFICATES NOT UPDATED. SSL connect() failed");

        this->stop();
        return false;
    }
    LOG_CLASS_INFO("updateCerts() -> CERTIFICATES UPDATED");
    return true;
}


std::vector<StoredCert> UBlox201_GSMSSLClient::listCertificates(CertType type)
{
    LOG_CLASS_INFO("listCertificates() -> Listing certificates of type: %s",
                   (type == CertType::All
                       ? "All"
                       : type == CertType::RootCA
                       ? "RootCA"
                       : type == CertType::ClientCert
                       ? "ClientCert"
                       : type == CertType::PrivateKey
                       ? "PrivateKey"
                       : "Unknown")
    );

    if (type == CertType::All || type == CertType::Unknown)
    {
        MODEM.send("AT+USECMNG=3"); // listar todos
    }
    else
    {
        MODEM.sendf("AT+USECMNG=3,%d", static_cast<int>(type));
    }


    String modemOutput;
    if (MODEM.waitForResponse(5000, &modemOutput) != 1)
    {
        LOG_CLASS_ERROR("listCertificates() -> No response from modem");
        return {}; // vacío
    }
    const unsigned int outLen = modemOutput.length();
    if (outLen <= 0)
    {
        LOG_CLASS_WARNING("listCertificates() -> Empty response");
        return {};
    }

    LOG_CLASS_INFO("listCertificates() -> Modem output length: %d", outLen);

    // Copiar a buffer C-style
    char modemResponseBuf[2048];
    strncpy(modemResponseBuf, modemOutput.c_str(), sizeof(modemResponseBuf) - 1);
    modemResponseBuf[sizeof(modemResponseBuf) - 1] = '\0';


    LOG_CLASS_INFO("listCertificates() -> Modem output copied to string-buf, length: %zu", strlen(modemResponseBuf));
    // [WARNING] Do not print the following line in full, it may overflow the log buffer depending on the display
    LOG_CLASS_INFO("listCertificates() -> MODEM Response (truncated): %.100s", modemResponseBuf);

    // Normalizar CRLF y eliminar comillas
    normalizeOutputCrlfAndQuotationMarks(modemResponseBuf);

    // Dividir por líneas
    const char* lines[64];
    const size_t numLines = splitByNewlines(modemResponseBuf, lines, 64);

    LOG_CLASS_INFO("listCertificates() -> Cleaned input size: %zu", strlen(modemResponseBuf));
    LOG_CLASS_INFO("listCertificates() -> Found ""%zu" " certificate lines", numLines);

    std::vector<StoredCert> certs = {};
    certs.reserve(numLines);

    // Parsear cada línea CSV
    for (size_t i = 0; i < numLines; ++i)
    {
        const char* linePtr = lines[i];
        if (linePtr == nullptr || linePtr[0] == '\0') continue;
        StoredCert cert = buildStoredCert(linePtr);
        certs.push_back(cert);
        LOG_CLASS_INFO("listCertificates() -> Parsed line %zu: %s", i, lines[i]);
    }

    LOG_CLASS_INFO("listCertificates() -> Total certificates parsed: %zu", certs.size());

    return certs;
}

bool UBlox201_GSMSSLClient::removeClientCertificate(const char* name)
{
    MODEM.sendf("AT+USECMNG=2,1,\"%s\"", name);
    int res = MODEM.waitForResponse(3000);
    return logAndCheckResponse(res, "client certificate", name);
}

bool UBlox201_GSMSSLClient::removePrivateKey(const char* name)
{
    MODEM.sendf("AT+USECMNG=2,2,\"%s\"", name);
    int res = MODEM.waitForResponse(3000);
    return logAndCheckResponse(res, "private key", name);
}

bool UBlox201_GSMSSLClient::removeRootCertificateByName(const char* name)
{
    MODEM.sendf("AT+USECMNG=2,0,\"%s\"", name);
    int res = MODEM.waitForResponse(3000);
    return logAndCheckResponse(res, "root certificate", name);
}

bool UBlox201_GSMSSLClient::removeTrustedRootCertificates(const std::vector<char*>& names)
{
    bool allOk = true;
    for (const auto& name : names)
    {
        if (!removeRootCertificateByName(name))
        {
            allOk = false; // seguimos, pero marcamos fallo
        }
    }
    return allOk;
}

bool UBlox201_GSMSSLClient::removeTrustedRootCertificates(const GSMRootCert* certs, size_t count)
{
    bool allOk = true;
    for (size_t i = 0; i < count; ++i)
    {
        const char* name = certs[i].name;
        if (!removeRootCertificateByName(name))
        {
            allOk = false;
        }
    }
    return allOk;
}

bool UBlox201_GSMSSLClient::removeCustomTrustedRoots()
{
    return removeTrustedRootCertificates(GSM_ROOT_CERTS, GSM_NUM_ROOT_CERTS);
}

void UBlox201_GSMSSLClient::importTrustedRoot(const uint8_t* cert, const char* name, size_t size)
{
    MODEM.sendf("AT+USECMNG=0,0,\"%s\",%d", name, size);
    int res = MODEM.waitForResponse(1000);
    logAndCheckResponse(res, "root certificate import (prepare)", name);

    MODEM.write(cert, size);
    res = MODEM.waitForResponse(2000);
    logAndCheckResponse(res, "root certificate import (write)", name);
}

void UBlox201_GSMSSLClient::importTrustedRoots(const GSMRootCert* certs, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        importTrustedRoot(certs[i].data, certs[i].name, certs[i].size);
    }
}

void UBlox201_GSMSSLClient::setTrustedRoots(const GSMRootCert* gsm_root_cert, const size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        this->setTrustedRoot(gsm_root_cert[i].name);
        LOG_CLASS_INFO(" -> Trusted root set: %s", gsm_root_cert[i].name);
    }
}


bool UBlox201_GSMSSLClient::logAndCheckResponse(const int result, const char* action, const char* name)
{
    switch (result)
    {
    case 1:
        LOG_CLASS_INFO("Removed %s: %s", action, name);
        return true;
    case 2:
        LOG_CLASS_ERROR(" -> AT command error while removing %s: %s", action, name);
        return false;
    case 3:
        LOG_CLASS_ERROR(" -> No carrier while removing %s: %s", action, name);
        return false;
    case -1:
    default:
        LOG_CLASS_ERROR(" -> Timeout while removing %s: %s", action, name);
        return false;
    }
}

#if ENVIRONMENT == ENV_TEST
bool UBlox201_GSMSSLClient::testTLSConnection(const char* host, const uint16_t port, const char* path)
{
    LOG_CLASS_INFO(" -> Testing secure connection to %s:%u", host, port);

    // Intentar conectar vía TLS
    LOG_CLASS_INFO(" -> Using TLS (GSMSSLClient base)");
    if (!connect(host, port))
    {
        LOG_CLASS_ERROR(" -> TLS connection FAILED");
        stop(); // Asegurar limpieza
        return false;
    }

    LOG_CLASS_INFO(" -> TLS connection SUCCESS");

    // ----------- Construir y enviar petición HTTPs ----------- //
    char request[256];
    std::snprintf(request, sizeof(request),
                  "GET %s HTTP/1.1\r\n"
                  "Host: %s\r\n"
                  "Connection: close\r\n\r\n",
                  path, host);

    print(request);
    LOG_CLASS_INFO(" -> HTTPS request sent: %s", request);

    // ----------- Leer respuesta limitada ----------- //
    constexpr size_t MAX_RESPONSE_SIZE = 1024;
    char response[MAX_RESPONSE_SIZE + 1];
    size_t totalRead = 0;

    LOG_CLASS_INFO(" -> Reading up to %zu bytes of HTTPS response...", MAX_RESPONSE_SIZE);

    const uint32_t startTime = millis();
    while (connected() && (millis() - startTime) < 10000 && totalRead < MAX_RESPONSE_SIZE)
    {
        while (available() && totalRead < MAX_RESPONSE_SIZE)
        {
            const int c = read();
            if (c < 0) continue;
            response[totalRead++] = static_cast<char>(c);
        }
    }

    response[totalRead] = '\0'; // Null-terminate

    if (totalRead >= MAX_RESPONSE_SIZE)
        LOG_CLASS_WARNING(" -> Response truncated to %zu bytes", MAX_RESPONSE_SIZE);

    LOG_CLASS_INFO(" -> HTTPS response (%zu bytes):\n%.1024s", totalRead, response);

    stop();
    LOG_CLASS_INFO(" -> Connection closed successfully.");

    return true;
}
#endif

#endif
