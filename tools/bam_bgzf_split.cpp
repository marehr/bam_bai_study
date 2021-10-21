#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <string_view>

struct bgzf_block_header
{
    uint8_t id1;
    uint8_t id2;
    uint8_t cm;
    uint8_t flg;
    uint32_t mtime;
    uint8_t xfl;
    uint8_t os;
    uint16_t xlen;
};

struct bgzf_block_header_extra
{
    uint8_t si1;
    uint8_t si2;
    uint16_t slen;
    uint16_t bsize;
};

static_assert(sizeof(bgzf_block_header) == 3 /*uint32_t*/ * 4 /*4 bytes_per_uint32_t*/);
static_assert(sizeof(bgzf_block_header_extra) == 3 /*uint16_t*/ * 2 /*2 bytes_per_uint16_t*/);

void passthrough_bgzf_block(std::ofstream & fout, std::ifstream & fin, std::vector<char> & buffer)
{
    using namespace std::literals;

    bgzf_block_header header;
    fin.read(reinterpret_cast<char *>(&header), sizeof(header));
    fout.write(reinterpret_cast<char *>(&header), sizeof(header));

    if (!(header.id1 == 31))
        throw std::runtime_error{"bgzf_block_header::id1 is wrong: expected 31, actual: "s + std::to_string(header.id1)};

    if (!(header.id2 == 139))
        throw std::runtime_error{"bgzf_block_header::id2 is wrong: expected 139, actual: "s + std::to_string(header.id2)};

    if (!(header.cm == 8))
        throw std::runtime_error{"bgzf_block_header::cm is wrong: expected 8, actual: "s + std::to_string(header.cm)};

    if (!(header.flg == 4))
        throw std::runtime_error{"bgzf_block_header::flg is wrong: expected 4, actual: "s + std::to_string(header.flg)};

    if (!(header.xlen == 6)) // sizeof(bgzf_block_header_extra)
        throw std::runtime_error{"bgzf_block_header::xlen is wrong: expected 6, actual: "s + std::to_string(header.xlen)};

    bgzf_block_header_extra extra_header;
    fin.read(reinterpret_cast<char *>(&extra_header), sizeof(extra_header));
    fout.write(reinterpret_cast<char *>(&extra_header), sizeof(extra_header));

    if (!(extra_header.si1 == 66))
        throw std::runtime_error{"bgzf_block_header_extra::si1 is wrong: expected 66, actual: "s + std::to_string(extra_header.si1)};

    if (!(extra_header.si2 == 67))
        throw std::runtime_error{"bgzf_block_header_extra::si2 is wrong: expected 67, actual: "s + std::to_string(extra_header.si2)};

    if (!(extra_header.slen == 0x0002u))
        throw std::runtime_error{"bgzf_block_header_extra::slen is wrong: expected 0x0002u, actual: "s + std::to_string(extra_header.slen)};

    // sizeof(bgzf_block_header) + header.xlen >= 12 + 6
    size_t header_size = sizeof(bgzf_block_header) + header.xlen;

    // crc32:uint32_t, isize:uint32_t = 8
    size_t footer_size = sizeof(uint32_t) + sizeof(uint32_t);

    if (!(extra_header.bsize >= header_size + footer_size))
        throw std::runtime_error{"bgzf_block_header_extra::bsize is wrong: expected at least header_size ("s + std::to_string(header_size) + ") + footer_size ("s + std::to_string(footer_size) + "), actual: "s + std::to_string(extra_header.bsize)};

    // also copy over crc32 and isize too
    size_t cdata_size = extra_header.bsize - header_size - footer_size + 1;
    fin.read(buffer.data(), cdata_size);
    fout.write(buffer.data(), cdata_size);

    // read crc32 and isize
    fin.read(buffer.data(), footer_size);
    fout.write(buffer.data(), footer_size);
}

size_t decimal_places(size_t number)
{
    return std::to_string(number).size();
}

std::string constant_width_number(size_t number, size_t decimal_places)
{
    std::string str = std::to_string(number);
    return std::string(decimal_places - str.length(), '0') + str;
}

int main(int argc, const char ** argv)
{
    if (argc < 2)
    {
        std::cout << argv[0] << " <BAM_FILE>" << std::endl;
        return 0;
    }

    std::filesystem::path bam_file_path = argv[1];

    std::cout << "Open File: " << bam_file_path << std::endl;

    std::ifstream fin(bam_file_path, std::ios::binary);
    using traits_t = typename decltype(fin)::traits_type;

    constexpr size_t max_block_size = 1 << (8 * sizeof(uint16_t));
    static_assert(max_block_size == 65536);
    std::vector<char> buffer(max_block_size + sizeof(uint32_t) /*crc32*/ + sizeof(uint32_t) /*isize*/, '\0');

    size_t max_decimal_places = decimal_places(std::filesystem::file_size(bam_file_path));

    while(traits_t::not_eof(fin.peek())) // copy until end of file
    {
        std::filesystem::path bgzf_block = bam_file_path;
        bgzf_block.replace_extension(std::string{".bam."} + constant_width_number((size_t)fin.tellg(), max_decimal_places) + ".gz");
        std::cout << "Write bgzf block: " << bgzf_block << std::endl;

        std::ofstream fout(bgzf_block, std::ios::binary);
        passthrough_bgzf_block(fout, fin, buffer);
    }
}
