#pragma once

#include <immintrin.h> // For _mm_popcnt_u64 (Clang supports this intrinsic)
#include <bit>
#include <cstdint>  // For uint64_t
//#include <bitset>   // For std::bitset








// Gets a specific bit from the bitboard
inline bool Get_bit(uint64_t bitboard, int index) {
    return (bitboard & (1ULL << index)) != 0;
}

// Sets a specific bit in the bitboard
inline void Set_bit(uint64_t& bitboard, int index) {
    bitboard |= (1ULL << index);
}

// Pops (clears) a specific bit from the bitboard
inline void Pop_bit(uint64_t& bitboard, int index) {
    (bitboard) &= ~(1ULL << index);
}

// Counts the number of set bits in the bitboard
inline int count_bits(uint64_t bitboard) {
    return __builtin_popcountll(bitboard);  // SSE4.2 intrinsic function for bit counting
}

// Gets the index of the least significant 1-bit
inline int get_ls1b(uint64_t bitboard) {
    return bitboard ? __builtin_ctzll(bitboard) : 64; // or return -1;
}
// Sets occupancy bits based on the index and attack mask
uint64_t set_occupancy(int index, int bits_in_mask, uint64_t attack_mask);