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

enum class CodePointType
{
    ONE_BYTE = 1,
    TWO_BYTE = 2,
    THREE_BYTE = 3,
    FOUR_BYTE = 4
};

struct CodePoint
{
    std::uint32_t code;
    CodePointType type;
};

struct Error
{
    std::size_t offset;
    std::string description;
};

struct DecodeResult
{
    std::vector<CodePoint> symbols;
    std::vector<Error> errors;
};

std::optional<std::vector<std::uint8_t>> read(const std::string &path);
DecodeResult decode(const std::vector<std::uint8_t> &buffer);

bool hasBOM(const std::vector<std::uint8_t> &buffer);
bool isTrailByte(const std::uint8_t byte);
bool isPrintable(const std::uint32_t codepoint);

void printSymbols(const std::vector<CodePoint> &symbols);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Error: La ruta del archivo esta faltante.\n";
        std::cerr << "Uso: " << argv[0] << " <ruta_de_archivo>\n";
        return 1;
    }

    std::string path = argv[1];

    auto buffer = read(path);

    if(!buffer)
    {
        std::cerr << "Error: No se pudo leer el archivo." << std::endl;
        return 1;
    }

    DecodeResult dr = decode(buffer.value());

    printSymbols(dr.symbols);
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

DecodeResult decode(const std::vector<std::uint8_t> &buffer)
{
    if (buffer.empty())
        return {};

    std::size_t offset = 0;

    if (hasBOM(buffer))
        offset = 3;

    std::vector<CodePoint> symbols;
    std::vector<Error> errors;

    symbols.reserve(buffer.size());

    while (offset < buffer.size())
    {
        auto b1 = buffer[offset];

        std::uint32_t codepoint = 0x0000;
        CodePointType type;

        if ((b1 & 0x80) == 0x00)
        {
            codepoint = b1;
            type = CodePointType::ONE_BYTE;
        }
        else if ((b1 & 0xE0) == 0xC0)
        {
            type = CodePointType::TWO_BYTE;

            if ((offset + 1) >= buffer.size())
            {
                errors.emplace_back(offset, "secuencia incompleta");
                offset += 1;
                continue;
            }

            auto b2 = buffer[offset + 1];

            if (!isTrailByte(b2))
            {
                errors.emplace_back(offset, "byte de continuación esperado, no encontrado");
                offset += 1;
                continue;
            }

            codepoint = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
        }
        else if ((b1 & 0xF0) == 0xE0)
        {
            type = CodePointType::THREE_BYTE;

            if ((offset + 2) >= buffer.size())
            {
                errors.emplace_back(offset, "secuencia incompleta");
                offset += 1;
                continue;
            }
            auto b2 = buffer[offset + 1];
            auto b3 = buffer[offset + 2];

            if (!isTrailByte(b2) || !isTrailByte(b3))
            {
                errors.emplace_back(offset, "byte de continuación esperado, no encontrado");
                offset += 1;
                continue;
            }

            codepoint = ((b1 & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        }
        else if ((b1 & 0xF8) == 0xF0)
        {
            type = CodePointType::FOUR_BYTE;

            if ((offset + 3) >= buffer.size())
            {
                errors.emplace_back(offset, "secuencia incompleta");
                offset += 1;
                continue;
            }

            auto b2 = buffer[offset + 1];
            auto b3 = buffer[offset + 2];
            auto b4 = buffer[offset + 3];

            if (!isTrailByte(b2) || !isTrailByte(b3) || !isTrailByte(b4))
            {
                errors.emplace_back(offset, "byte de continuación esperado, no encontrado");
                offset += 1;
                continue;
            }

            codepoint = ((b1 & 0x07) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);
        }
        else if (isTrailByte(b1))
        {
            errors.emplace_back(offset, "byte de continuación inesperado");
            offset += 1;
            continue;
        }
        else
        {
            errors.emplace_back(offset, "byte lider invalido");
            offset += 1;
            continue;
        }

        symbols.emplace_back(codepoint, type);
        offset += static_cast<std::size_t>(type);
    }

    return {symbols, errors};
}

bool hasBOM(const std::vector<std::uint8_t> &buffer)
{
    if (buffer.size() < 3)
        return false;

    bool first = buffer[0] == 0xEF;
    bool second = buffer[1] == 0xBB;
    bool third = buffer[2] == 0xBF;

    return (first && second && third);
}

bool isTrailByte(const std::uint8_t byte)
{
    return ((byte & 0xC0) == 0x80);
}

void printSymbols(const std::vector<CodePoint> &symbols)
{
    if (symbols.empty())
    {
        std::cout << "Sin simbolos." << std::endl;
        return;
    }

    for (CodePoint sym : symbols)
    {
        if(isPrintable(sym.code))
        {
            std::cout << static_cast<char>(sym.code) << std::endl;
            continue;
        }

        std::string code = std::format("U+{:04X}", sym.code);
        std::cout << code << std::endl;
    }
}

bool isPrintable(const std::uint32_t codepoint)
{
    return (codepoint >= 0x0020 && codepoint <= 0x007E);
}