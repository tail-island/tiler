#include <chrono>
#include <future>
#include <iostream>
#include <tuple>

#include <boost/range/algorithm.hpp>

#include <windows.h>

#include "chokudai_search.hpp"
#include "configuration.hpp"
#include "evaluate.hpp"
#include "game.hpp"
#include "io.hpp"

void main(int argc, char** argv) {
  const auto& starting_time = std::chrono::system_clock::now();

  std::cin.tie(0);
  std::ios::sync_with_stdio(false);

  const auto& [obstacle_blocks, polyhexes] = tiler::read_question();

  auto& solver_1 = tiler::chokudai_search<tiler::evaluate< 8, 3,   0, 29>, 100>(obstacle_blocks, polyhexes);
  auto& solver_2 = tiler::chokudai_search<tiler::evaluate<18, 5, 105,  0>, 400>(obstacle_blocks, polyhexes);

  auto& solver_1_future = std::async(std::launch::async, [&]() noexcept { ::SetThreadAffinityMask(::GetCurrentThread(), 0x00000001); return solver_1(); });
  auto& solver_2_future = std::async(std::launch::async, [&]() noexcept { ::SetThreadAffinityMask(::GetCurrentThread(), 0x00000004); return solver_2(); });

  solver_1_future.wait_until(starting_time + std::chrono::milliseconds(8000));
  solver_1.stop();

  solver_2_future.wait_until(starting_time + std::chrono::milliseconds(8000));
  solver_2.stop();

  const auto& result_1 = solver_1_future.get();
  const auto& result_2 = solver_2_future.get();

  const auto& result = [&]() noexcept {
    if (std::get<0>(result_1) == std::get<0>(result_2)) {
      return boost::count(std::get<1>(result_1), 12 * 64 * 64) > boost::count(std::get<1>(result_2), 12 * 64 * 64) ? result_1 : result_2;
    }

    return std::get<0>(result_1) > std::get<0>(result_2) ? result_1 : result_2;
  }();

  tiler::write_answer(std::get<1>(result));

  std::cerr << std::get<0>(result) << "\t" << (polyhexes.size() - boost::count(std::get<1>(result), 12 * 64 * 64)) << std::endl;

  std::quick_exit(0);
}
