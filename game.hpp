#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <vector>

#include <intrin.h>

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
// #include <boost/range/numeric.hpp>

namespace tiler {
  class bitmap final {
    std::array<std::uint64_t, 64> _lines;

  public:
    bitmap(const std::array<std::uint64_t, 64>& lines) noexcept: _lines(lines) {
      ;
    }

    bitmap() noexcept: bitmap(std::array<std::uint64_t, 64>()) {
      ;
    }

    __forceinline const auto& lines() const noexcept {
      return _lines;
    }

    __forceinline auto test(int y, int x) const noexcept {
      return lines()[y] & static_cast<std::uint64_t>(1) << x;
    }

    __forceinline auto count() const noexcept {
      // return boost::accumulate(lines(), static_cast<size_t>(0), [](const auto& acc, const auto& line) noexcept { return acc + _mm_popcnt_u64(line); });

      auto result = static_cast<std::size_t>(0);

      for (auto i = 0; i < 64; ++i) {
        result += _mm_popcnt_u64(lines()[i]);
      }

      return result;
    }

    __forceinline auto all() const noexcept {
      return boost::algorithm::all_of(lines(), [](const auto& line) noexcept { return line == ~static_cast<std::uint64_t>(0); });
    }

    __forceinline auto empty() const noexcept {
      return boost::algorithm::all_of(lines(), [](const auto& line) noexcept { return line ==  static_cast<std::uint64_t>(0); });
    }

    __forceinline auto set(int y, int x) noexcept {
      _lines[y] |= static_cast<std::uint64_t>(1) << x;
    }

    __forceinline auto reset(int y, int x) noexcept {
      _lines[y] &= ~(static_cast<std::uint64_t>(1) << x);
    }

    __forceinline auto paint(int y, int x) noexcept {
      auto points = std::deque<std::tuple<int, int>>{{y, x}};

      while (!points.empty()) {
        const auto& [y, x] = points.front(); points.pop_front();

        if (test(y, x)) {
          continue;
        }

        const auto& l = [&]() noexcept {
          auto result = x;

          for (; result > 0; --result) {
            if (test(y, result - 1)) {
              break;
            }
          }

          return result;
        }();

        const auto& r = [&]() noexcept {
          auto result = x;

          for (; result < 63; ++result) {
            if (test(y, result + 1)) {
              break;
            }
          }

          return result;
        }();

        for (auto i = l; i <= r; ++i) {
          set(y, i);
        }

        const auto& scan_line = [&](int y, int l, int r) noexcept {
          if (y < 0 || y > 63) {
            return;
          }

          l = l >=  0 ? l :  0;
          r = r <= 63 ? r : 63;

          while (l <= r) {
            for (; l <= r; ++l) {
              if (!test(y, l)) {
                break;
              }
            }

            if (l > r) {
              break;
            }

            for (; l <= r; ++l) {
              if (test(y, l)) {
                break;
              }
            }

            points.emplace_back(y, l - 1);
          }
        };

        scan_line(y - 1, l    , r + 1);
        scan_line(y + 1, l - 1, r    );
      }
    }
  };

  class point final {
    int _y;
    int _x;

  public:
    point(int y, int x) noexcept: _y(y), _x(x) {
      ;
    }

    auto y() const noexcept {
      return _y;
    }

    auto x() const noexcept {
      return _x;
    }

    auto around_points() const noexcept {
      return std::array<point, 6>{point(y()    , x() + 1),
                                  point(y() - 1, x() + 1),
                                  point(y() - 1, x()    ),
                                  point(y()    , x() - 1),
                                  point(y() + 1, x() - 1),
                                  point(y() + 1, x()    )};
    }

    auto flip() const noexcept {
      return point(y(), 0 - x() - y());
    }

    auto rotate() const noexcept {
      return point(x() + y(), 0 - y());
    }
  };

  inline auto operator==(const point& point_1, const point& point_2) noexcept {
    return point_1.y() == point_2.y() && point_1.x() == point_2.x();
  }

  class polyhex_shape final {
    bitmap _blocks;
    bitmap _around_blocks;

    int _left;
    int _top;
    int _right;
    int _bottom;

  private:
    static auto create_blocks(const std::vector<point>& points) noexcept {
      auto result = bitmap();

      for (const auto& point: points) {
        result.set(point.y() + 32, point.x() + 32);
      }

      return result;
    }

    static auto create_around_blocks(const std::vector<point>& points) noexcept {
      auto result = bitmap();

      for (const auto& point: points) {
        for (const auto& around_point: point.around_points()) {
          result.set(around_point.y() + 32, around_point.x() + 32);
        }
      }

      for (const auto& point: points) {
        result.reset(point.y() + 32, point.x() + 32);
      }

      return result;
    }

    static auto create_left(const std::vector<point>& points) noexcept {
      return boost::min_element(points, [](const auto& point_1, const auto& point_2) noexcept { return point_1.x() < point_2.x(); })->x() + 32;
    }

    static auto create_top(const std::vector<point>& points) noexcept {
      return boost::min_element(points, [](const auto& point_1, const auto& point_2) noexcept { return point_1.y() < point_2.y(); })->y() + 32;
    }

    static auto create_right(const std::vector<point>& points) noexcept {
      return boost::max_element(points, [](const auto& point_1, const auto& point_2) noexcept { return point_1.x() < point_2.x(); })->x() + 32;
    }

    static auto create_bottom(const std::vector<point>& points) noexcept {
      return boost::max_element(points, [](const auto& point_1, const auto& point_2) noexcept { return point_1.y() < point_2.y(); })->y() + 32;
    }

  public:
    polyhex_shape(const std::vector<point>& points) noexcept: _blocks(create_blocks(points)), _around_blocks(create_around_blocks(points)), _left(create_left(points)), _top(create_top(points)), _right(create_right(points)), _bottom(create_bottom(points)) {
      ;
    }

    const auto& blocks() const noexcept {
      return _blocks;
    }

    const auto& around_blocks() const noexcept {
      return _around_blocks;
    }

    auto left() const noexcept {
      return _left;
    }

    auto top() const noexcept {
      return _top;
    }

    auto right() const noexcept {
      return _right;
    }

    auto bottom() const noexcept {
      return _bottom;
    }
  };

  class polyhex final {
    std::array<polyhex_shape, 12> _shapes;
    std::array<bool,          12> _can_ignores;

  private:
    static auto flip(const std::vector<point>& points) noexcept {
      return boost::copy_range<std::vector<point>>(points | boost::adaptors::transformed([](const auto& point) noexcept { return point.flip(); }));
    }

    static auto rotate(const std::vector<point>& points) noexcept {
      return boost::copy_range<std::vector<point>>(points | boost::adaptors::transformed([](const auto& point) noexcept { return point.rotate(); }));
    }

    static auto create_shapes(const std::vector<point>& points) noexcept {
      auto result = boost::container::static_vector<std::vector<point>, 12>();

      result.emplace_back(points);
      result.emplace_back(rotate(result.back()));
      result.emplace_back(rotate(result.back()));
      result.emplace_back(rotate(result.back()));
      result.emplace_back(rotate(result.back()));
      result.emplace_back(rotate(result.back()));

      result.emplace_back(flip(points));
      result.emplace_back(rotate(result.back()));
      result.emplace_back(rotate(result.back()));
      result.emplace_back(rotate(result.back()));
      result.emplace_back(rotate(result.back()));
      result.emplace_back(rotate(result.back()));

      return std::array<polyhex_shape, 12>{polyhex_shape(result[ 0]),
                                           polyhex_shape(result[ 1]),
                                           polyhex_shape(result[ 2]),
                                           polyhex_shape(result[ 3]),
                                           polyhex_shape(result[ 4]),
                                           polyhex_shape(result[ 5]),
                                           polyhex_shape(result[ 6]),
                                           polyhex_shape(result[ 7]),
                                           polyhex_shape(result[ 8]),
                                           polyhex_shape(result[ 9]),
                                           polyhex_shape(result[10]),
                                           polyhex_shape(result[11])};
    }

    static auto is_same_blocks(const bitmap& blocks_1, const bitmap& blocks_2, int y, int x) noexcept {
      for (auto i = 0, j = (i - y) & 0x003f; i < 64; ++i, ++j &= 0x003f) {
        if (blocks_1.lines()[i] != _rotr64(blocks_2.lines()[j], x)) {
          return false;
        }
      }

      return true;
    }

    static auto is_same_blocks(const bitmap& blocks_1, const bitmap& blocks_2) noexcept {
      for (auto y = -4; y <= 4; ++y) {
        for (auto x = -4; x <= 4; ++x) {
          if (is_same_blocks(blocks_1, blocks_2, y & 0x003f, x & 0x003f)) {
            return true;
          }
        }
      }

      return false;
    }

    static auto create_can_ignores(const std::array<polyhex_shape, 12>& shapes) noexcept {
      auto result = std::array<bool, 12>();

      for (auto i = 0; i < 11; ++i) {
        if (result[i]) {
          continue;
        }

        for (auto j = i + 1; j < 12; ++j) {
          if (result[j]) {
            continue;
          }

          result[j] = is_same_blocks(shapes[i].blocks(), shapes[j].blocks());
        }
      }

      return result;
    }

  public:
    polyhex(const std::vector<point>& points) noexcept: _shapes(create_shapes(points)), _can_ignores(create_can_ignores(_shapes)) {
      ;
    }

    const auto& shapes() const noexcept {
      return _shapes;
    }

    const auto& can_ignores() const noexcept {
      return _can_ignores;
    }
  };

  inline auto action_indice(const bitmap& blocks, const bitmap& obstacle_blocks, const polyhex& polyhex) noexcept {
    const auto& is_vacant = [&](const auto& t, const auto& y, const auto& x) noexcept {
      const auto& polyhex_shape = polyhex.shapes()[t];

      for (auto i = polyhex_shape.top(), j = (i - y) & 0x003f; i <= polyhex_shape.bottom(); ++i, ++j &= 0x003f) {
        if ((blocks.lines()[j] | obstacle_blocks.lines()[j]) & _rotr64(polyhex_shape.blocks().lines()[i], x)) {
          return false;
        }
      }

      return true;
    };

    const auto& is_connected = [&](const auto& t, const auto& y, const auto& x, const auto& is_first_time) noexcept {
      // 初回は、隣接していなくても合法。
      if (is_first_time) {
        return true;
      }

      const auto& polyhex_shape = polyhex.shapes()[t];

      // 左右にスクロールして、周囲の点だけが逆側からはみ出している場合があるので、マスクしなければなりません。
      const auto& mask = (((polyhex_shape.left() - x) & 0x003f) == 0 ? 0x7fffffffffffffff : 0xffffffffffffffff) & (((polyhex_shape.right() - x) & 0x003f) == 63 ? 0xfffffffffffffffe : 0xffffffffffffffff);

      // 周囲の点のtopとbottomを計算します。上下にスクロールして、周囲の点だけが逆側からはみ出している場合があるので、topやbottomをそのまま使うことはできません。
      const auto& top    = ((polyhex_shape.top()    - y) & 0x003f) ==  0 ? polyhex_shape.top()    : polyhex_shape.top()    - 1;
      const auto& bottom = ((polyhex_shape.bottom() - y) & 0x003f) == 63 ? polyhex_shape.bottom() : polyhex_shape.bottom() + 1;

      for (auto i = top, j = (i - y) & 0x003f; i <= bottom; ++i, ++j &= 0x003f) {
        if (blocks.lines()[j] & _rotr64(polyhex_shape.around_blocks().lines()[i], x) & mask) {
          return true;
        }
      }

      return false;
    };

    auto result = boost::container::static_vector<std::uint16_t, 12 * 64 * 64 + 1>{12 * 64 * 64};  // パスの分。

    for (auto t = 0; t < 12; ++t) {
      if (polyhex.can_ignores()[t]) {
        continue;
      }

      const auto& polyhex_shape = polyhex.shapes()[t];

      for (auto y = 0; y < 64; ++y) {  // yは、origin_yを表現しています。だから、yが大きくなると、上にスクロールします。で、ごめんなさい、募集要項のyとは向きが逆です。
        if (polyhex_shape.top() < y && y <= polyhex_shape.bottom()) {
          continue;
        }

        for (auto x = 0; x < 64; ++x) {  // xは、origin_xを表現しています。だから、xが大きくなると、左にスクロールします。で、ごめんなさい、募集要項のxとは向きが逆です。
          if (polyhex_shape.left() < x && x <= polyhex_shape.right()) {
            continue;
          }

          if (!is_vacant(t, y, x)) {
            continue;
          }

          if (!is_connected(t, y, x, blocks.empty())) {
            continue;
          }

          result.emplace_back(t << 12 | y << 6 | x);
        }
      }
    }

    return result;
  }

  inline auto next_blocks(const bitmap& blocks, const polyhex& polyhex, int action_index) noexcept {
    const auto& t = action_index >> 12;
    const auto& y = action_index >>  6 & 0x003f;
    const auto& x = action_index       & 0x003f;

    if (t == 12) {  // パスの場合。
      return blocks;
    }

    auto result = blocks.lines();

    const auto& polyhex_shape = polyhex.shapes()[t];

    for (auto i = polyhex_shape.top(), j = (i - y) & 0x003f; i <= polyhex_shape.bottom(); ++i, ++j &= 0x003f) {
      result[j] |= _rotr64(polyhex_shape.blocks().lines()[i], x);
    }

    return bitmap(result);
  }
}
