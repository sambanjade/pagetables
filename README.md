
# Multi-Level Page Table Simulation

## Overview
This project simulates a simple multi-level page table in the C library.  It supports from two to the max of "LEVELS" of the page table and 'POBITS' is configurable through the 'config.h' file.
## Configuration Guide
- `LEVELS` (1-6): Determines the level of page table structure. 
- `POBITS` (4-18): Determines page size. 
For users they must (to ensure compatibility), that '(POBITS-3) * (LEVELS+1) <= 60. 

## Building the file
Use the Makefile:
`make all` compiles `libmlpt.a`, by linking some variables as shown in the following list;
* `-lm`: Compiles `libmlpt.a`, linking floating point operations
* `-lpthread`: Compiles `libmlpt.a` linking Posix threads
* `-ldl`: Compiles `libmlpt.a` linking dynamic links librarians
Their filters include make clean, which deletes compiled files.
## API of the file
- `size_t translate(size_t va)`: Returns the physical address for `va`. If not mapped, returns all bits set to 1.
- `void page_allocate(size_t va)`: Allocates memory and updates the page table.
- `void page_deallocate(size_t va)`: Frees memory associated with `va`.

## Testing of edge cases and such
- Testing includes counters for memory allocations and manual `ptbr` setups for isolated testing.

## Complexity of the methods
- **Time**: `O(LEVELS)`
- **Space**: Proportional to `LEVELS` and `POBITS`.

## License (Copyright)
Licensed under MIT. See `LICENSE` for explanation of other licenses considered and why MIT was chosen. 
