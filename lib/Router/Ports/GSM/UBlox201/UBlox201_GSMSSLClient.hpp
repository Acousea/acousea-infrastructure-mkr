#ifndef UBLOX_201_GSM_SSL_CLIENT_HPP
#define UBLOX_201_GSM_SSL_CLIENT_HPP

#if defined(ARDUINO) && defined(PLATFORM_HAS_GSM)

#define OVERRIDE_DEFAULT_GSMROOT_CERTS_H // Uncomment to use custom trust anchors

#ifdef OVERRIDE_DEFAULT_GSMROOT_CERTS_H
#warning "Using custom root certificates for GSM, default GSMRootCerts will NOT be included."
#include "TrustAnchors.h" // include default trust anchors (MUST be included BEFORE GSMSSLClient.h)
#endif


#include <GSMSSLClient.h>
#include <vector>
#include "ClassName.h"
#include "GPRS.h"
#include "Logger/Logger.h"
#include "Ports/GSM/GsmConfig.hpp"
#include "wait/WaitFor.hpp"


enum class CertType : uint8_t
{
    RootCA = 0, // 0: trusted root CA (certificate authority) certificate
    ClientCert = 1, // 1: client certificate
    PrivateKey = 2, // 2: client private key,
    All = 3,
    Unknown = 4
};

struct StoredCert
{
    char type[8];
    char internalName[64];
    char commonName[64];
    char expiration[32];
};

// Estructura para contener el certificado
struct GSMUserCertificate
{
    const char* name; // Nombre del certificado
    const uint8_t* data; // Datos del certificado (en formato binario)
    unsigned int size; // Tamaño del certificado
};

// Estructura para contener la clave privada
struct GSMPrivateKey
{
    const char* name; // Nombre de la clave privada
    const uint8_t* data; // Datos de la clave privada (en formato binario)
    unsigned int size; // Tamaño de la clave privada
};


/**
 * https://kmpelectronics.eu/wp-content/uploads/2018/08/SARA-U2_ATCommands.pdf [PAGE 622 SPECIFICATION]
 */
class UBlox201_GSMSSLClient final : public GSMSSLClient
{
    CLASS_NAME(UBlox201_GSMSSLClient)
    GSM gsmAccess;
    GPRS gprs;

public:
    UBlox201_GSMSSLClient() : GSMSSLClient()
    {
        GSMSSLClient::setUserRoots(GSM_CUSTOM_ROOT_CERTS, GSM_CUSTOM_NUM_ROOT_CERTS);
    }

    [[nodiscard]] bool init(const GsmConfig& config);

    unsigned long getTime();

    int connect(const char* host, uint16_t port) override
    {
        return GSMSSLClient::connect(host, port);
    }

    static void setModemDebug() { MODEM.debug(); }
    static void setModemNoDebug() { MODEM.noDebug(); }

    // 🔹 Helper centralizado
    static bool logAndCheckResponse(int result, const char* action, const char* name);


    // 🔹 Actualizar certificados raíz de confianza
    [[nodiscard]] bool updateCerts(const GSMRootCert* rootCerts, size_t rootCertsSize,
                                   const char* testBroker = "google.com");

    // 🔹 Listar certificados almacenados
    [[nodiscard]] static std::vector<StoredCert> listCertificates(CertType type);


    // // 🔹 Eliminar un certificado de cliente
    [[nodiscard]] static bool removeClientCertificate(const char* name);

    // 🔹 Eliminar una clave privada
    [[nodiscard]] static bool removePrivateKey(const char* name);

    // 🔹 Eliminar un certificado raíz por nombre
    [[nodiscard]] static bool removeRootCertificateByName(const char* name);

    // 🔹 Eliminar múltiples certificados raíz (vector<string>)
    [[nodiscard]] static bool removeTrustedRootCertificates(const std::vector<char*>& names);

    // 🔹 Eliminar múltiples certificados raíz (array GSMRootCert)
    [[nodiscard]] static bool removeTrustedRootCertificates(const GSMRootCert* certs, size_t count);

    // 🔹 Eliminar todos los certificados raíz custom (trust_anchors)
    [[nodiscard]] static bool removeCustomTrustedRoots();

    // 🔹 Eliminar todos los certificados raíz por defecto (defined in GSM_ROOT_CERTS))
    [[nodiscard]] static bool removeDefaultTrustedRoots();

    // 🔹 Importar un certificado raíz
    static void importTrustedRoot(const uint8_t* cert, const char* name, size_t size);

    // 🔹 Importar múltiples certificados raíz
    static void importTrustedRoots(const GSMRootCert* certs, size_t count);

    // 🔹 Establecer múltiples certificados raíz de confianza para la conexión SSL
    void setTrustedRoots(const GSMRootCert* gsm_root_cert, size_t size);

    // 🔹 Establecer certificado de cliente y clave privada
    void setSignedCertificate(const uint8_t* cert, const char* name, size_t size) override;

    // 🔹 Establecer clave privada de cliente
    void setPrivateKey(const uint8_t* key, const char* name, size_t size) override;

    [[nodiscard]] static bool setClockFromUnix(uint32_t unixTime, int8_t tzQuarterHours = 0);

    [[nodiscard]] static bool getClock(String& outTime, uint32_t* outUnix = nullptr);

    static void printTLSProfileStatus();


#if ENVIRONMENT == ENV_TEST
    // 🔹 Probar conexión HTTPS/TLS a un host y puerto dados
    [[nodiscard]] bool testTLSConnection(const char* host, uint16_t port, const char* path = "/");
#endif
};

#endif // ARDUINO

#endif //MYGSMSSLCLIENT_HPP
