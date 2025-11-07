#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include <cstdint>

int main(int argc, char** argv)
{
    // ================================
    // Parse arguments
    // ================================

    std::string outputFile = "private_key.h";
    int length = 400;

    if (argc >= 3)
    {
        std::cout << "Using provided arguments.\n";
        outputFile = argv[1];
        length = std::stoi(argv[2]);
    }
    else
    {
        std::cout << "Using default arguments.\n";
    }


    if (length <= 0)
    {
        std::cerr << "Error: length must be positive.\n";
        return 1;
    }

    // ================================
    // Deterministic compile-time seed (can use __TIME__ or fixed)
    // ================================
    std::seed_seq seed{
        static_cast<unsigned int>(__TIME__[0]),
        static_cast<unsigned int>(__TIME__[1]),
        static_cast<unsigned int>(__TIME__[2]),
        static_cast<unsigned int>(__TIME__[3]),
        static_cast<unsigned int>(__TIME__[4]),
        static_cast<unsigned int>(__TIME__[5])
    };
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 255);

    // ================================
    // Generate header file
    // ================================
    std::ofstream out(outputFile);
    if (!out.is_open())
    {
        std::cerr << "Error: cannot open " << outputFile << "\n";
        return 1;
    }

    out << "#pragma once\n\n";
    out << "// Auto-generated file. Do not edit manually.\n\n";
    out << "inline unsigned char private_key[] = {";

    for (int i = 0; i < length; ++i)
    {
        if (i % 12 == 0) out << "\n    ";
        out << "0x" << std::hex << std::uppercase << std::setw(2)
            << std::setfill('0') << dist(rng);
        if (i != length - 1) out << ", ";
    }

    out << "\n};\n\n";
    out << "inline unsigned int private_keylen = " << std::dec << length << ";\n";

    out.close();
    std::cout << "Generated header: " << outputFile << " (" << length << " bytes)\n";
    return 0;
}
