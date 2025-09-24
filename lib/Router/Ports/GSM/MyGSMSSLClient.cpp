#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)

#include "MyGSMSSLClient.hpp"

#include <vector>
#include <string>
#include <algorithm> // std::remove


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


std::vector<std::string> splitByNewlines(const std::string& input){
    // Copia limpia sin comillas
    std::string cleanInput = input;
    cleanInput.erase(
        std::remove(cleanInput.begin(), cleanInput.end(), '"'),
        cleanInput.end()
    );

    std::vector<std::string> lines;
    lines.reserve(16);

    size_t start = 0;
    const size_t length = cleanInput.size();

    while (start < length){
        size_t currentLineEnd = cleanInput.find("\r\n", start);
        if (currentLineEnd == std::string::npos){
            Logger::logWarning(
                "splitByNewlines() -> No additional CRLF found, taking rest of string."
                " Current start: " + std::to_string(start) + ", length: " + std::to_string(length));
            currentLineEnd = length;
        }

        std::string internalLine = cleanInput.substr(start, currentLineEnd - start);

        // if (!internalLine.empty() && internalLine.back() == '\r'){
        //     internalLine.pop_back(); // eliminar LF si está al final
        // }

        if (!internalLine.empty()){
            lines.push_back(std::move(internalLine));
        }

        start = currentLineEnd + 2;
    }

    return lines;
}

StoredCert buildStoredCert(const std::string& input){
    std::vector<std::string> certLineParts;
    size_t start = 0;
    const size_t length = input.size();

    while (start <= length){
        size_t end = input.find(',', start);
        if (end == std::string::npos){
            end = length;
        }

        std::string certToken = input.substr(start, end - start);
        certLineParts.push_back(certToken);
        start = end + 1;
    }

    if (certLineParts.size() <= 3){
        Logger::logWarning("buildStoredCert() -> Invalid cert line, expected at least 3 parts: " + input);
        return StoredCert{
            .type = "",
            .internalName = "",
            .commonName = "",
            .expiration = ""
        };
    }

    return StoredCert{
        .type = certLineParts[0],
        .internalName = certLineParts[1],
        .commonName = certLineParts[2],
        .expiration = certLineParts[3]
    };
}

void MyGSMSSLClient::updateCerts(const GSMRootCert* rootCerts, const size_t rootCertsSize){
    this->eraseTrustedRoot();
    this->setUserRoots(GSM_ROOT_CERTS, std::size(GSM_ROOT_CERTS));
    for (size_t i = 0; i < rootCertsSize; i++){
        this->setTrustedRoot(rootCerts[i].name);
    }
    if (this->connect("google.com", 443)){
        Logger::logInfo(getClassNameString() + "GsmPort::updateCerts() -> CERTIFICATES UPDATED");
    }
    else{
        // forzar carga
        Logger::logError(getClassNameString() + " -> CERTIFICATES NOT UPDATED. SSL connect() failed");
    }
}


void MyGSMSSLClient::useCertificateWithPrivateKey(const GSMCertificate& cert, const GSMPrivateKey& key){
    // Llamar a las funciones correspondientes para establecer el certificado y la clave privada
    this->setSignedCertificate(cert.data, cert.name, cert.size); // Establecer el certificado
    Logger::logInfo(getClassNameString() + " -> Device certificate set: " + cert.name);
    this->setPrivateKey(key.data, key.name, key.size); // Establecer la clave privada
    Logger::logInfo(
        getClassNameString() + " -> Device certificate and private key set: " + cert.name + ", " + key.name);
    this->useSignedCertificate(cert.name); // Usar el certificado
    Logger::logInfo(getClassNameString() + " -> Device certificate set: " + cert.name);
    this->usePrivateKey(key.name); // Usar la clave privada
    Logger::logInfo(
        getClassNameString() + " -> Device certificate and private key set: " + cert.name + ", " + key.name);
}


std::vector<StoredCert> MyGSMSSLClient::listCertificates(CertType type){
    if (type == CertType::All || type == CertType::Unknown){
        MODEM.send("AT+USECMNG=3"); // listar todos
    }
    else{
        MODEM.sendf("AT+USECMNG=3,%d", static_cast<int>(type));
    }

    std::vector<StoredCert> certs;
    String modemOutput;
    if (MODEM.waitForResponse(5000, &modemOutput) != 1){
        Logger::logError(getClassNameString() + "listCertificates() -> No response from modem");
        return certs; // vacío
    }
    // quitamos CR/LF sobrantes
    modemOutput.trim();
    // Logger::logInfo(getClassNameString() + "listCertificates() -> MODEM Response: " + modemOutput.c_str());

    // saltar líneas vacías
    if (modemOutput.length() == 0){
        Logger::logWarning(getClassNameString() + "listCertificates() -> Empty response");
        return certs;
    }

    const std::vector<std::string> certStrLines = splitByNewlines(modemOutput.c_str());
    Logger::logInfo(
        getClassNameString() + "listCertificates() -> Found " + std::to_string(certStrLines.size()) +
        " certificate lines"
    );

    for (const auto& line : certStrLines){
        // Logger::logInfo(getClassNameString() + "listCertificates() -> Line: " + line);
        StoredCert cert = buildStoredCert(line);
        certs.push_back(cert);
    }

    Logger::logInfo(
        getClassNameString() + "listCertificates() -> Total certificates parsed: " + std::to_string(certs.size()));

    return certs;
}

#endif
