# mask_utils

## general

It's a simple wrapper on standard shift operations. E.g. the following code

```
enum class Flag {...};
const int mask = get_flags();
mask |= 1; // set first flag to true
const auto second_bit_set = (mask >> 1) & 1;
```

might be transformed into the next one

```
const auto mask = get_flags();
mask::set(mask, Flag::First);
const auto second_bit_set = mask::contains(mask, Flag::Second);
```

Another example is to split the mask to the separated flags and handle them in range

```
enum class Compiler {
    MSVC,
    GCC,
    Clang
};

std::string to_string(const Compiler t) noexcept {
    switch(t) {
        case Compiler::MSVC : return "MSVC";
        case Compiler::GCC : return "GCC";
        case Compiler::Clang : return "Clang";
        default: return "Invalid";
    }
}

enum class CompilerSupport {
    Empty,
    Compiles = mask::combine(Compiler::MSVC, Compiler::GCC),
    Tested = mask::combine(Compiler::MSVC)
};

auto join(std::string sep) {
    return [sep](std::string res, std::string const & next) {
        return res.empty() ? next : std::move(res) + sep + next;
    };
}

int main()
{
    const auto compilers = mask::split<Compiler>(CompilerSupport::Compiles);
    const auto range = compilers | std::views::transform(to_string);
    std::cout << std::accumulate(range.begin(), range.end(), std::string{}, join(", "));
}
```

## functions

### set(MaskType mask, ElementType elem, ElementType base = mask::define_base<MaskType>()) -> MaskType

Cast `elem` to the underlying type (int by default) and do the following:

```
const auto new_mask_value = mask | (1 << (elem - base)) 
```

It's UB when `elem - base` is greater than the size of the undelying type of the MaskType

### contains(MaskType mask, ElementType elem, ElementType base = mask::define_base<MaskType>()) -> bool

Cast `elem` to the underlying type (int by default) and do the following:

```
const auto contains = mask >> (1 << (elem - base)) & 1;
```

It's UB when `elem - base` is greater than the size of the undelying type of the MaskType

### split(MaskType mask) -> mask::internal::range<MaskType, ElementType>

Split the mask to different elements. E.g. mask 0b00001011 will be splited to three elements (0, 1 and 3). The result is presented in a range-like container (it sutisfies `std::range::range` concept requirements)

