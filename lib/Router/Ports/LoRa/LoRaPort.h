#ifndef LORA_PORT_H
#define LORA_PORT_H

#if defined(PLATFORM_ARDUINO)&& defined(PLATFORM_HAS_LORA)

#include <deque>

#include "ClassName.h"
#include "LoRa.h"
#include "Ports/IPort.h"

typedef struct
{
    uint8_t bandwidth_index;
    // Ancho de banda [0,9]: Como norma general, multiplicarlo por dos supone reducir a la mitad el tiempo de transmisión.
    // Reducir el ancho de banda permite reducir el impacto de interferencias y obtener distancias de enlace mayores.
    uint8_t spreadingFactor;
    // Factor de dispersión [6,12]: Aumentar el Spf permite hacer que la señal aumente su inmunidad al ruido y puedan
    // lograrse distancias de enlace mayores. Sin embargo, el uso de factores de dispersión mayores incrementa de forma
    // muy significativa el tiempo de transmisión de un dato, lo que puede provocar un aumento importante en el consumo
    // de energía o exceder el tiempo actividad o duty-cycle permitido.
    uint8_t codingRate;
    // Tasa de codificación [5, 8]:  Esta estrategia permite detectar y corregir, hasta cierto punto, los errores que
    // se produzcan en la recepción. Como contrapartida, aumentar la tasa de codificación incrementa el número de símbolos
    // a transmitir y, por tanto, el tiempo de transmisión
    uint8_t txPower;
    // Potencia de transmisión [2,20]: Potencia deseada expresada en dBm. . Para pruebas en interior es aconsejable poner
    // poner este valor al mínimo y evitar así saturar los receptores, ya que de lo contrario fallarán las transmisiones.
    long frequency; // Frecuencia de operación
    uint8_t preambleLength; // Longitud del preámbulo
    uint8_t syncWord; // Palabra de sincronización
} LoRaConfig;

// Valores de ancho de banda en kHz
extern double bandwidth_kHz[10];

extern LoRaConfig defaultLoraConfig;

// Prototipo del callback
void onReceiveWrapper(int packetSize);

class LoraPort : public IPort
{
    CLASS_NAME(LoraPort)

public:
    LoraPort(FlashPacketQueue& flashQueue, const LoRaConfig& config = defaultLoraConfig);

    void init() override;

    bool send(const uint8_t* data, size_t length) override;

    bool available() override;

    uint16_t readInto(uint8_t* buffer, uint16_t maxSize) override;

    bool sync() override;

    void onReceive(int packetSize);

private:
    const LoRaConfig& config;

    void configureLora(const LoRaConfig& config);
};


#endif // ARDUINO

#endif // LORA_PORT_H
