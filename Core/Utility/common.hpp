#ifndef COMMON_HPP_
#define COMMON_HPP_

template <typename T, std::size_t N>
constexpr std::size_t array_size(T const (&)[N]) noexcept {
    return N;
}

#endif