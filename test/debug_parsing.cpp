#include <iostream>


#include <vector>
#include <string>
#include <algorithm> // std::remove
#include <cstdint>

#define String std::string


String modemOutput;

enum class CertType : uint8_t{
    RootCA = 0, // 0: trusted root CA (certificate authority) certificate
    ClientCert = 1, // 1: client certificate
    PrivateKey = 2, // 2: client private key,
    All = 3,
    Unknown = 4
};

struct StoredCert{
    std::string type;
    std::string internalName;
    std::string commonName;
    std::string expiration;
};


inline void logInfo(const std::string& msg){
    std::cout << "[INFO] " << msg << std::endl;
}

inline void logWarning(const std::string& msg){
    std::cout << "[WARNING] " << msg << std::endl;
}

inline void logError(const std::string& msg){
    std::cout << "[ERROR] " << msg << std::endl;
}

struct Logger{
    static constexpr auto logInfo = ::logInfo;
    static constexpr auto logWarning = ::logWarning;
    static constexpr auto logError = ::logError;
};


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

// Split by newlines, normalize CRLF, compact, and return non-empty lines
std::vector<std::string> splitByNewlines(const std::string& input){
    std::string clean = input;

    // Remove quotes
    clean.erase(std::remove(clean.begin(), clean.end(), '"'), clean.end());

    // Normalize CRLF -> LF
    for (size_t pos = 0; (pos = clean.find("\r\n", pos)) != std::string::npos;)
        clean.replace(pos, 2, "\n");

    // Compact double LF
    for (size_t pos = 0; (pos = clean.find("\n\n")) != std::string::npos;)
        clean.replace(pos, 2, "\n");

    std::vector<std::string> lines;
    size_t start = 0;
    const size_t length = clean.size();
    while (start < length){
        size_t end = clean.find('\n', start);
        if (end == std::string::npos) end = length;

        if (end > start){
            // skip empty lines
            std::string line = clean.substr(start, end - start);
            lines.push_back(line);
            Logger::logInfo("splitByNewlines() -> Line: " + line);
        }
        start = end + 1;
    }

    Logger::logInfo("splitByNewlines() -> Total lines parsed: " + std::to_string(lines.size()));
    return lines;
}

// Build a StoredCert from a single CSV-like line
StoredCert buildStoredCert(const std::string& input){
    std::vector<std::string> parts;
    size_t start = 0;
    const size_t length = input.size();

    while (start <= length){
        size_t end = input.find(',', start);
        if (end == std::string::npos) end = length;

        std::string token = input.substr(start, end - start);
        token.erase(std::remove(token.begin(), token.end(), '"'), token.end());
        parts.push_back(token);

        start = end + 1;
    }

    // Ensure 4 fields (pad with "")
    while (parts.size() < 4) parts.emplace_back("");

    StoredCert cert{parts[0], parts[1], parts[2], parts[3]};

    Logger::logInfo("buildStoredCert() -> Parsed cert line -"
        " Type: " + cert.type +
        ", Name: " + cert.internalName +
        ", CN: " + cert.commonName +
        ", Expiration: " + cert.expiration
    );

    return cert;
}


int main(){
    std::string modemResponse = R"(CA,"DigiCertGlobalRootG2","DigiCert Global Root G2","2038/01/15 12:00:00"

CA,"Microsoft RSA Root Certificate Authority 2017","Microsoft RSA Root Certificate Authority 2017","2042/07/26 22:15:47"

CA,"COMODO_RSA_Certification_Authority","COMODO RSA Certification Authority","2038/01/18 23:59:59"

CA,"DigiCert_High_Assurance_EV_Root_CA","DigiCert High Assurance EV Root CA","2031/11/10 00:00:00"

CA,"Entrust_Root_Certification_Authority","Entrust Root Certification Authority","2026/11/27 20:53:42"

CA,"GeoTrust_Primary_Certification_Authority_G3","GeoTrust Primary Certification Authority - G3","2037/12/01 23:59:59"

CA,"GlobalSign","GlobalSign Root CA","2028/01/28 12:00:00"

CA,"Go_Daddy_Root_Certificate_Authority_G2","Go Daddy Root Certificate Authority - G2","2037/12/31 23:59:59"

CA,"VeriSign_Class_3_Public_Primary_Certification_Authority_G5","VeriSign Class 3 Public Primary Certification Authority - G5","2036/07/16 23:59:59"

CA,"Starfield_Services_Root_Certificate_Authority_G2","Starfield Services Root Certificate Authority - G2","2037/12/31 23:59:59"

CA,"ISRG_Root_X1","ISRG Root X1","2035/06/04 11:04:38"

CA,"GTS_Root_R1","GTS Root R1","2036/06/22 00:00:00"

CA,"R2_Internet_Security_Research_Group","R12","2027/03/12 23:59:59"

CA,"AMAZON_ROOT_CA","Amazon Root CA 1","2038/01/17 00:00:00"

CC,"my_device_cert","AWS IoT Certificate","2049/12/31 23:59:59"

CA,"AddTrust_External_CA_Root","AddTrust External CA Root","2020/05/30 10:48:38")";

    // Dividir en líneas
    const std::vector<std::string> lines = splitByNewlines(modemResponse);

    // Parsear cada línea en StoredCert
    std::vector<StoredCert> certs;
    certs.reserve(lines.size());
    for (const auto& line : lines){
        certs.push_back(buildStoredCert(line));
    }

    // Mostrar resultado
    std::cout << "Total certificates parsed: " << certs.size() << "\n";
    for (const auto& cert : certs){
        std::cout << "Type: " << cert.type
            << " | Name: " << cert.internalName
            << " | CN: " << cert.commonName
            << " | Exp: " << cert.expiration << "\n";
    }

    return 0;
}
