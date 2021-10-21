#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <string_view>

struct voffset
{
    uint16_t _uoffset;
    uint64_t _coffset : 48;

    // // bam file offset to a bgzf block
    uint64_t coffset()
    {
        /* 48 bits to address */
        return _coffset;
    }

    uint16_t uoffset()
    {
        return _uoffset;
    }

    friend std::ostream& operator<<(std::ostream& os, voffset p)
    {
        os << "(.coffset: " << p.coffset() << ", .uoffset: " << p.uoffset() << ")";
        return os;
    }
};

static_assert(sizeof(voffset) == sizeof(uint64_t));

struct chunk
{
    voffset begin;
    voffset end;

    friend std::ostream& operator<<(std::ostream& os, chunk const & p)
    {
        os << "{begin: " << p.begin << ", end: " << p.end << "}";
        return os;
    }
};

struct bin
{
    uint32_t id;
    std::vector<chunk> chunks;
};

struct reference
{
    std::vector<bin> bins;
    std::vector<voffset> ioffset;
};

struct bai
{
    std::vector<reference> references;
    uint8_t n_no_coor;
};

struct bam_alignment_header
{
    int32_t block_size{};
    int32_t ref_id;
    int32_t pos;

    uint8_t read_name_size;
    uint8_t mapq;
    uint16_t bin;
    uint16_t cigar_op_size;
    uint16_t flag;

    uint32_t read_size;
    int32_t next_ref_id;

    int32_t next_pos;
    int32_t template_length;

    friend std::ostream& operator<<(std::ostream& os, bam_alignment_header const & p)
    {
        os << "{" << "\n" <<
            "\tblock_size: " << p.block_size << ", " << "\n" <<
            "\tref_id: " << p.ref_id << ", " << "\n" <<
            "\tpos: " << p.pos << ", " << "\n" <<
            "\tread_name_size: " << (unsigned)p.read_name_size << ", " << "\n" <<
            "\tmapq: " << (unsigned)p.mapq << ", " << "\n" <<
            "\tbin: " << p.bin << ", " << "\n" <<
            "\tcigar_op_size: " << p.cigar_op_size << ", " << "\n" <<
            "\tflag: " << p.flag << ", " << "\n" <<
            "\tread_size: " << p.read_size << ", " << "\n" <<
            "\tnext_ref_id: " << p.next_ref_id << ", " << "\n" <<
            "\tnext_pos: " << p.next_pos << ", " << "\n" <<
            "\ttemplate_length: " << p.template_length << "\n" <<
        "}";
        return os;
    }
};

struct bam_alignment : bam_alignment_header
{
    std::string read_name;

    friend std::ostream& operator<<(std::ostream& os, bam_alignment const & p)
    {
        os << static_cast<bam_alignment_header const &>(p) << "\n"
           << "read_name: " << std::string_view{p.read_name.data(), p.read_name.size()};
        return os;
    }
};

struct print_bam_bin_index
{
    uint16_t bin_id;

    struct interval_t
    {
        uint32_t block_id;
        uint32_t interval_size;
    };

    interval_t interval() const
    {
        static constexpr size_t smallest_region = 16 * 1024; // 2^14
        if (bin_id == 0u)
            return interval_t{.block_id{bin_id}, .interval_size{8u * 8u * 8u * 8u *  8u * smallest_region /*2^29*/}};
        else if (bin_id <= 8u)
            return interval_t{.block_id{bin_id - 0u - 1u}, .interval_size{8u * 8u * 8u * 8u * smallest_region /*2^26*/}};
        else if (bin_id <= 72u)
            return interval_t{.block_id{bin_id - 8u - 1u}, .interval_size{8u * 8u * 8u * smallest_region /*2^23*/}};
        else if (bin_id <= 584u)
            return interval_t{.block_id{bin_id - 72u - 1u}, .interval_size{8u * 8u * smallest_region /*2^20*/}};
        else if (bin_id <= 4680u)
            return interval_t{.block_id{bin_id - 584u - 1u}, .interval_size{8u * smallest_region /*2^17*/}};
        else if (bin_id <= 37448u)
            return interval_t{.block_id{bin_id - 4680u - 1u}, .interval_size{smallest_region /*2^14*/}};
        return {};
    };

    friend std::ostream& operator<<(std::ostream& os, print_bam_bin_index const & p)
    {
        print_bam_bin_index::interval_t interval = p.interval();
        os << "(.block_id: " << interval.block_id
           << ", .interval_size: " << interval.interval_size
           << ", region: [" << (interval.block_id) * interval.interval_size
                            << ", " << (interval.block_id + 1) * interval.interval_size
           << ")" << " )";
        return os;
    }
};

struct print_voffset
{
    voffset offset;
    bam_alignment alignment;

    friend std::ostream& operator<<(std::ostream& os, print_voffset const & p)
    {
        os << "\"" << p.alignment.read_name << "\" "
           << "pos: " << p.alignment.pos << " "
           << "read_size: " << p.alignment.read_size << " "
           << "bin: " << p.alignment.bin << " "
           << print_bam_bin_index{p.alignment.bin}
           << " " << p.offset;
        return os;
    }
};

bam_alignment seek_bam_alignment_header(std::filesystem::path bai_file_path, voffset offset)
{
    if (offset.coffset() == 0u)
    {
        return bam_alignment{.read_name{"BAM_HEADER"}};
    }

    auto constant_width_number = [](size_t number, size_t decimal_places) -> std::string
    {
        std::string str = std::to_string(number);
        return std::string(decimal_places - str.length(), '0') + str;
    };

    bam_alignment alignment;

    std::filesystem::path bgzf_block_file_path = bai_file_path;
    bgzf_block_file_path.replace_extension(std::string{"."} + constant_width_number(offset.coffset(), 4));

    std::ifstream bam_fin(bgzf_block_file_path, std::ios::binary);

    if (!bam_fin.is_open())
        throw std::runtime_error{"Could not open bam_file!"};

    bam_fin.seekg(offset.uoffset());

    // std::cout << "Seek to " << bgzf_block_file_path << ":" << offset.uoffset() << std::endl;

    bam_alignment_header & header_ptr = alignment;
    bam_fin.read(reinterpret_cast<char *>(&header_ptr), sizeof(bam_alignment_header));

    alignment.read_name.resize(alignment.read_name_size);
    bam_fin.read(reinterpret_cast<char *>(alignment.read_name.data()), alignment.read_name_size * sizeof(char));

    if (alignment.block_size == 0u)
    {
        return bam_alignment{.read_name{"EOF"}};
    }
    // std::cout << "alignment: " << alignment << std::endl;

    return alignment;
}

int main(int argc, const char ** argv)
{
    using namespace std::literals;


    if (argc < 2)
    {
        std::cout << argv[0] << " <BAI_FILE>" << std::endl;
        return 0;
    }

    std::filesystem::path bai_file_path = argv[1];

    std::cout << "Open File: " << bai_file_path << std::endl;

    std::ifstream bai_fin(bai_file_path, std::ios::binary);

    if (!bai_fin.is_open())
        throw std::runtime_error{"Could not open bai_file!"};

    ::bai bai{};

    uint32_t magic_header{};
    bai_fin.read(reinterpret_cast<char *>(&magic_header), sizeof(magic_header));

    if (!(magic_header == 21578050))
        throw std::runtime_error{"magic_header is wrong: expected 21578050, actual: "s + std::to_string(magic_header)};

    uint32_t references_size{};
    bai_fin.read(reinterpret_cast<char *>(&references_size), sizeof(references_size));
    bai.references.resize(references_size);
    std::cout << "number of references: " << references_size << std::endl;

    for (::reference & reference: bai.references)
    {
        uint32_t bins_size{};
        bai_fin.read(reinterpret_cast<char *>(&bins_size), sizeof(bins_size));
        reference.bins.resize(bins_size);
        std::cout << "\tnumber of bins: " << bins_size << std::endl;

        for (::bin & bin: reference.bins)
        {
            bai_fin.read(reinterpret_cast<char *>(&bin.id), sizeof(bin.id));
            std::cout << "\t\tbin_id: " << bin.id << std::endl;

            uint32_t chunks_size{};
            bai_fin.read(reinterpret_cast<char *>(&chunks_size), sizeof(chunks_size));
            bin.chunks.resize(chunks_size);
            std::cout << "\t\tnumber of chunks: " << chunks_size << std::endl;

            bai_fin.read(reinterpret_cast<char *>(bin.chunks.data()), chunks_size * sizeof(::chunk));
            for (::chunk & chunk: bin.chunks)
            {
                bam_alignment begin_alignment = seek_bam_alignment_header(bai_file_path, chunk.begin);
                bam_alignment end_alignment = seek_bam_alignment_header(bai_file_path, chunk.end);
                std::cout << "\t\t\tchunk.begin: " << print_voffset{chunk.begin, begin_alignment} << "\n";
                std::cout << "\t\t\tchunk.end: " << print_voffset{chunk.end, end_alignment} << "\n";
            }
        }

        uint32_t ioffset_size{};
        bai_fin.read(reinterpret_cast<char *>(&ioffset_size), sizeof(ioffset_size));
        reference.ioffset.resize(ioffset_size);
        std::cout << "\tnumber of ioffset: " << ioffset_size << std::endl;

        bai_fin.read(reinterpret_cast<char *>(reference.ioffset.data()), ioffset_size * sizeof(::voffset));
        for (::voffset & ioffset: reference.ioffset)
        {
            bam_alignment alignment = seek_bam_alignment_header(bai_file_path, ioffset);
            std::cout << "\t\tioffset: " << print_voffset{ioffset, alignment} << std::endl;
        }
    }
}
