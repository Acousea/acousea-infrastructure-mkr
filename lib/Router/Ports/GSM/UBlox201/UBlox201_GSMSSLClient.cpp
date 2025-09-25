#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)

#include "UBlox201_GSMSSLClient.hpp"
#include <Arduino.h>

#include <vector>
#include <string>


inline CertType certTypeFromInt(int type){
    switch (type){
    case 0: return CertType::RootCA;
    case 1: return CertType::ClientCert;
    case 2: return CertType::PrivateKey;
    default: return CertType::All;
    }
}

inline CertType certTypeFromString(const std::string& typeStr){
    if (typeStr == "CA") return CertType::RootCA;
    if (typeStr == "CC") return CertType::ClientCert;
    if (typeStr == "PK") return CertType::PrivateKey;
    return CertType::Unknown; // mejor que All si es un error
}

/**
 * Copies only \n characters once to normalize CRLF to LF and remove duplicate LFs.
*/
std::string& normalizeOutputCrlfAndQuotationMarks(std::string& input){
    Logger::logInfo("normalizeCrlf() -> Normalizing input of size: " + std::to_string(input.size()));

    size_t writeIndex = 0;
    bool lastWasNewline = false;

    for (size_t readIndex = 0; readIndex < input.size(); ++readIndex){
        char c = input[readIndex];

        if (c == '\r' || c == '"'){
            continue; // ignorar CR y comillas
        }

        if (c == '\n'){
            if (lastWasNewline){
                continue; // evitar saltos de línea duplicados
            }
            lastWasNewline = true;
        } else {
            lastWasNewline = false;
        }

        input[writeIndex++] = c; // sobrescribe en la misma cadena
    }

    input.resize(writeIndex); // recortar al nuevo tamaño
    return input; // devolver referencia al mismo string
}


// Split by newlines, normalize CRLF, compact, and return non-empty lines
std::vector<std::string> splitByNewlines(const std::string& normalizedCleanInput){
    std::vector<std::string> lines;
    std::string current;
    current.reserve(128); // para evitar muchas realocaciones

    Logger::logInfo("splitByNewlines() -> Cleaned input size: " + std::to_string(normalizedCleanInput.size()));

    for (const char c : normalizedCleanInput){
        if (c != '\n'){
            current.push_back(c);
            continue;
        }
        lines.push_back(current);
        Logger::logInfo("splitByNewlines() -> Line: " + current);
        current.clear();
    }

    // añadir la última línea si no terminó en '\n'
    if (!current.empty()){
        lines.push_back(current);
        Logger::logInfo("splitByNewlines() -> Line: " + current);
    }

    Logger::logInfo("splitByNewlines() -> Total lines parsed: " + std::to_string(lines.size()));
    return lines;
}

// Build a StoredCert from a single CSV-like line
StoredCert buildStoredCert(const std::string& input){
    std::vector<std::string> parts;
    std::string current;
    current.reserve(64); // reservar memoria inicial

    for (const char c : input){
        if (c == ','){
            // fin de campo → guardamos
            parts.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(c);
    }
    // último campo
    parts.push_back(current);

    // asegurar 4 campos
    while (parts.size() < 4){
        parts.emplace_back("");
    }

    StoredCert cert{parts[0], parts[1], parts[2], parts[3]};

    Logger::logInfo("buildStoredCert() -> Parsed cert line -"
        " Type: " + cert.type +
        ", Name: " + cert.internalName +
        ", CN: " + cert.commonName +
        ", Expiration: " + cert.expiration
    );

    return cert;
}


bool UBlox201_GSMSSLClient::updateCerts(
    const GSMRootCert* rootCerts, const size_t rootCertsSize, const std::string& testBroker /* = "google.com" */
){
    Logger::logInfo(getClassNameString() + " -> Loading new trusted root CAs...");
    this->eraseTrustedRoot(); // Erase the currently existing roots
    this->setUserRoots(rootCerts, rootCertsSize);
    for (size_t i = 0; i < rootCertsSize; i++){
        const GSMRootCert& rootCert = rootCerts[i];
        this->importTrustedRoot(rootCert.data, rootCert.name, rootCert.size);
        this->setTrustedRoot(rootCert.name);
        Logger::logInfo(getClassNameString() + " -> Trusted root CA added: " + rootCert.name
        );
    }

    Logger::logInfo(getClassNameString() + " -> Verifying by connecting to " + testBroker + ":443");
    if (!this->connect(testBroker.c_str(), 443)){
        Logger::logError(getClassNameString() + "updateCerts()  CERTIFICATES NOT UPDATED. SSL connect() failed");
        this->stop();
        return false;
    }
    Logger::logInfo(getClassNameString() + "updateCerts() -> CERTIFICATES UPDATED");
    return true;
}


std::vector<StoredCert> UBlox201_GSMSSLClient::listCertificates(CertType type){
    Logger::logInfo(getClassNameString() + "listCertificates() -> Listing certificates of type: " +
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

    if (type == CertType::All || type == CertType::Unknown){
        MODEM.send("AT+USECMNG=3"); // listar todos
    }
    else{
        MODEM.sendf("AT+USECMNG=3,%d", static_cast<int>(type));
    }


    String modemOutput;
    if (MODEM.waitForResponse(5000, &modemOutput) != 1){
        Logger::logError(getClassNameString() + "listCertificates() -> No response from modem");
        return {}; // vacío
    }

    Logger::logInfo(
        getClassNameString() + "listCertificates() -> Modem output length: " + std::to_string(modemOutput.length()));

    std::string response(modemOutput.c_str());

    Logger::logInfo(getClassNameString() + "listCertificates() -> Modem output copied to std::string, length: " +
        std::to_string(response.length())
    );

    // FIXME: [WARNING] Do not print the following line in full, it may overflow the log buffer depending on the display
    Logger::logInfo(getClassNameString() + "listCertificates() -> MODEM Response: " + response);

    // saltar líneas vacías
    if (modemOutput.length() == 0){
        Logger::logWarning(getClassNameString() + "listCertificates() -> Empty response");
        return {};
    }

    const auto normalizedCleanResponse = normalizeOutputCrlfAndQuotationMarks(response);

    Logger::logInfo(getClassNameString() +
        "listCertificates() -> Cleaned input size: " + std::to_string(normalizedCleanResponse.size())
    );

    const std::vector<std::string> certStrLines = splitByNewlines(normalizedCleanResponse);
    Logger::logInfo(getClassNameString() + "listCertificates() -> Found " + std::to_string(certStrLines.size()) +
        " certificate lines"
    );

    std::vector<StoredCert> certs = {};
    for (const auto& line : certStrLines){
        Logger::logInfo(getClassNameString() + "listCertificates() -> Line: " + line);
        StoredCert cert = buildStoredCert(line);
        certs.push_back(cert);
    }

    Logger::logInfo(
        getClassNameString() + "listCertificates() -> Total certificates parsed: " + std::to_string(certs.size()));


    return certs;
}

bool UBlox201_GSMSSLClient::removeClientCertificate(const std::string& name){
    MODEM.sendf("AT+USECMNG=2,1,\"%s\"", name.c_str());
    int res = MODEM.waitForResponse(3000);
    return logAndCheckResponse(res, "client certificate", name);
}

bool UBlox201_GSMSSLClient::removePrivateKey(const std::string& name){
    MODEM.sendf("AT+USECMNG=2,2,\"%s\"", name.c_str());
    int res = MODEM.waitForResponse(3000);
    return logAndCheckResponse(res, "private key", name);
}

bool UBlox201_GSMSSLClient::removeRootCertificateByName(const std::string& name){
    MODEM.sendf("AT+USECMNG=2,0,\"%s\"", name.c_str());
    int res = MODEM.waitForResponse(3000);
    return logAndCheckResponse(res, "root certificate", name);
}

bool UBlox201_GSMSSLClient::removeTrustedRootCertificates(const std::vector<std::string>& names){
    bool allOk = true;
    for (const auto& name : names){
        if (!removeRootCertificateByName(name)){
            allOk = false; // seguimos, pero marcamos fallo
        }
    }
    return allOk;
}

bool UBlox201_GSMSSLClient::removeTrustedRootCertificates(const GSMRootCert* certs, size_t count){
    bool allOk = true;
    for (size_t i = 0; i < count; ++i){
        std::string name(certs[i].name);
        if (!removeRootCertificateByName(name)){
            allOk = false;
        }
    }
    return allOk;
}

bool UBlox201_GSMSSLClient::removeDefaultTrustedRoots(){
    return removeTrustedRootCertificates(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
}

void UBlox201_GSMSSLClient::importTrustedRoot(const uint8_t* cert, const char* name, size_t size){
    MODEM.sendf("AT+USECMNG=0,0,\"%s\",%d", name, size);
    int res = MODEM.waitForResponse(1000);
    logAndCheckResponse(res, "root certificate import (prepare)", name);

    MODEM.write(cert, size);
    res = MODEM.waitForResponse(2000);
    logAndCheckResponse(res, "root certificate import (write)", name);
}

void UBlox201_GSMSSLClient::importTrustedRoots(const GSMRootCert* certs, size_t count){
    for (size_t i = 0; i < count; ++i){
        importTrustedRoot(certs[i].data, certs[i].name, certs[i].size);
    }
}

void UBlox201_GSMSSLClient::setTrustedRoots(const GSMRootCert* gsm_root_cert, const size_t size){
    for (size_t i = 0; i < size; ++i){
        this->setTrustedRoot(gsm_root_cert[i].name);
        Logger::logInfo(getClassNameString() + " -> Trusted root set: " + std::string(gsm_root_cert[i].name));
    }
}


#endif
