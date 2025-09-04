#ifndef IDISPLAY_H
#define IDISPLAY_H

#include <cstdint>
#include <vector>
#include <string>

class IDisplay
{
public:
    virtual void print(const uint8_t* data, size_t length) = 0;
    virtual void print(const std::vector<uint8_t>& data) = 0; // Cambio de std::vector<uint8_t> a String (linea 33
    virtual void print(const std::string& message) = 0;
    virtual void print(const char* message) = 0;
    virtual void clear() = 0;

    // --------------------------------------------- Colores ---------------------------------------------
    // Solo 3 colores disponibles
    enum class Color
    {
        DEFAULT, // Color por defecto del display
        RED, // Rojo
        ORANGE // Amarillo anaranjado
    };

    // Configuración de color
    void setColor(const Color color)
    {
        activeColor = color;
    }

    [[nodiscard]] Color getColor() const
    {
        return activeColor;
    }

    class ColorUtils
    {
    public:
        // Códigos ANSI para ConsoleDisplay
        static const char* getAnsiCode(Color color)
        {
            switch (color)
            {
            case Color::DEFAULT: return "\x1b[39m"; // Color por defecto
            case Color::RED: return "\x1b[31m"; // Rojo
            case Color::ORANGE: return "\x1b[33m"; // Amarillo (más parecido a naranja en terminal)
            default: return "\x1b[39m";
            }
        }

        // Reset ANSI para volver al color por defecto
        static const char* getAnsiReset()
        {
            return "\x1b[0m";
        }

        // Extraer componentes RGB del valor del enum
        static uint8_t getRed(Color color)
        {
            return static_cast<uint8_t>((static_cast<uint32_t>(color) >> 16) & 0xFF);
        }

        static uint8_t getGreen(Color color)
        {
            return static_cast<uint8_t>((static_cast<uint32_t>(color) >> 8) & 0xFF);
        }

        static uint8_t getBlue(Color color)
        {
            return static_cast<uint8_t>(static_cast<uint32_t>(color) & 0xFF);
        }

        // Convertir a formato RGB565 para displays LCD/TFT
        static uint16_t toRGB565(Color color)
        {
            uint32_t rgb = static_cast<uint32_t>(color);
            uint8_t r = (rgb >> 19) & 0x1F; // 5 bits para rojo
            uint8_t g = (rgb >> 10) & 0x3F; // 6 bits para verde
            uint8_t b = (rgb >> 3) & 0x1F; // 5 bits para azul
            return (r << 11) | (g << 5) | b;
        }
    };

protected:
    // Color activo actual
    Color activeColor = Color::DEFAULT;
};

#endif
