#include "ConsoleDisplay.hpp"
#include <cstdio>

static inline void print_hex_line(const uint8_t* data, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        std::printf("%02X", data[i]);
        if (i + 1 < length) std::printf(" ");
    }
    std::printf("\n");
}

void ConsoleDisplay::print(const uint8_t* data, size_t length)
{
    if (!data || length == 0) return;
    // Aplicar color si no es el por defecto
    std::printf("%s", ColorUtils::getAnsiCode(activeColor));
    print_hex_line(data, length);
    std::fflush(stdout);
}

void ConsoleDisplay::print(const std::vector<uint8_t>& data)
{
    if (data.empty()) return;
    print(data.data(), data.size());
}

void ConsoleDisplay::print(const char* message)
{
    if (!message) return;
    std::printf("%s", ColorUtils::getAnsiCode(activeColor));
    std::printf("%s\n", message);
    std::fflush(stdout);
}

void ConsoleDisplay::print(const std::string& message)
{
    if (message.empty()) return;
    print(message.c_str());
    std::fflush(stdout);
}

void ConsoleDisplay::clear()
{
    // ANSI clear screen + move cursor home
    std::printf("\x1b[2J\x1b[H");
    std::printf("%s", ColorUtils::getAnsiReset());

    std::fflush(stdout);
}
