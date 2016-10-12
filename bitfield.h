#include <cstdint>
#include <type_traits>

namespace bitfield_
{
    template<typename... Ts>
    struct pack
    {
        static constexpr std::size_t size = sizeof...(Ts);
    };

    template<typename T1, typename T2, typename... Rest>
    static constexpr std::size_t total_bits(pack<T1, T2, Rest...>) noexcept
    {
        return T1::bits + total_bits(pack<T2, Rest...>());
    }

    template<typename Tag>
    static constexpr std::size_t total_bits(pack<Tag>) noexcept
    {
        return Tag::bits;
    }

    template<typename ThisTag, typename T1, typename T2, typename... Rest>
    static constexpr std::size_t position_of(pack<T1, T2, Rest...>) noexcept
    {
        return std::is_same<ThisTag, T1>::value ? 0 : T1::bits + position_of<ThisTag>(pack<T2, Rest...>());
    }

    template<typename ThisTag, typename Tag>
    static constexpr std::size_t position_of(pack<Tag>) noexcept
    {
        return std::is_same<ThisTag, Tag>::value ? 0 : Tag::bits;
    }

    template<std::size_t Bits, typename T>
    struct type_info
    {
        using type = T;

        static constexpr std::size_t bits = Bits;
        static constexpr type full_mask = T(~T(0));
    };

    template<std::size_t Size, typename Enable = void>
    struct make_type_info;

    template<std::size_t Size>
    struct make_type_info<Size, typename std::enable_if<(Size <= 8)>::type>
    {
        using type = type_info<8, uint8_t>;
    };

    template<std::size_t Size>
    struct make_type_info<Size, typename std::enable_if<(Size > 8 and Size <= 16)>::type>
    {
        using type = type_info<16, uint16_t>;
    };

    template<std::size_t Size>
    struct make_type_info<Size, typename std::enable_if<(Size > 16 and Size <= 32)>::type>
    {
        using type = type_info<32, uint32_t>;
    };

    template<std::size_t Size>
    struct make_type_info<Size, typename std::enable_if<(Size > 32 and Size <= 64)>::type>
    {
        using type = type_info<64, uint64_t>;
    };

    template<typename... Tags>
    using type_info_for = typename make_type_info<total_bits(pack<Tags...>{})>::type;

    template<typename TypeInfo, std::size_t Position, typename Tag>
    struct field_base
    {
        using tag_info = Tag;
        using field_type = typename tag_info::type;
        using type_info = TypeInfo;
        using value_type = typename type_info::type;

        static constexpr std::size_t position = Position;
        static constexpr std::size_t bits = tag_info::bits;
        static constexpr value_type sub_mask = value_type(type_info::full_mask >> (type_info::bits - bits));
        static constexpr value_type field_mask = value_type(~(sub_mask << position));

        constexpr field_type get(const value_type& value) const noexcept
        {
            return field_type((value >> position) & sub_mask);
        }

        template<typename U>
        void set(value_type& value, U sub_value) noexcept
        {
            value = (value & field_mask) | ((field_type(sub_value) & sub_mask) << position);
        }

        constexpr field_type max() const noexcept
        {
            return field_type(sub_mask);
        }
    };

    template<typename TypeInfo, std::size_t Position, typename Tag>
    constexpr typename TypeInfo::type field_base<TypeInfo, Position, Tag>::field_mask;

    template<typename TypeInfo, typename... Tags>
    struct fields : field_base<TypeInfo, position_of<Tags>(pack<Tags...>()), Tags>...{};
}

template<typename... Tags>
class bitfield : private bitfield_::fields<bitfield_::type_info_for<Tags...>, Tags...>
{
    using pack_type = bitfield_::pack<Tags...>;
    using type_info = bitfield_::type_info_for<Tags...>;
    using value_type = typename type_info::type;

    template<typename Tag>
    using field_class = bitfield_::field_base<type_info, bitfield_::position_of<Tag>(pack_type()), Tag>;

public:
    constexpr bitfield() noexcept
    {
    }

    template<typename U>
    constexpr bitfield(const U& value) noexcept :
        value_(value)
    {
    }

    template<typename Tag, typename U>
    void set(const U& sub_value) noexcept
    {
        static_cast<field_class<Tag>&>(*this).set(value_, sub_value);
    }

    template<typename Tag>
    typename Tag::type get() const noexcept
    {
        return static_cast<const field_class<Tag>&>(*this).get(value_);
    }

    template<typename Tag>
    typename Tag::type max() const noexcept
    {
        return static_cast<const field_class<Tag>&>(*this).max();
    }

    value_type& raw() noexcept
    {
        return value_;
    }

    const value_type& raw() const noexcept
    {
        return value_;
    }

private:
    value_type value_;
};
