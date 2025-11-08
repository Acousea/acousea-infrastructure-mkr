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
    LOG_INFO("normalizeCrlf() -> Normalizing input of size %d", input ? strlen(input) : 0);

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
    LOG_INFO("splitByNewlines() -> Splitting text into lines (up to %d lines)", maxLines);
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
    LOG_CLASS_INFO("updateCerts() -> Number of root certs to load: %d", rootCertsSize);
    GSMSSLClient::eraseTrustedRoot(); // Erase the currently existing roots
    GSMSSLClient::setUserRoots(rootCerts, rootCertsSize);
    for (size_t i = 0; i < rootCertsSize; i++)
    {
        const GSMRootCert& rootCert = rootCerts[i];
        UBlox201_GSMSSLClient::importTrustedRoot(rootCert.data, rootCert.name, rootCert.size);
        GSMSSLClient::setTrustedRoot(rootCert.name);
        LOG_CLASS_INFO(" -> Trusted root CA added: %s", rootCert.name);
    }

    LOG_CLASS_INFO(" -> Verifying by connecting to %s:443", testBroker);
    if (!this->connect(testBroker, 443))
    {
        LOG_CLASS_ERROR("updateCerts()  CERTIFICATES NOT UPDATED. SSL connect() failed");

        GSMClient::stop();
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


    LOG_CLASS_INFO("listCertificates() -> Modem output copied to string-buf, length: %d", strlen(modemResponseBuf));
    // [WARNING] Do not print the following line in full, it may overflow the log buffer depending on the display
    LOG_CLASS_INFO("listCertificates() -> MODEM Response (truncated): %.100s", modemResponseBuf);

    // Normalizar CRLF y eliminar comillas
    normalizeOutputCrlfAndQuotationMarks(modemResponseBuf);

    // Dividir por líneas
    const char* lines[64];
    const size_t numLines = splitByNewlines(modemResponseBuf, lines, 64);

    LOG_CLASS_INFO("listCertificates() -> Cleaned input size: %d", strlen(modemResponseBuf));
    LOG_CLASS_INFO("listCertificates() -> Found ""%d" " certificate lines", numLines);

    std::vector<StoredCert> certs = {};
    certs.reserve(numLines);

    // Parsear cada línea CSV
    for (size_t i = 0; i < numLines; ++i)
    {
        const char* linePtr = lines[i];
        if (linePtr == nullptr || linePtr[0] == '\0') continue;
        StoredCert cert = buildStoredCert(linePtr);
        certs.push_back(cert);
        LOG_CLASS_INFO("listCertificates() -> Parsed line %d: %s", i, lines[i]);
    }

    LOG_CLASS_INFO("listCertificates() -> Total certificates parsed: %d", certs.size());

    return certs;
}

bool UBlox201_GSMSSLClient::removeClientCertificate(const char* name)
{
    MODEM.sendf("AT+USECMNG=2,1,\"%s\"", name);
    int res = MODEM.waitForResponse(3000);
    return logAndCheckResponse(res, "remove client certificate", name);
}

bool UBlox201_GSMSSLClient::removePrivateKey(const char* name)
{
    MODEM.sendf("AT+USECMNG=2,2,\"%s\"", name);
    const int res = MODEM.waitForResponse(3000);
    return logAndCheckResponse(res, "remove private key", name);
}

bool UBlox201_GSMSSLClient::removeRootCertificateByName(const char* name)
{
    MODEM.sendf("AT+USECMNG=2,0,\"%s\"", name);
    const int res = MODEM.waitForResponse(3000);
    return logAndCheckResponse(res, "remove root certificate", name);
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
    bool allOk = true;

    // Nombres codificados en ejecución, se crean en el stack de forma temporal
    const char* names[] = {
        "AmazonRootCA1",
        "BaltimoreCyberTrust",
        "COMODO RSA Certification Authority",
        "DigiCertGlobalRootCA",
        "DigiCertGlobalRootG2",
        "Digicert_Root",
        "EnTrust",
        "GeoTrustPrimaryCertificationAuthority-G3",
        "GlobalSignRootR1",
        "GoDaddyRootCertificateAuthority-G2",
        "ISRGRootX1",
        "MicrosoftRSARootCertificateAuthority2017",
        "VeriSign"
    };

    // Este bucle solo reserva memoria temporal en stack
    for (const char* name : names)
    {
        if (!removeRootCertificateByName(name))
            allOk = false;
    }
    return allOk;

    return removeTrustedRootCertificates(GSM_CUSTOM_ROOT_CERTS, GSM_CUSTOM_NUM_ROOT_CERTS);
}

bool UBlox201_GSMSSLClient::removeDefaultTrustedRoots()
{
    bool allOk = true;

    // Nombres codificados en ejecución, se crean en el stack de forma temporal
    const char* names[] = {
        "AddTrust_External_CA_Root",
        "Baltimore_CyberTrust_Root",
        "COMODO_RSA_Certification_Authority",
        "DST_Root_CA_X3",
        "DigiCert_High_Assurance_EV_Root_CA",
        "Entrust_Root_Certification_Authority",
        "Equifax_Secure_Certificate_Authority",
        "GeoTrust_Global_CA",
        "GeoTrust_Primary_Certification_Authority_G3",
        "GlobalSign",
        "Go_Daddy_Root_Certificate_Authority_G2",
        "VeriSign_Class_3_Public_Primary_Certification_Authority_G5",
        "Starfield_Services_Root_Certificate_Authority_G2"
    };

    // Este bucle solo reserva memoria temporal en stack
    for (const char* name : names)
    {
        if (!removeRootCertificateByName(name))
            allOk = false;
    }

    return allOk;
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
/**
 * =============================================================================
 *  ⚠️  WARNING — CONSTRUCTOR CONTEXT: NO VIRTUAL DISPATCH ALLOWED
 * -----------------------------------------------------------------------------
 *  This method is invoked *from the UBlox201_GSMSSLClient constructor*.
 *  Virtual dispatch is undefined during object construction, so any call
 *  from the current instanceto a virtual function of the parent (GSMSSLClient)
 *  must be avoided. Hence,  setTrustedRoot() is explicitly called using
 *  "GSMSSLClient::setTrustedRoot(...)" — this forces a non-virtual,
 *  base-class-qualified call, bypassing the vtable and ensuring safe behavior.
 * =============================================================================
 */
void UBlox201_GSMSSLClient::setTrustedRoots(const GSMRootCert* gsm_root_cert, const size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        GSMSSLClient::setTrustedRoot(gsm_root_cert[i].name);  // explicit non-virtual call
        LOG_CLASS_INFO(" -> Trusted root set: %s", gsm_root_cert[i].name);
    }
}


void UBlox201_GSMSSLClient::setSignedCertificate(const uint8_t* cert, const char* name, size_t size)
{
    return GSMSSLClient::setSignedCertificate(cert, name, size);
    if (!cert || !name || size == 0)
    {
        LOG_CLASS_ERROR("setSignedCertificate() -> Invalid input");
        return;
    }

    LOG_CLASS_INFO("Uploading client certificate: %s (%d bytes)", name, size);
    MODEM.sendf("AT+USECMNG=0,1,\"%s\",%d", name, size);

    // Esperar prompt '>'
    if (MODEM.waitForPrompt(5000) != 1)
    {
        LOG_CLASS_ERROR("setSignedCertificate() -> No prompt '>' from modem");
        return;
    }

    // Enviar datos y finalizar con retorno de carro
    MODEM.write(cert, size);
    MODEM.write('\r');

    const int res = MODEM.waitForResponse(10000);
    logAndCheckResponse(res, "client certificate upload", name);
}


void UBlox201_GSMSSLClient::setPrivateKey(const uint8_t* key, const char* name, size_t size)
{
    // return GSMSSLClient::setPrivateKey(key, name, size);

    if (!key || !name || size == 0)
    {
        LOG_CLASS_ERROR("setPrivateKey() -> Invalid input");
        return;
    }

    LOG_CLASS_INFO("Uploading private key: %s (%d bytes)", name, size);
    MODEM.sendf("AT+USECMNG=0,2,\"%s\",%d", name, size);

    // Esperar prompt '>'
    if (MODEM.waitForPrompt(5000) != 1)
    {
        LOG_CLASS_ERROR("setPrivateKey() -> No prompt '>' from modem");
        return;
    }

    // Enviar datos y CR final
    MODEM.write(key, size);

    const int res = MODEM.waitForResponse(5000);
    logAndCheckResponse(res, "private key upload", name);
}


bool UBlox201_GSMSSLClient::logAndCheckResponse(const int result, const char* action, const char* name)
{
    switch (result)
    {
    case 1:
        LOG_CLASS_INFO(" -> Successfully done [%s]: %s", action, name);
        return true;
    case 2:
        LOG_CLASS_ERROR(" -> AT command error while doing [%s]: %s", action, name);
        return false;
    case 3:
        LOG_CLASS_ERROR(" -> No carrier while doing [%s]: %s", action, name);
        return false;
    case -1:
    default:
        LOG_CLASS_ERROR(" -> Timeout while doing [%s]: %s", action, name);
        return false;
    }
}


bool UBlox201_GSMSSLClient::getClock(String& outTime, uint32_t* outUnix)
{
    LOG_CLASS_INFO("Querying modem clock...");

    String response;
    MODEM.send("AT+CCLK?");
    int result = MODEM.waitForResponse(2000, &response);

    if (result != 1)
    {
        LOG_CLASS_ERROR("getClock() -> No OK response from modem");
        return false;
    }

    // Ejemplo esperado: +CCLK: "25/11/07,15:30:00+00"
    int idx = response.indexOf("+CCLK:");
    if (idx < 0)
    {
        LOG_CLASS_ERROR("getClock() -> No +CCLK found in response: %s", response.c_str());
        return false;
    }

    int start = response.indexOf('"', idx);
    int end = response.indexOf('"', start + 1);
    if (start == -1 || end == -1)
    {
        LOG_CLASS_ERROR("getClock() -> Invalid +CCLK format: %s", response.c_str());
        return false;
    }

    outTime = response.substring(start + 1, end);
    LOG_CLASS_INFO("getClock() -> Modem time string: %s", outTime.c_str());

    if (outUnix)
    {
        const char* p = outTime.c_str(); // Formato: "yy/MM/dd,HH:mm:ss+ZZ"
        struct tm tm{};
        int tzQH = 0;

        // Helper lambda para parsear con strtol de forma segura
        auto parseField = [](const char* str, int& out, int base = 10) -> bool {
            errno = 0;
            char* end;
            long val = strtol(str, &end, base);
            if (end == str || errno == ERANGE) return false;
            out = static_cast<int>(val);
            return true;
        };

        bool ok =
            parseField(p + 0, tm.tm_year) &&
            parseField(p + 3, tm.tm_mon) &&
            parseField(p + 6, tm.tm_mday) &&
            parseField(p + 9, tm.tm_hour) &&
            parseField(p + 12, tm.tm_min) &&
            parseField(p + 15, tm.tm_sec) &&
            parseField(p + 18, tzQH);

        if (ok)
        {
            tm.tm_year += 100; // años desde 1900 → base 2000
            tm.tm_mon -= 1;

            time_t utc = mktime(&tm);
            *outUnix = utc - tzQH * 15 * 60; // tzQH = quarter-hours
            LOG_CLASS_INFO("getClock() -> Parsed UNIX time: %lu", static_cast<unsigned long>(*outUnix));
        }
        else
        {
            LOG_CLASS_WARNING("getClock() -> Failed to parse datetime string: %s", outTime.c_str());
        }
    }

    return true;
}

void UBlox201_GSMSSLClient::printTLSProfileStatus()
{
    LOG_CLASS_INFO("Querying full TLS profile 0 status (AT+USECPRF=0,<op_code>)...");

    const char* PARAM_NAMES[] = {
        "Certificate validation level",
        "Minimum TLS version",
        "Cipher suite",
        "Trusted root CA name",
        "Expected server hostname",
        "Client certificate name",
        "Client private key name",
        "Client private key password",
        "Pre-shared key",
        "Pre-shared key name",
        "SNI (Server Name Indication)"
    };

    String response;
    for (int op = 0; op <= 10; ++op)
    {
        response = "";
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "AT+USECPRF=0,%d", op);

        MODEM.send(cmd);
        const int result = MODEM.waitForResponse(1000, &response);

        if (result != 1)
        {
            LOG_CLASS_WARNING("No OK for op_code=%d", op);
            continue;
        }

        // Busca la línea +USECPRF:
        int start = response.indexOf("+USECPRF:");
        if (start == -1)
        {
            LOG_CLASS_WARNING("No response data for op_code=%d", op);
            continue;
        }

        const String line = response.substring(start);
        LOG_CLASS_INFO("[op=%02d] %-35s => %s",
                       op,
                       PARAM_NAMES[op],
                       line.c_str());
    }

    LOG_CLASS_INFO("End of TLS profile status report.");
}


bool UBlox201_GSMSSLClient::setClockFromUnix(uint32_t unixTime, int8_t tzQuarterHours)
{
    time_t t = unixTime;
    struct tm* utc = gmtime(&t);
    if (!utc)
    {
        LOG_CLASS_ERROR("setClockFromUnix() -> Invalid time conversion");
        return false;
    }

    char timeString[32];
    snprintf(
        timeString,
        sizeof(timeString),
        "%02d/%02d/%02d,%02d:%02d:%02d%+03d",
        (utc->tm_year + 1900) % 100,
        utc->tm_mon + 1,
        utc->tm_mday,
        utc->tm_hour,
        utc->tm_min,
        utc->tm_sec,
        tzQuarterHours
    );

    LOG_CLASS_INFO("Setting modem clock -> %s", timeString);

    MODEM.sendf("AT+CCLK=\"%s\"", timeString);
    const int result = MODEM.waitForResponse(2000);
    if (result == 1)
    {
        LOG_CLASS_INFO("setClockFromUnix() -> Clock successfully set");
        return true;
    }

    LOG_CLASS_ERROR("setClockFromUnix() -> Failed to set clock (CME or timeout)");
    return false;
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

    LOG_CLASS_INFO(" -> Reading up to %d bytes of HTTPS response...", MAX_RESPONSE_SIZE);

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
        LOG_CLASS_WARNING(" -> Response truncated to %d bytes", MAX_RESPONSE_SIZE);

    LOG_CLASS_INFO(" -> HTTPS response (%d bytes):\n%.1024s", totalRead, response);

    stop();
    LOG_CLASS_INFO(" -> Connection closed successfully.");

    return true;
}
#endif

#endif
