#pragma once

#include <cstdint>
#include <vector>

#include <intrin.h>

#include "game.hpp"

namespace tiler {
  template<int A, int B, int C, int D>
  class evaluate final {
    const bitmap&               _obstacle_blocks;
    const std::vector<polyhex>& _polyhexes;

  public:
    evaluate(const bitmap& obstacle_blocks, const std::vector<polyhex>& polyhexes) noexcept: _obstacle_blocks(obstacle_blocks), _polyhexes(polyhexes) {
      ;
    }

  private:
    __forceinline auto lines(const bitmap& blocks) const noexcept {
      auto result = blocks.lines();

      for (auto i = 0; i < 64; ++i) {
        result[i] |= _obstacle_blocks.lines()[i];
      }

      return result;
    }

    __forceinline auto edge(const std::array<std::uint64_t, 64>& lines) const noexcept {
      auto result = 0;

      {
        const auto i = 0;

        result += 3 * static_cast<int>(_mm_popcnt_u64(lines[i    ] << 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i    ] >> 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i + 1] << 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i + 1]      & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110));

        result += 6 * static_cast<int>(_mm_popcnt_u64(lines[i    ] >> 1 & ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001) +
                                       _mm_popcnt_u64(lines[i + 1]      & ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001));

        result += 4 * static_cast<int>(_mm_popcnt_u64(lines[i    ] << 1 & ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000) +
                                       _mm_popcnt_u64(lines[i + 1] << 1 & ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000) +
                                       _mm_popcnt_u64(lines[i + 1]      & ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000));
      }

      for (auto i = 1; i < 63; ++i) {
        result += 2 * static_cast<int>(_mm_popcnt_u64(lines[i - 1]      & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i - 1] >> 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i    ] << 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i    ] >> 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i + 1] << 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i + 1]      & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110));

        result += 3 * static_cast<int>(_mm_popcnt_u64(lines[i - 1]      & ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001) +
                                       _mm_popcnt_u64(lines[i - 1] >> 1 & ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001) +
                                       _mm_popcnt_u64(lines[i    ] >> 1 & ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001) +
                                       _mm_popcnt_u64(lines[i + 1]      & ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001));

        result += 3 * static_cast<int>(_mm_popcnt_u64(lines[i - 1]      & ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000) +
                                       _mm_popcnt_u64(lines[i    ] << 1 & ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000) +
                                       _mm_popcnt_u64(lines[i + 1] << 1 & ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000) +
                                       _mm_popcnt_u64(lines[i + 1]      & ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000));
      }

      {
        const auto i = 63;

        result += 3 * static_cast<int>(_mm_popcnt_u64(lines[i - 1]      & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i - 1] >> 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i    ] << 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110) +
                                       _mm_popcnt_u64(lines[i    ] >> 1 & ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110));

        result += 4 * static_cast<int>(_mm_popcnt_u64(lines[i - 1]      & ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001) +
                                       _mm_popcnt_u64(lines[i - 1] >> 1 & ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001) +
                                       _mm_popcnt_u64(lines[i    ] >> 1 & ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001));

        result += 6 * static_cast<int>(_mm_popcnt_u64(lines[i - 1]      & ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000) +
                                       _mm_popcnt_u64(lines[i    ] << 1 & ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000));
      }

      return result;
    }

    __forceinline auto one_hole(const std::array<std::uint64_t, 64>& lines) const noexcept {
      auto result = 0;

      {
        const auto i = 0;

        result += static_cast<int>(_mm_popcnt_u64(lines[i    ] << 1 &
                                                  lines[i    ] >> 1 &
                                                  lines[i + 1] << 1 &
                                                  lines[i + 1]      &
                                                  ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110));

        result += static_cast<int>(_mm_popcnt_u64(lines[i    ] >> 1 &
                                                  lines[i + 1]      &
                                                  ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001));

        result += static_cast<int>(_mm_popcnt_u64(lines[i    ] << 1 &
                                                  lines[i + 1] << 1 &
                                                  lines[i + 1]      &
                                                  ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000));
      }

      for (auto i = 1; i < 63; ++i) {
        result += static_cast<int>(_mm_popcnt_u64(lines[i - 1]      &
                                                  lines[i - 1] >> 1 &
                                                  lines[i    ] << 1 &
                                                  lines[i    ] >> 1 &
                                                  lines[i + 1] << 1 &
                                                  lines[i + 1]      &
                                                  ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110));

        result += static_cast<int>(_mm_popcnt_u64(lines[i - 1]      &
                                                  lines[i - 1] >> 1 &
                                                  lines[i    ] >> 1 &
                                                  lines[i + 1]      &
                                                  ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001));

        result += static_cast<int>(_mm_popcnt_u64(lines[i - 1]      &
                                                  lines[i    ] << 1 &
                                                  lines[i + 1] << 1 &
                                                  lines[i + 1]      &
                                                  ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000));
      }

      {
        const auto i = 63;

        result += static_cast<int>(_mm_popcnt_u64(lines[i - 1]      &
                                                  lines[i - 1] >> 1 &
                                                  lines[i    ] << 1 &
                                                  lines[i    ] >> 1 &
                                                  ~lines[i] & 0b0111111111111111111111111111111111111111111111111111111111111110));

        result += static_cast<int>(_mm_popcnt_u64(lines[i - 1]      &
                                                  lines[i - 1] >> 1 &
                                                  lines[i    ] >> 1 &
                                                  ~lines[i] & 0b0000000000000000000000000000000000000000000000000000000000000001));

        result += static_cast<int>(_mm_popcnt_u64(lines[i - 1]      &
                                                  lines[i    ] << 1 &
                                                  ~lines[i] & 0b1000000000000000000000000000000000000000000000000000000000000000));
      }

      return result;
    }

    __forceinline auto small_polyhex(int polyhex_index, int action_index) const noexcept {
      if (action_index == 12 * 64 * 64) {
        return 0;
      }

      return 16 - static_cast<int>(_polyhexes[polyhex_index].shapes()[0].blocks().count()) + 1;
    }

  public:
    __forceinline auto operator()(const bitmap& blocks, int polyhex_index, int action_index) const noexcept {
      const auto& lines = evaluate::lines(blocks);

      return A * static_cast<int>(blocks.count()) - B * edge(lines) - C * one_hole(lines) + D * small_polyhex(polyhex_index, action_index);
    }
  };
}
