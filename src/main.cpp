#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <algorithm>
#include <ranges>
#include "mask_utils.h"

enum class MaskWithDefaultBase {
    Empty = 0,
};

enum class Element {
    Zero,
    One,
    Two,
};

enum class MaskWithDefinedBase {
    Empty = 0,
};

namespace mask {
    template<>
    constexpr Element define_base<MaskWithDefinedBase, Element>() {
        return Element::One;
    }
}

enum class MaskWithPreDefinedValues {
    Empty = 0,
    OneAndTwo = mask::combine(Element::One, Element::Two)
};

enum class MaskElemTogether {
    Empty,
    Fst = 1 << CHAR_BIT,
    Snd
};

namespace mask {
    template<>
    constexpr MaskElemTogether define_base<MaskElemTogether, MaskElemTogether>() {
        return MaskElemTogether::Fst;
    }
}

TEST_CASE("set/contains with default base", "[common]") {
    constexpr const auto m1 = mask::set(MaskWithDefaultBase::Empty, Element::One);
    REQUIRE(!mask::contains(m1, Element::Zero));
    REQUIRE(mask::contains(m1, Element::One));
    REQUIRE(!mask::contains(m1, Element::Two));
}

TEST_CASE("set/contains with defined base", "[common]") {
    constexpr const auto m1 = mask::set(MaskWithDefinedBase::Empty, Element::One);
    REQUIRE(!mask::contains(m1, Element::Zero));
    REQUIRE(mask::contains(m1, Element::One));
    REQUIRE(!mask::contains(m1, Element::Two));
}

TEST_CASE("set/contains with manual base", "[common]") {
    constexpr const auto m1 = mask::set(MaskWithDefinedBase::Empty, Element::Zero, Element::Zero);
    REQUIRE(!mask::contains(m1, Element::Two)); // The base is One. So the second bit is empty.
    REQUIRE(mask::contains(m1, Element::Two, Element::Two));
}

TEST_CASE("set/contains in one enum", "[common]") {
    constexpr const auto m1 = mask::set(MaskElemTogether::Empty, MaskElemTogether::Snd);
    REQUIRE(mask::contains(m1, MaskElemTogether::Snd));
    REQUIRE(!mask::contains(m1, MaskElemTogether::Fst));
}

TEST_CASE("combine", "[common]") {
    REQUIRE(mask::combine(Element::Zero) == 0b00000001);
    REQUIRE(mask::combine(Element::One, Element::Two) == 0b00000110);
    REQUIRE(mask::combine(Element::Two, Element::Zero) == 0b00000101);
}

TEST_CASE("combine in constexpr", "[common]") {
    static_assert(mask::contains(MaskWithPreDefinedValues::OneAndTwo, Element::One));
    static_assert(mask::contains(MaskWithPreDefinedValues::OneAndTwo, Element::Two));
    static_assert(!mask::contains(MaskWithPreDefinedValues::OneAndTwo, Element::Zero));
}

TEST_CASE("split in for-range loop", "[split]") {
    for (const auto el : mask::split<Element>(MaskWithPreDefinedValues::OneAndTwo)) {
        REQUIRE((el == Element::One || el == Element::Two));
    }
}

TEST_CASE("split in find", "[split]") {
    const auto range = mask::split<Element>(MaskWithPreDefinedValues::OneAndTwo);
    const auto it1 = std::ranges::find(range, Element::One);
    REQUIRE(it1 != range.end());
    const auto it2 = std::ranges::find(range, Element::Zero);
    REQUIRE(it2 == range.end());
}

TEST_CASE("split in transform and filter", "[split]") {
    auto filtered = mask::split<Element>(MaskWithPreDefinedValues::OneAndTwo)
        | std::views::filter([](auto el) { return el == Element::One; })
        | std::views::transform([](auto el) { return utl::to_underlying(el); });
    for (const auto i : filtered) {
        REQUIRE(i == 1);
    }
}