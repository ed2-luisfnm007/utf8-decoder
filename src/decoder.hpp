#pragma once

#include "types.hpp"

#include <cstdint>
#include <vector>

class Decoder
{
  public:
    DecodeResult decode(const std::vector<std::uint8_t> &buffer);

  private:
    bool isTrailByte(const std::uint8_t byte);
    bool hasBOM(const std::vector<std::uint8_t> &buffer);
};