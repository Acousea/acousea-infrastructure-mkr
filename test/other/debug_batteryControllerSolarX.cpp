#include <iostream>
#include <cmath>
#include <string>
#include <chrono>
#include <thread>

// ---------------------------------------------------------
// MOCKS Y UTILIDADES
// ---------------------------------------------------------

// Mock de constrain (estilo Arduino)
template <typename T>
T constrain(T val, T min_val, T max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

// Mock de millis()
unsigned long millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

// Mock del logger
struct Logger {
    static void logInfo(const std::string& msg) { std::cout << "[INFO] " << msg << std::endl; }
    static void logError(const std::string& msg) { std::cerr << "[ERROR] " << msg << std::endl; }
};

// Mock de LinearCurve: mapea %↔voltaje lineal
class LinearCurve {
    float slope;
    float offset;

public:
    LinearCurve(float slope_, float offset_) : slope(slope_), offset(offset_) {}

    // Dado un porcentaje [0–100], devuelve el voltaje
    float direct(float percentage) const { return offset + slope * percentage; }

    // Dado un voltaje, devuelve el porcentaje equivalente
    float inverse(float voltage) const { return (voltage - offset) / slope; }
};

// Mock del enum acousea_BatteryStatus
enum acousea_BatteryStatus {
    acousea_BatteryStatus_BATTERY_STATUS_ERROR = 0,
    acousea_BatteryStatus_BATTERY_STATUS_DISCHARGING = 1,
    acousea_BatteryStatus_BATTERY_STATUS_CHARGING = 2,
    acousea_BatteryStatus_BATTERY_STATUS_IDLE = 3
};

// ---------------------------------------------------------
// MOCK DE TEST CONTROLLER
// ---------------------------------------------------------

class TestSolarXBatteryController {
    struct VoltageRange {
        float min;
        float max;
    };

private:
    bool _initialized = true;

    // Parámetros físicos
    static constexpr float NOMINAL_CAPACITY_AH = 14.0f;
    static constexpr uint8_t NUM_CELLS = 6;
    static constexpr VoltageRange VOLTAGE_RANGE = {1.92f * NUM_CELLS, 2.15f * NUM_CELLS};
    static constexpr float R_INTERNAL = 0.8f;
    static constexpr float SOC_CORRECTION_ALPHA = 0.02f;
    static constexpr float QUIET_CURRENT_THRESHOLD = 0.05f;
    static constexpr float VOLTAGE_FILTER_ALPHA = 0.1f;

    static const LinearCurve voltageToPercentageCurve;

    // Estado interno
    float _soc = 70.0f;
    float _filteredVoltage = 12.4f;
    unsigned long _lastUpdate = 0;

    // Valores simulados (mock)
    float _simVoltage = 12.5f;
    float _simCurrent = 0.0f;  // A

public:
    TestSolarXBatteryController() { _lastUpdate = millis(); }

    // Simulación de medidas
    float batteryVolts() { return _simVoltage; }
    float batteryCurrentAmp() { return _simCurrent; }

    // Simula un entorno dinámico (carga/descarga)
    void simulateStep(float loadCurrent, float deltaVoltage) {
        _simCurrent = loadCurrent;
        _simVoltage += deltaVoltage;  // se actualiza lentamente según carga
    }

    // Implementación de accuratePercentage()
    double accuratePercentage();
};

// Definición estática
const LinearCurve TestSolarXBatteryController::voltageToPercentageCurve(
    (VOLTAGE_RANGE.max - VOLTAGE_RANGE.min) / 100.0f,
    VOLTAGE_RANGE.min
);

// ---------------------------------------------------------
// IMPLEMENTACIÓN accuratePercentage()
// ---------------------------------------------------------
double TestSolarXBatteryController::accuratePercentage() {
    if (!_initialized) {
        Logger::logError("Sensor not initialized.");
        return 0;
    }

    const float I = batteryCurrentAmp();
    const float Vraw = batteryVolts();

    // 1. Filtro exponencial del voltaje
    _filteredVoltage = _filteredVoltage * (1.0f - VOLTAGE_FILTER_ALPHA) + Vraw * VOLTAGE_FILTER_ALPHA;

    // 2. Compensación por resistencia interna
    const float Vocv = _filteredVoltage - (I * R_INTERNAL);

    // 3. Mapear voltaje corregido → SOCv
    float socFromV = voltageToPercentageCurve.inverse(
        constrain(Vocv, VOLTAGE_RANGE.min, VOLTAGE_RANGE.max)
    );

    // 4. Calcular delta tiempo
    const unsigned long now = millis();
    const float dt_h = (now - _lastUpdate) / 3600000.0f;
    _lastUpdate = now;

    // 5. Integrar Coulomb counting
    _soc += (I * dt_h / NOMINAL_CAPACITY_AH) * 100.0f;

    // 6. Corrección lenta cuando hay reposo
    if (fabs(I) < QUIET_CURRENT_THRESHOLD) {
        _soc += SOC_CORRECTION_ALPHA * (socFromV - _soc);
    }

    // 7. Clamping 0–100 %
    if (_soc < 0.0f) _soc = 0.0f;
    if (_soc > 100.0f) _soc = 100.0f;

    Logger::logInfo("SOC hybrid -> Vraw=" + std::to_string(Vraw) +
                    " Vocv=" + std::to_string(Vocv) +
                    " I=" + std::to_string(I) +
                    " SOCv=" + std::to_string(socFromV) +
                    " SOC=" + std::to_string(_soc));

    return _soc;
}

// ---------------------------------------------------------
// MAIN DE TEST
// ---------------------------------------------------------
int main() {
    TestSolarXBatteryController ctrl;

    Logger::logInfo("=== Iniciando simulación de SolarXBatteryController ===");

    // Simulación: alternar carga, descarga y reposo
    for (int step = 0; step < 50; ++step) {
        if (step < 10) {
            // Cargando lentamente
            ctrl.simulateStep(+0.2f, +0.5f);
        } else if (step < 20) {
            // Descargando
            ctrl.simulateStep(-0.25f, -0.6f);
        } else {
            // Reposo
            ctrl.simulateStep(0.0f, 0.0f);
        }

        ctrl.accuratePercentage();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    Logger::logInfo("=== Fin de simulación ===");
    return 0;
}
