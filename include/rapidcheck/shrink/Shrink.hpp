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
  using UInt = typename std::make_unsigned<T>::type;

  TowardsSeq(T value, T target)
      : m_value(std::move(value))
      , m_down(target < m_value) {
    UInt diff = m_down ? (m_value - target) : (target - m_value);
    while (diff > 0) {
      m_steps.push_back(diff);
      diff /= 2;
    }
  }

  Maybe<T> operator()() {
    if (m_index >= m_steps.size())
      return Nothing;
    UInt step = m_steps[m_index++];
    T ret = m_down ? m_value - step : m_value + step;
    return std::move(ret);
  }

private:
  T m_value;
  bool m_down;
  std::vector<UInt> m_steps;
  std::size_t m_index = 0;
};


template <typename Container>
class RemoveChunksSeq {
public:
  using Ptr = std::shared_ptr<const Container>;

  template <typename ContainerArg>
  explicit RemoveChunksSeq(ContainerArg &&elements)
      : m_elements(std::make_shared<const Container>(
            std::forward<ContainerArg>(elements)))
      , m_start(0)
      , m_size(m_elements->size()) {}

  Maybe<Container> operator()() {
    if (m_size == 0)
      return Nothing;

    Container result;
    result.reserve(m_elements->size() - m_size);

    const auto &vec = *m_elements;
    result.insert(result.end(), vec.begin(), vec.begin() + m_start);
    result.insert(result.end(), vec.begin() + m_start + m_size, vec.end());

    if ((m_size + m_start) >= vec.size()) {
      m_size--;
      m_start = 0;
    } else {
      m_start++;
    }

    return std::move(result);
  }

private:
  Ptr m_elements;
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
