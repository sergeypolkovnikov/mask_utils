#pragma once
#include <type_traits>
#include <stdexcept>
#include <iterator>

namespace utl {
    template<typename T>
    concept enum_type = std::is_enum_v<T>;

    template<enum_type T>
    constexpr auto to_underlying(T v) noexcept {
        return static_cast<std::underlying_type_t<T>>(v);
    }
}

namespace mask
{
    template<utl::enum_type Mask, utl::enum_type Element>
    constexpr Element define_base() {
        return Element{ 0 };
    }

    template<typename Mask, typename Element>
    concept base_defined = requires(Mask m) {
        { mask::define_base<Mask, Element>() } -> std::convertible_to<Element>;
    };

    namespace internal
    {
        template<utl::enum_type Mask, mask::base_defined<Mask> Element>
        constexpr size_t offset(Element const el, Element const base = mask::define_base<Mask, Element>()) noexcept {
            const auto diff = utl::to_underlying(el) - utl::to_underlying(base);
            return diff >= 0 ? static_cast<size_t>(diff) : 0;
        }


        template<utl::enum_type Enum>
        constexpr Enum clear_right_most_bit(Enum const val) noexcept {
            return static_cast<Enum>((utl::to_underlying(val) - 1) & utl::to_underlying(val));
        }

        template<utl::enum_type Mask, mask::base_defined<Mask> Element>
        constexpr Element transform_mask_to_value(Mask const mask, Element const base, const size_t index = 0) noexcept {
            const auto maskVal = utl::to_underlying(mask);
            return (maskVal & 1) == 0
                ? transform_mask_to_value<Mask, Element>(static_cast<Mask>(maskVal >> 1), base, index + 1)
                : static_cast<Element>(index + utl::to_underlying(base));
        }
    }

    template<utl::enum_type Mask, mask::base_defined<Mask>... Element>
    constexpr Mask set_mask(Element... elems) {
        Mask result = Mask{0};
        ([&]() mutable { result = set(result, elems); }(), ...);
        return result;
    }

    template<utl::enum_type Mask, mask::base_defined<Mask> Element>
    constexpr Mask set(Mask const mask, Element const el, Element const base = mask::define_base<Mask, Element>()) noexcept {
        return static_cast<Mask>(utl::to_underlying(mask) | 1 << internal::offset<Mask, Element>(el, base));
    }

    namespace internal
    {
        template<utl::enum_type Mask, mask::base_defined<Mask> Element>
        struct range
        {
            using value_type = Element;
            struct iterator
            {
                using iterator_category = std::input_iterator_tag;
                using value_type = typename range::value_type;
                using difference_type = std::ptrdiff_t;
                using pointer = value_type*;
                using reference = value_type;

                constexpr iterator() noexcept
                    : mask{ Mask{0} }
                    , base{ Element{0} }
                {}
                constexpr explicit iterator(const Mask mask_, Element const base_) noexcept
                    : mask{ mask_ }
                    , base{ base_ }
                {}
                constexpr iterator& operator++() noexcept { mask = clear_right_most_bit(mask); return *this; }
                constexpr iterator operator++(int) noexcept { iterator retval = *this; ++(*this); return retval; }
                constexpr bool operator==(iterator other) const noexcept { return mask == other.mask; }
                constexpr bool operator!=(iterator other) const noexcept { return !(*this == other); }
                constexpr reference operator*() const noexcept
                {
                    return transform_mask_to_value<Mask, Element>(mask, base);
                }

            private:
                Mask mask;
                Element base;
            };

            using const_iterator = iterator;

            constexpr range(Mask const mask_, Element const base_) noexcept
                : mask{ mask_ }
                , base{ base_ }
            {}
            constexpr range() noexcept = default;
            constexpr iterator begin() const noexcept { return iterator{ mask, base }; }
            constexpr iterator end() const noexcept { return iterator{ static_cast<Mask>(0), base }; }

        private:
            Mask mask;
            Element base;
        };
    }

    // split mask to elements
    // base is a first element which is represented by first bit in mask
    template<utl::enum_type Element, utl::enum_type Mask>
    constexpr auto split(Mask const mask, Element const base = mask::define_base<Mask, Element>()) {
        return internal::range<Mask, Element>{ mask, base };
    }

    template<utl::enum_type Mask, mask::base_defined<Mask> Element>
    constexpr bool contains(Mask const mask, Element const el, Element const base = mask::define_base<Mask, Element>()) noexcept {
        if (el < base) {
            return false;
        }
        return (utl::to_underlying(mask) & (1 << internal::offset<Mask>(el, base))) != 0;
    }
}