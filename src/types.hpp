#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

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
