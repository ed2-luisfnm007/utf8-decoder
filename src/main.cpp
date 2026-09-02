#include "decoder.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::optional<std::vector<std::uint8_t>> read(const std::string &path);

bool isPrintable(const std::uint32_t codepoint);

void printSymbols(const std::vector<CodePoint> &symbols);
void printErrors(const std::vector<Error> &errors);
void printSummary(const std::vector<CodePoint> &symbols, std::size_t totalBytes, int totalErrors);
void printWelcome(const std::string &path);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Error: Falta la ruta del archivo.\n";
        std::cerr << "Uso: " << argv[0] << " <ruta_del_archivo>\n";
        return 1;
    }

    std::string path = argv[1];

    auto buffer = read(path);

    if (!buffer)
    {
        std::cerr << "Error: No se pudo leer el archivo." << std::endl;
        return 1;
    }

    Decoder decoder;
    DecodeResult dr = decoder.decode(buffer.value());

    printWelcome(path);
    printSymbols(dr.symbols);

    std::cout << "========================\n";
    std::cout << std::endl << ">> ERRORES:" << std::endl;
    printErrors(dr.errors);

    printSummary(dr.symbols, buffer->size(), static_cast<int>(dr.errors.size()));

    return 0;
}

std::optional<std::vector<std::uint8_t>> read(const std::string &path)
{
    fs::path file(path);
    std::ifstream in(file, std::ios::binary | std::ios::ate);
    if (!in)
        return std::nullopt;

    auto position = in.tellg();
    if (position == std::streampos(-1))
        return std::nullopt;

    std::size_t size = static_cast<std::size_t>(position);

    in.seekg(0, std::ios::beg);
    if (!in)
        return std::nullopt;

    std::vector<std::uint8_t> buffer(size);

    for (std::size_t i = 0; i < size; i++)
    {
        in.read(reinterpret_cast<char *>(&buffer[i]), sizeof(std::uint8_t));
        if (!in)
            return std::nullopt;
    }

    return buffer;
}

void printSymbols(const std::vector<CodePoint> &symbols)
{
    if (symbols.empty())
    {
        std::cout << "No se encontraron símbolos." << std::endl;
        return;
    }

    for (CodePoint sym : symbols)
    {
        if (isPrintable(sym.code))
        {
            std::cout << static_cast<char>(sym.code) << std::endl;
            continue;
        }

        std::cout << std::format("U+{:04X}\n", sym.code);
    }
}

void printSummary(const std::vector<CodePoint> &symbols, std::size_t totalBytes, int totalErrors)
{
    int b1 = 0, b2 = 0, b3 = 0, b4 = 0;

    for (const CodePoint &codep : symbols)
    {
        switch (codep.type)
        {

        case CodePointType::ONE_BYTE:
            b1++;
            break;
        case CodePointType::TWO_BYTE:
            b2++;
            break;
        case CodePointType::THREE_BYTE:
            b3++;
            break;
        case CodePointType::FOUR_BYTE:
            b4++;
            break;
        default:
            break;
        }
    }

    int totalValid = b1 + b2 + b3 + b4;

    std::cout << "\n>> RESUMEN: \n";
    std::cout << std::format("Total de bytes procesados: {}\n", totalBytes);
    std::cout << std::format("Code points validos: {}\n", totalValid);
    std::cout << std::format("Total de errores encontrados: {}\n", totalErrors);

    std::cout << "\n>> CANTIDAD DE SIMBOLOS POR NUMERO DE BYTES: \n";
    std::cout << std::format("1 byte\t{}\n", b1);
    std::cout << std::format("2 byte\t{}\n", b2);
    std::cout << std::format("3 byte\t{}\n", b3);
    std::cout << std::format("4 byte\t{}\n", b4);
}

void printErrors(const std::vector<Error> &errors)
{
    if (errors.empty())
    {
        std::cout << "No se encontraron errores." << std::endl;
        return;
    }

    for (Error err : errors)
    {
        std::cout << std::format("[Desplazamiento: {}] Error: {}\n", err.offset, err.description);
    }
}

bool isPrintable(const std::uint32_t codepoint)
{
    return (codepoint >= 0x0020 && codepoint <= 0x007E);
}

void printWelcome(const std::string &path)
{
    std::cout << std::format("========================\n"
                             "      UTF-8 DECODER      \n"
                             "========================\n"
                             ">> ARCHIVO: {}\n"
                             ">> CONTENIDO:\n\n",
                             path);
}
