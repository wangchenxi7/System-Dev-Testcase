#ifndef RATIO_H
#define RATIO_H

#include <iostream>

#include "cstdint"

#define PAGE_SIZE 4096
#define PAGEMAP_ENTRY 8  // 64 bits per entry

#define GET_BIT(X,Y) (X & ((uint64_t)1<<Y)) >> Y
#define GET_PFN(X) X & 0x7FFFFFFFFFFFFF   // not mapped : NULL, present : virt page index ? , swapped : swap entry offset ?
#define PG_PRESENT(X) (GET_BIT(X, 63))
#define PG_SWAPPED(X) (GET_BIT(X, 62))

const int __endian_bit = 1;  // 0x0000 0001
#define IS_BIG_ENDIAN ((*(char*)&__endian_bit) == 0)  // Intel is little-endian, starts from Least Significant Endian.

double pages_inmem_ratio(pid_t pid, unsigned long addr_start, unsigned long addr_end);

#endif //RATIO_H
