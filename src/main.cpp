#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

std::optional<std::vector<std::uint8_t>> read(const std::string &path);

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cerr << "Error: File's path is missing.\n";
        std::cerr << "Use: " << argv[0] << " <file_path>\n";
        return 1; 
    }

    std::string path = argv[1];
    return 0;     
}

std::optional<std::vector<std::uint8_t>> read(const std::string &path)
{
    fs::path file(path);
    std::ifstream in(file, std::ios::binary | std::ios::ate);
    if(!in)
        return std::nullopt;

    auto position = in.tellg();
    if(position == std::streampos(-1))
        return std::nullopt;

    size_t size = static_cast<size_t>(position);

    in.seekg(0,std::ios::beg);
    if(!in)
        return std::nullopt;

    std::vector<std::uint8_t> buffer(size);

    for(size_t i = 0; i < size; i++)
    {
        in.read(reinterpret_cast<char *>(&buffer[i]), sizeof(std::uint8_t));
        if(!in)
            return std::nullopt;
    }

    return buffer;
}