# LibEntropy
A collection of (currently) header only utilities for use with other projects.

## Containers

### Array
A `std::vector`-like container which uses 32 bit size and capacity variables to achieve a size of only 16 bytes.

Currently supports most of `std::vector`'s features with a few exceptions:
- No aliasing support *(Not planned)*
- Debug Iterators and bounds checking for insertion/deletion overflows. *(Not planned, implemented through asserts)*
- Rollback-aware mutation *(Not planned)*
- `erase_if()` *(Planned)*
- `std::hash` specialisation *(Planned)*
- `ranges` support and other C++23 features *(Unsure)*