#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <boost/heap/priority_queue.hpp>
#include <boost/range/algorithm.hpp>

#include "game.hpp"

namespace tiler {
  template <typename Evaluate, int A>
  class chokudai_search final {
    class state final {
      bitmap                     _blocks;
      std::vector<std::uint16_t> _answer;
      int                        _value;

    public:
      state(const bitmap& blocks, const std::vector<std::uint16_t>& answer, int value) noexcept: _blocks(blocks), _answer(answer), _value(value) {
        ;
      }

      const auto& blocks() const noexcept {
        return _blocks;
      }

      const auto& answer() const noexcept {
        return _answer;
      }

      auto value() const noexcept {
        return _value;
      }
    };

    struct compare_state {
      auto operator()(const state& state_1, const state& state_2) const noexcept {
        return state_1.value() < state_2.value();
      }
    };

    using state_queue = boost::heap::priority_queue<state, boost::heap::compare<compare_state>>;

    const bitmap&               _obstacle_blocks;
    const std::vector<polyhex>& _polyhexes;
    const Evaluate              _evaluate;

    std::atomic<bool>           _stop;

    std::unordered_map<std::size_t, int> _visited_states;  // operator()の自動変数にした方が美しいのですけど、破棄のコスト削減のためにメンバーにします。
    std::vector<state_queue>             _queues;          // operator()の自動変数にした方が美しいのですけど、破棄のコスト削減のためにメンバーにします。

  public:
    chokudai_search(const bitmap& obstacle_blocks, const std::vector<polyhex>& polyhexes) noexcept: _obstacle_blocks(obstacle_blocks), _polyhexes(polyhexes), _evaluate(obstacle_blocks, polyhexes), _stop(false), _visited_states(), _queues(_polyhexes.size() + 1) {
      ;
    }

  private:
    auto state_hash(const bitmap& blocks, int polyhex_index) const noexcept {
      auto result = static_cast<std::size_t>(0);

      for (auto i = 0; i < 64; ++i) {
        boost::hash_combine(result, blocks.lines()[i]);
      }

      boost::hash_combine(result, polyhex_index);

      return result;
    }

    auto next_answer(const std::vector<std::uint16_t>& answer, std::uint16_t action_index) const noexcept {
      auto result = std::vector<std::uint16_t>(); result.reserve(answer.size() + 1);

      result.assign(std::begin(answer), std::end(answer));
      result.emplace_back(action_index);

      return result;
    }

  public:
    auto operator()() /*const*/ noexcept {
      // auto visited_states = std::unordered_map<std::size_t, int>();

      // auto queues = std::vector<state_queue>(_polyhexes.size() + 1);
      _queues[0].emplace(bitmap(), std::vector<std::uint16_t>(), 0);

      while (std::any_of(std::begin(_queues), std::end(_queues) - 1, [](const auto& queue) noexcept { return !queue.empty(); }) && !_stop) {
        for (auto i = 0; i < static_cast<int>(_polyhexes.size()); ++i) {
          if (_queues[i].empty()) {
            continue;
          }

          const auto& top = _queues[i].top();

          const auto& blocks = top.blocks();
          const auto& answer = top.answer();

          if (_stop) {
            if (_queues.back().empty()) {
              _queues[i + 1].emplace(blocks, next_answer(answer, 12 * 64 * 64), top.value());
            }

            continue;
          }

          for (const auto& action_index: action_indice(blocks, _obstacle_blocks, _polyhexes[i])) {
            const auto& next_blocks = tiler::next_blocks(blocks, _polyhexes[i], action_index);
            const auto& next_hash   = state_hash(next_blocks, i);
            const auto& next_answer = chokudai_search::next_answer(answer, action_index);

            const auto& it          = _visited_states.find(next_hash);
            const auto& pass_count  = static_cast<int>(boost::count(next_answer, 12 * 64 * 64));

            if (it != std::end(_visited_states) && it->second >= pass_count) {
              continue;
            }

            _visited_states.emplace(state_hash(next_blocks, i), pass_count);

            _queues[i + 1].emplace(next_blocks, next_answer, _evaluate(next_blocks, i, action_index));
          }

          if (_queues[i + 1].size() > A) {
            auto queue = state_queue();

            for (auto j = 0; j < A; ++j) {
              queue.push(_queues[i + 1].top()); _queues[i + 1].pop();
            }

            _queues[i + 1].swap(queue);
          }

          _queues[i].pop();
        }
      }

      const auto& best_state = std::max_element(
        std::begin(_queues.back()),
        std::end(_queues.back()),
        [&](const auto& state_1, const auto& state_2) noexcept  {
          const auto& count_1 = state_1.blocks().count();
          const auto& count_2 = state_2.blocks().count();

          if (count_1 == count_2) {
            return boost::count(state_1.answer(), 12 * 64 * 64) < boost::count(state_2.answer(), 12 * 64 * 64);
          }

          return count_1 < count_2;
        });

      return std::make_tuple(best_state->blocks().count(), best_state->answer());
    }

    auto stop() noexcept {
      _stop = true;
    }
  };
}
