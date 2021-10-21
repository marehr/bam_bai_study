#include <charconv>
#include <iostream>
#include <string_view>

/* calculate bin given an alignment covering [beg,end) (zero-based, half-closed-half-open) */
int region2bin(int beg, int end)
{
    --end;
    if (beg>>14 == end>>14) return ((1<<15)-1)/7 + (beg>>14);
    if (beg>>17 == end>>17) return ((1<<12)-1)/7 + (beg>>17);
    if (beg>>20 == end>>20) return ((1<<9)-1)/7  + (beg>>20);
    if (beg>>23 == end>>23) return ((1<<6)-1)/7  + (beg>>23);
    if (beg>>26 == end>>26) return ((1<<3)-1)/7  + (beg>>26);
    return 0;
}

int main(int argc, const char ** argv)
{
    if (argc < 3)
    {
        std::cout << argv[0] << " <begin_index> <end_index>" << std::endl;
        return 0;
    }

    std::string_view begin_index_string{argv[1]};
    std::string_view end_index_string{argv[2]};

    int begin_index;
    int end_index;
    std::from_chars(begin_index_string.begin(), begin_index_string.end(), begin_index);
    std::from_chars(end_index_string.begin(), end_index_string.end(), end_index);

    std::cout << "region2bin(" << begin_index << ", " << end_index << ") = "
              << region2bin(begin_index, end_index) << std::endl;
}
