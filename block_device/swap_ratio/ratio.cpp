/**
 * [?] What's the theory after the Pagemap ?
 *      Does it traverse the page table for every read?
 * 
 *  
 * The format of Pagemap
 *  1) start from virtu address  0x0.
 *  2) index by virtual page_index
 *  3) 8 bytes for each entry. 
 * 
 */


#include "ratio.h"


double pages_inmem_ratio(pid_t pid, unsigned long addr_start, unsigned long addr_end)
{
    std::string pagemap_path = "/proc/" + std::to_string(pid) + "/pagemap";
    unsigned long num_pages = (addr_end - addr_start) / 0x1000;

    FILE *pagemap = fopen(pagemap_path.c_str(), "rb");
    if (!pagemap) {
        std::cerr << "cannot open " << pagemap_path << std::endl;
        return -1;
    }

    uint64_t base_offset = addr_start / PAGE_SIZE * PAGEMAP_ENTRY; //  8 bytes/ page entry, calculate the byte offset.
    int status = fseek(pagemap, base_offset, SEEK_SET);
    if (status) {
        std::cerr << "cannot seek to " << base_offset << std::endl;
        return -1;
    }

    unsigned long num_pages_in = 0;
    unsigned long num_pages_swapped = 0;

    for (int j = 0; j < num_pages; j++) {
        uint64_t read_val = 0;
        unsigned char c_buf[PAGEMAP_ENTRY];
        for (int i = 0; i < PAGEMAP_ENTRY; i++) {
            int c = getc(pagemap);  // Read from low addr. For LE, read the Least Significant char first
            int idx = IS_BIG_ENDIAN ?  i  : PAGEMAP_ENTRY - i - 1;
            c_buf[idx] = c; // Put the Most Signicant unsigned char in [0]. For litttle endian, the first char in pagemap is the Lest signicicant char.
        }

        // read order : c_buf[0]<<56  c_buf[1]<<48  ... c_buf[7]<< 0
        for (unsigned char i = 0; i< PAGEMAP_ENTRY; i++) {
            read_val = (read_val << 8) + c_buf[i]; // covert each unsigned char to a part of the uint64_t/unsigned long.
        }

        if (PG_PRESENT(read_val)) num_pages_in++;
        if (PG_SWAPPED(read_val)) num_pages_swapped++;
    }

    fclose(pagemap);
    return double(num_pages_in) / num_pages;
}




int main(int argc, char **argv)
{
    if (argc != 4) {
        // argv[0] is ratio.o, 
        std::cout << "usage: ./ratio <pid> <addr_start> <addr_end>" << std::endl;  
        return -1;
    }
    uint64_t pid = std::stoi(argv[1]);
    uint64_t addr_start = std::stol(argv[2], nullptr, 16);
    uint64_t addr_end = std::stol(argv[3], nullptr, 16);

    double ratio = pages_inmem_ratio(pid, addr_start, addr_end);
    std::cout << ratio << std::endl;
    return 0;
}
