#pragma once

#include <cmath>
#include <locale>

#include "rapidcheck/seq/Transform.h"
#include "rapidcheck/seq/Create.h"
#include "rapidcheck/Compat.h"

namespace rc {
namespace shrink {
namespace detail {

template <typename T>
class TowardsSeq {
public:
  TowardsSeq(T value, T target)
      : m_value(value)
      , m_target(target)
      , m_diff((target < value) ? (value - target) : (target - value))
      , m_down(target < value) {}

  Maybe<T> operator()() {
    if (m_diff == 0)
      return Nothing;

    if constexpr (std::is_integral_v<T>) {
      m_diff >>= 1;
    } else {
      m_diff /= 2;
    }

    return m_down ? (m_value - m_diff) : (m_value + m_diff);
  }

private:
  T m_value;
  T m_target;
  std::make_unsigned_t<T> m_diff;
  bool m_down;
};

template <typename Container>
class RemoveChunksSeq {
public:
  RemoveChunksSeq(Container elements)
      : m_elements(std::move(elements))
      , m_start(0)
      , m_size(m_elements.size()) {}

  Maybe<Container> operator()() {
    if (m_size == 0)
      return Nothing;

    Container result;
    result.reserve(m_elements.size() - m_size);

    std::copy_n(begin(m_elements), m_start, std::back_inserter(result));
    std::copy(begin(m_elements) + m_start + m_size,
              end(m_elements),
              std::back_inserter(result));

    updateIndices();
    return result;
  }

private:
  void updateIndices() {
    if (m_start + m_size >= m_elements.size()) {
      m_size--;
      m_start = 0;
    } else {
      m_start++;
    }
  }

  Container m_elements;
  std::size_t m_start;
  std::size_t m_size;
};

template <typename Container, typename Shrink>
class EachElementSeq {
public:
  EachElementSeq(Container elements, Shrink shrink)
      : m_elements(std::move(elements))
      , m_shrink(std::move(shrink))
      , m_index(0) {
    if (!m_elements.empty()) {
      m_currentShrink = m_shrink(m_elements[0]);
    }
  }

  Maybe<Container> operator()() {
    while (true) {
      if (auto nextValue = m_currentShrink.next()) {
        auto result = m_elements;
        result[m_index] = std::move(*nextValue);
        return result;
      }

      if (++m_index >= m_elements.size()) {
        return Nothing;
      }

      m_currentShrink = m_shrink(m_elements[m_index]);
    }
  }

private:
  Container m_elements;
  Shrink m_shrink;
  Seq<typename Container::value_type> m_currentShrink;
  std::size_t m_index;
};

template <typename T>
Seq<T> integral(T value, std::true_type) {
  // The check for > min() is important since -min() == min() and we never
  // want to include self
  if ((value < 0) && (value > std::numeric_limits<T>::min())) {
    // Drop the zero from towards and put that before the negation value
    // so we don't have duplicate zeroes
    return seq::concat(seq::just<T>(static_cast<T>(0), static_cast<T>(-value)),
                       seq::drop(1, shrink::towards<T>(value, 0)));
  }

  return shrink::towards<T>(value, 0);
}

template <typename T>
Seq<T> integral(T value, std::false_type) {
  return shrink::towards<T>(value, 0);
}

} // namespace detail

template <typename Container>
Seq<Container> removeChunks(Container elements) {
  return makeSeq<detail::RemoveChunksSeq<Container>>(std::move(elements));
}

template <typename Container, typename Shrink>
Seq<Container> eachElement(Container elements, Shrink shrink) {
  return makeSeq<detail::EachElementSeq<Container, Shrink>>(std::move(elements),
                                                            std::move(shrink));
}

template <typename T>
Seq<T> towards(T value, T target) {
  return makeSeq<detail::TowardsSeq<T>>(value, target);
}

template <typename T>
Seq<T> integral(T value) {
  return detail::integral(value, std::is_signed<T>());
}

template <typename T, typename>
Seq<T> integral(T value);

template <typename T>
Seq<T> real(T value) {
  std::vector<T> shrinks;

  if (std::abs(value) > 0) {
    shrinks.push_back(T(0.0));
  }

  if (value < 0) {
    shrinks.push_back(-value);
  }

  T truncated = std::trunc(value);
  if (std::abs(truncated) < std::abs(value)) {
    shrinks.push_back(truncated);
  }

  return seq::fromContainer(shrinks);
}

Seq<bool> boolean(bool value) { return value ? seq::just(false) : Seq<bool>(); }

template <typename T>
Seq<T> character(T value) {
  const auto &locale = std::locale::classic();
  auto shrinks = seq::cast<T>(seq::concat(
      seq::fromContainer(std::string("abc")),
      // TODO this seems a bit hacky
      std::islower(static_cast<char>(value), locale)
          ? Seq<char>()
          : seq::just(static_cast<char>(std::tolower(value, locale))),
      seq::fromContainer(std::string("ABC123 \n"))));

  return seq::takeWhile(std::move(shrinks), [=](T x) { return x != value; });
}

} // namespace shrink
} // namespace rc
