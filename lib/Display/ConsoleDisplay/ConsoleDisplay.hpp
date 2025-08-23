#ifndef CONSOLEDISPLAY_HPP
#define CONSOLEDISPLAY_HPP


#include "IDisplay.h"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

class ConsoleDisplay : public IDisplay{
public:
    void print(const uint8_t* data, size_t length) override;
    void print(const std::vector<uint8_t>& data) override;
    void print(const std::string& message) override;
    void print(const char* message) override;
    void clear() override;
};

#endif //CONSOLEDISPLAY_HPP
