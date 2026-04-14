#ifndef COMMON_HPP_
#define COMMON_HPP_

template <typename T, std::size_t N>
constexpr std::size_t array_size(T const (&)[N]) noexcept {
    return N;
}

constexpr float clamp(float x, float min, float max) {
    return (x < min) ? min : ((x > max) ? max : x);
}

template<typename T, typename U>
constexpr T mapRange(U value, U inMin, U inMax, T outMin, T outMax) {
    return static_cast<T>(outMin + (static_cast<T>(value) - static_cast<T>(inMin)) * 
                         (outMax - outMin) / (static_cast<T>(inMax) - static_cast<T>(inMin))
);}

#endif