# bitfield

This class provides another way to create and manipulate bit fields in C++.

Example:

```c++
#include <iostream>
#include <bitset>
#include "bitfield.h"

struct value1
{
    using type = uint8_t;
    static constexpr unsigned bits = 4;
};

struct value2
{
    using type = uint8_t;
    static constexpr unsigned bits = 4;
};

// Only using boolean fields can be done to manipulate the bitfield as flags
struct boolflag
{
    using type = bool;
    static constexpr unsigned bits = 1;
};

struct value3
{
    using type = int16_t;
    static constexpr unsigned bits = 16;
};

struct value4
{
    using type = uint8_t;
    static constexpr unsigned bits = 7;
};

// Bits are defined here (from LSB to MSB)
using my_bitfield = bitfield<value1, value2, boolflag, value3, value4>;

// my_bitfield will then store an uint32_t value:

// MSB                                              LSB
// 0000000  0000000000000000  0          0000      0000
// value4   value3            boolflag   value2  value1

int main()
{
    my_bitfield bf(0);
    bf.set<value1>(3);
    bf.set<boolflag>(true);
    bf.set<value3>(-100);

    std::cout << std::boolalpha;
    std::cout << int(bf.get<value1>()) << std::endl;
    std::cout << int(bf.get<value2>()) << std::endl;
    std::cout << bf.get<boolflag>() << std::endl;
    std::cout << bf.get<value3>() << std::endl;
    std::cout << int(bf.get<value4>()) << std::endl;
    
    // Maximum values (for unsigned types only)
    std::cout << "Max for value2: " << int(bf.max<value2>()) << std::endl;
    std::cout << "Max for value4: " << int(bf.max<value4>()) << std::endl;

    std::bitset<32> bs(bf.raw());
    std::cout << bs << std::endl;
    return 0;
}
```

Output (tested on GCC 4.9.2 and clang 3.5.0):

```
3
0
true
-100
0
Max for value2: 15
Max for value4: 127
00000001111111110011100100000011
```

Bitfields with less than or equal to 8 bits will store an uint8_t, and so on for 16, 32 and 64 bits.

You should only use signed integer types and floating point types with their whole size, and not rely on max() for them.
