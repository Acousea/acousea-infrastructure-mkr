#ifndef MYSSLCLIENT_HPP
#define MYSSLCLIENT_HPP

#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)

#include <GSMSSLClient.h>
#include <vector>
#include <string>

#include "ClassName.h"
#include "Logger/Logger.h"

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

// Estructura para contener el certificado
struct GSMCertificate {
    const char* name;        // Nombre del certificado
    const uint8_t* data;     // Datos del certificado (en formato binario)
    unsigned int size;                // Tamaño del certificado
};

// Estructura para contener la clave privada
struct GSMPrivateKey {
    const char* name;        // Nombre de la clave privada
    const uint8_t* data;     // Datos de la clave privada (en formato binario)
    unsigned int size;                // Tamaño de la clave privada
};

/**
 * https://kmpelectronics.eu/wp-content/uploads/2018/08/SARA-U2_ATCommands.pdf [PAGE 622 SPECIFICATION]
 */
class MyGSMSSLClient final : public GSMSSLClient{
    CLASS_NAME(MyGSMSSLClient)

public:
    void updateCerts(const GSMRootCert* rootCerts, const size_t rootCertsSize);
    void useCertificateWithPrivateKey(const GSMCertificate& cert, const GSMPrivateKey& key);
    std::vector<StoredCert> listCertificates(CertType type);
    // 🔹 Hook para inyectar SNI antes de conectar
    void applySNI(const std::string& host){
        if (host.empty()){
            Logger::logWarning(getClassNameString() + " -> applySNI() called with empty host");
            return;
        }
        Logger::logInfo(getClassNameString() + " -> Setting SNI: " + host);
        MODEM.sendf("AT+USECPRF=0,10,\"%s\"", host.c_str());
        MODEM.waitForResponse(5000);
    }

    void getCurrentlyConfiguredSNI(){
        Logger::logInfo(getClassNameString() + " -> Querying SNI value...");
        String responseDataStorage;
        MODEM.setResponseDataStorage(&responseDataStorage);
        MODEM.send("AT+USECPRF=0,10"); // perfil 0, op_code 10 = SNI
        if (MODEM.waitForResponse(5000) == 1){
            Logger::logInfo(getClassNameString() + " -> Current SNI: " + std::string(responseDataStorage.c_str()));
        }
        else{
            Logger::logError(getClassNameString() + " -> Failed to query SNI");
        }
    }


    // Sobrescribir connectSSL para aplicar SNI automáticamente
    int connect(const char* host, uint16_t port) override{
        // applySNI(host); // mandamos el comando AT
        // getCurrentlyConfiguredSNI();
        return GSMSSLClient::connect(host, port);
    }
};

#endif // ARDUINO

#endif //MYSSLCLIENT_HPP
