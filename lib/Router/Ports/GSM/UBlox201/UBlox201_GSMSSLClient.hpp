#ifndef UBLOX_201_GSM_SSL_CLIENT_HPP
#define UBLOX_201_GSM_SSL_CLIENT_HPP

#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)

#include <GSMSSLClient.h>
#include <vector>
#include <string>

#include "ClassName.h"
#include "Logger/Logger.h"
#include "TrustAnchors.h"

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
struct GSMCertificate{
    const char* name; // Nombre del certificado
    const uint8_t* data; // Datos del certificado (en formato binario)
    unsigned int size; // Tamaño del certificado
};

// Estructura para contener la clave privada
struct GSMPrivateKey{
    const char* name; // Nombre de la clave privada
    const uint8_t* data; // Datos de la clave privada (en formato binario)
    unsigned int size; // Tamaño de la clave privada
};

/**
 * https://kmpelectronics.eu/wp-content/uploads/2018/08/SARA-U2_ATCommands.pdf [PAGE 622 SPECIFICATION]
 */
class UBlox201_GSMSSLClient final : public GSMSSLClient{
    CLASS_NAME(MyGSMSSLClient)

public:
    // using GSMSSLClient::GSMSSLClient; // heredar constructores
    UBlox201_GSMSSLClient() : GSMSSLClient(){
        // llama al constructor del padre
        this->GSMSSLClient::setUserRoots(nullptr, 0);
        // this->GSMSSLClient::setUserRoots(reinterpret_cast<const GSMRootCert*>(trust_anchors), trust_anchors_num);
    }

    int connect(const char* host, uint16_t port) override{
        return GSMSSLClient::connect(host, port);
    }

    void setModemDebug(){ MODEM.debug(); }
    void setModemNoDebug(){ MODEM.noDebug(); }

    // 🔹 Helper centralizado
    static bool logAndCheckResponse(const int result, const std::string& action, const std::string& name){
        switch (result){
        case 1:
            LOG_CLASS_INFO("Removed %s: %s", action.c_str(), name.c_str());
            return true;
        case 2:
            LOG_CLASS_ERROR(" -> AT command error while removing %s: %s", action.c_str(), name.c_str());
            return false;
        case 3:
            LOG_CLASS_ERROR(" -> No carrier while removing %s: %s", action.c_str(), name.c_str());
            return false;
        case -1:
        default:
            LOG_CLASS_ERROR(" -> Timeout while removing %s: %s", action.c_str(), name.c_str());
            return false;
        }
    }


    [[nodiscard]] bool updateCerts(const GSMRootCert* rootCerts, const size_t rootCertsSize,
                                   const std::string& testBroker = "google.com"
    );

    [[nodiscard]] std::vector<StoredCert> listCertificates(CertType type);


    // 🔹 Eliminar un certificado de cliente
    [[nodiscard]] bool removeClientCertificate(const std::string& name);

    // 🔹 Eliminar una clave privada
    [[nodiscard]] bool removePrivateKey(const std::string& name);

    // 🔹 Eliminar un certificado raíz por nombre
    [[nodiscard]] bool removeRootCertificateByName(const std::string& name);

    // 🔹 Eliminar múltiples certificados raíz (vector<string>)
    [[nodiscard]] bool removeTrustedRootCertificates(const std::vector<std::string>& names);

    // 🔹 Eliminar múltiples certificados raíz (array GSMRootCert)
    [[nodiscard]] bool removeTrustedRootCertificates(const GSMRootCert* certs, size_t count);

    // 🔹 Eliminar todos los certificados raíz por defecto (trust_anchors)
    [[nodiscard]] bool removeDefaultTrustedRoots();

    // 🔹 Importar un certificado raíz
    void importTrustedRoot(const uint8_t* cert, const char* name, size_t size);

    // 🔹 Importar múltiples certificados raíz
    void importTrustedRoots(const GSMRootCert* certs, size_t count);

    // 🔹 Establecer múltiples certificados raíz de confianza para la conexión SSL
    void setTrustedRoots(const GSMRootCert* gsm_root_cert, const size_t size);
};

#endif // ARDUINO

#endif //MYGSMSSLCLIENT_HPP
