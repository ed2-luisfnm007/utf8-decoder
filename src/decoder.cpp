#include "decoder.hpp"
#include "types.hpp"
#include <vector>

DecodeResult Decoder::decode(const std::vector<std::uint8_t> &buffer)
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

            if (codepoint < 0x80)
            {
                errors.emplace_back(offset, "secuencia sobre larga");
                offset += 2;
                continue;
            }
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

            if (codepoint < 0x800)
            {
                errors.emplace_back(offset, "secuencia sobrelarga");
                offset += 3;
                continue;
            }
        }
        else if ((b1 & 0xF8) == 0xF0)
        {
            type = CodePointType::FOUR_BYTE;

            if (b1 > 0xF4)
            {
                errors.emplace_back(offset, "byte lider invalido");
                offset++;
                continue;
            }

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

            if (codepoint < 0x10000)
            {
                errors.emplace_back(offset, "secuencia sobrelarga");
                offset += 4;
                continue;
            }
        }
        else if (isTrailByte(b1))
        {
            errors.emplace_back(offset, "byte de continuación inesperado");
            offset += 1;
            continue;
        }
        else
        {
            errors.emplace_back(offset, "byte líder inválido");
            offset += 1;
            continue;
        }

        symbols.emplace_back(codepoint, type);
        offset += static_cast<std::size_t>(type);
    }

    return {symbols, errors};
}

bool Decoder::hasBOM(const std::vector<std::uint8_t> &buffer)
{
    if (buffer.size() < 3)
        return false;

    bool first = buffer[0] == 0xEF;
    bool second = buffer[1] == 0xBB;
    bool third = buffer[2] == 0xBF;

    return (first && second && third);
}

bool Decoder::isTrailByte(const std::uint8_t byte)
{

    return ((byte & 0xC0) == 0x80);
}
