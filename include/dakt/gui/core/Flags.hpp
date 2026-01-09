#pragma once

#include <type_traits>

namespace dakt::gui {

    /**
     * Flags utilities for strongly-typed enum classes used as bitmasks
     *
     * Usage:
     *      enum class WindowFlags : uint32_t { None=0, NoTitleBar=1<<0, ... };
     *
     *      DAKT_DECLARE_FLAGS(WindowFlags);
     *
     *      WindowFlags f = WindowFlags::NoTitleBar | WindowFlags::NoResize;
     *      if (hasFlag(f, WindowFlags::NoResize)). { ... }
     */
    
    template <typename Enum>
    struct EnableBitmaskOperators : std::false_type {};

    // Macro to enable bitmask operators for a given enum class
    #define DAKTLIB_DECLARE_FLAGS(EnumType) \
        template <> struct EnableBitmaskOperators<EnumType> : std::true_type {}

    // Underlying integral type helper
    template <typename Enum>
    using EnumUType = std::underlying_type_t<Enum>;

    // Convert enum to underlying
    template <typename Enum>
    constexpr EnumUType<Enum> toU(Enum e) noexcept {
        return static_cast<EnumUType<Enum>>(e);
    }

    // Operator overloads for enabled enums
    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, Enum>
    operator|(Enum a, Enum b) noexcept {
        return static_cast<Enum>(toU(a) | toU(b));
    }

    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, Enum>
    operator&(Enum a, Enum b) noexcept {
        return static_cast<Enum>(toU(a) & toU(b));
    }

    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, Enum>
    operator^(Enum a, Enum b) noexcept {
        return static_cast<Enum>(toU(a) ^ toU(b));
    }

    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, Enum>
    operator~(Enum a) noexcept {
        return static_cast<Enum>(~toU(a));
    }

    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, Enum&>
    operator|=(Enum& a, Enum b) noexcept {
        a = a | a;
        return a;
    }

    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, Enum&>
    operator&=(Enum& a, Enum b) noexcept {
        a = a & b;
        return a;
    }

    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, Enum&>
    operator^=(Enum& a, Enum b) noexcept {
        a = a ^ b;
        return a;
    }

    // Flag helper functions

    /**
     * @brief True if `flags` contains `flag`.
     */
    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, bool>
    hasFlag(Enum flags, Enum flag) noexcept {
        return (toU(flags) & toU(flag)) != 0;
    }

    /**
     * @brief True if `flags` contains all bits in `mask`.
     */
    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, bool>
    hasAllFlags(Enum flags, Enum mask) noexcept {
        return (toU(flags) & toU(mask)) == toU(mask);
    }

    /**
     * @brief True if `flags` contains any bit in `mask`.
     */
    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, bool>
    hasAnyFlags(Enum flags, Enum mask) noexcept {
        return (toU(flags) & toU(mask)) != 0;
    }

    /**
     * @brief Adds bits from `mask` into `flags`.
     */
    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, void>
    addFlags(Enum& flags, Enum mask) noexcept {
        flags |= mask;
    }

    /**
     * @brief Removes bits in `mask` from `flags`.
     */
    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, void>
    removeFlags(Enum& flags, Enum mask) noexcept {
        flags = static_cast<Enum>(toU(flags) & ~toU(mask));
    }

    /**
     * @brief Sets or clears bits in `mask` depending on `enabled`.
     */
    template <typename Enum>
    constexpr std::enable_if_t<EnableBitmaskOperators<Enum>::value, void>
    setFlag(Enum& flags, Enum mask, bool enabled) noexcept {
        if (enabled) addFlags(flags, mask);
        else removeFlags(flags, mask);
    }

} // namespace dakt::gui