#ifndef IDISPLAY_H
#define IDISPLAY_H

#include <cstdint>
#include <vector>
#include <string>

class IDisplay{
public:
    virtual void print(const uint8_t* data, size_t length) = 0;
    virtual void print(const std::vector<uint8_t>& data) = 0; // Cambio de std::vector<uint8_t> a String (linea 33
    virtual void print(const std::string& message) = 0;
    virtual void print(const char* message) = 0;
    virtual void clear() = 0;

    // --------------------------------------------- Colores ---------------------------------------------
    // Solo 3 colores disponibles
    enum class Color{
        DEFAULT, // Color por defecto del display
        RED, // Rojo
        ORANGE, // Amarillo anaranjado
        RESET
    };

    // Configuración de color
    void setColor(const Color color){
        activeColor = color;
    }

    [[nodiscard]] Color getColor() const{
        return activeColor;
    }

    class ColorUtils{
    public:
        // Códigos ANSI para ConsoleDisplay
        static const char* getAnsiCode(Color color){
            switch (color){
            case Color::DEFAULT: return "\x1b[39m"; // Color por defecto
            case Color::RED: return "\x1b[31m"; // Rojo
            case Color::ORANGE: return "\x1b[33m"; // Amarillo (más parecido a naranja en terminal)
            case Color::RESET: return "\x1b[0m";
            default: return "\x1b[39m"; // Color por defecto
            }
        }

        // Convertir a formato RGB565 para displays LCD/TFT
        static uint16_t getRGBValue(Color color){
            switch (color){
            case Color::DEFAULT: return 0xFFFF; // Blanco
            case Color::RED: return 0xF800; // Rojo
            case Color::ORANGE: return 0xFD20; // Naranja
            case Color::RESET: return 0xFFFF;
            default: return 0xFFFF; // Blanco
            }
        }


        // Reset ANSI para volver al color por defecto
        // static const char* getAnsiReset(){
        //     return "\x1b[0m";
        // }
    };

protected:
    // Color activo actual
    Color activeColor = Color::DEFAULT;
};

#endif
