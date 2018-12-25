#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include "game.hpp"

namespace tiler {
  inline auto read_question() noexcept {
    int y_size, x_size; std::cin >> x_size; std::cin >> y_size;

    auto read_obstacle_blocks =
      [&]() noexcept {
        auto result = bitmap();

        int obstacle_size; std::cin >> obstacle_size;

        for (auto i = 0; i < obstacle_size; ++i) {
          int y, x; char separator; std::cin >> x >> separator >> y;

          result.set(y, x);
        }

        for (auto i = 0; i < 64; ++i) {
          for (auto j = 0; j < 64; ++j) {
            if (i >= y_size || j >= x_size) {
              result.set(i, j);
            }
          }
        }

        return result;
      };

    auto read_polyhex_points =
      [&]() noexcept {
        auto result = std::vector<point>();

        int point_size; std::cin >> point_size;
        for (auto i = 0; i < point_size; ++i) {
          int y, x; char separator; std::cin >> separator >> x >> separator >> y;

          result.emplace_back(y, x);
        }

        return result;
      };

    auto read_polyhexes =
      [&]() noexcept {
        auto result = std::vector<polyhex>();

        int polyhex_size; std::cin >> polyhex_size;
        for (auto i = 0; i < polyhex_size; ++i) {
          result.emplace_back(read_polyhex_points());
        }

        return result;
      };

    const auto& obstacle_blocks = read_obstacle_blocks();
    const auto& polyhexes       = read_polyhexes();

    return std::make_tuple(obstacle_blocks, polyhexes);
  }

  inline auto write_answer(const std::vector<std::uint16_t>& answer) noexcept {
    for (const auto& action_index: answer) {
      const auto& t = action_index >> 12;
      const auto& y = action_index >>  6 & 0x003f;
      const auto& x = action_index       & 0x003f;

      if (t == 12) {  // パスの場合。
        std::cout << "P" << std::endl;
        continue;
      }

      std::cout << ((32 - x + 64) % 64) << "," << ((32 - y + 64) % 64) << ";" << (t / 6) << ";" << ((t % 6) * 60) << std::endl;
    }
  }
}
