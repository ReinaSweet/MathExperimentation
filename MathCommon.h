#pragma once

#define ENUM_OPS(Enum) \
    inline           Enum& operator|=(Enum& lhs, Enum rhs) { return lhs = (Enum)((std::underlying_type<Enum>::type)lhs | (std::underlying_type<Enum>::type)rhs); } \
    inline           Enum& operator&=(Enum& lhs, Enum rhs) { return lhs = (Enum)((std::underlying_type<Enum>::type)lhs & (std::underlying_type<Enum>::type)rhs); } \
    inline           Enum& operator^=(Enum& lhs, Enum rhs) { return lhs = (Enum)((std::underlying_type<Enum>::type)lhs ^ (std::underlying_type<Enum>::type)rhs); } \
    inline           Enum& operator+=(Enum& lhs, std::underlying_type<Enum>::type rhs) { return lhs = (Enum)((std::underlying_type<Enum>::type)lhs + rhs); } \
    inline           Enum& operator-=(Enum& lhs, std::underlying_type<Enum>::type rhs) { return lhs = (Enum)((std::underlying_type<Enum>::type)lhs - rhs); } \
    inline           Enum& operator++(Enum& lhs) { return lhs = (Enum)((std::underlying_type<Enum>::type)lhs + (std::underlying_type<Enum>::type)1); } \
    inline           Enum& operator--(Enum& lhs) { return lhs = (Enum)((std::underlying_type<Enum>::type)lhs - (std::underlying_type<Enum>::type)1); } \
    inline           Enum  operator++(Enum& lhs, int) { const Enum lastValue = lhs; lhs = (Enum)((std::underlying_type<Enum>::type)lhs + (std::underlying_type<Enum>::type)1); return lastValue; } \
    inline           Enum  operator--(Enum& lhs, int) { const Enum lastValue = lhs; lhs = (Enum)((std::underlying_type<Enum>::type)lhs - (std::underlying_type<Enum>::type)1); return lastValue; } \
    inline constexpr Enum  operator| (Enum  lhs, Enum rhs) { return (Enum)((std::underlying_type<Enum>::type)lhs | (std::underlying_type<Enum>::type)rhs); } \
    inline constexpr Enum  operator& (Enum  lhs, Enum rhs) { return (Enum)((std::underlying_type<Enum>::type)lhs & (std::underlying_type<Enum>::type)rhs); } \
    inline constexpr Enum  operator^ (Enum  lhs, Enum rhs) { return (Enum)((std::underlying_type<Enum>::type)lhs ^ (std::underlying_type<Enum>::type)rhs); } \
    inline constexpr bool  operator! (Enum  value)         { return !(std::underlying_type<Enum>::type)value; } \
    inline constexpr Enum  operator~ (Enum  value)         { return (Enum)~(std::underlying_type<Enum>::type)value; } \
    inline constexpr Enum  operator+ (Enum  lhs, Enum rhs) { return (Enum)((std::underlying_type<Enum>::type)lhs + (std::underlying_type<Enum>::type)rhs); } \
    inline constexpr Enum  operator- (Enum  lhs, Enum rhs) { return (Enum)((std::underlying_type<Enum>::type)lhs - (std::underlying_type<Enum>::type)rhs); } \
    inline constexpr bool  operator< (Enum  lhs, Enum rhs) { return ((std::underlying_type<Enum>::type)lhs < (std::underlying_type<Enum>::type)rhs); } \
    inline constexpr bool  operator> (Enum  lhs, Enum rhs) { return ((std::underlying_type<Enum>::type)lhs > (std::underlying_type<Enum>::type)rhs); } \
    inline constexpr bool  operator<=(Enum  lhs, Enum rhs) { return ((std::underlying_type<Enum>::type)lhs <= (std::underlying_type<Enum>::type)rhs); } \
    inline constexpr bool  operator>=(Enum  lhs, Enum rhs) { return ((std::underlying_type<Enum>::type)lhs >= (std::underlying_type<Enum>::type)rhs); } \
    inline constexpr std::underlying_type<Enum>::type operator+(Enum value) { return (std::underlying_type<Enum>::type)value; }


#define ENUM_STRING_CONVERT_DEFINE(Enum, EnumMax, StringArray)\
    constexpr size_t StringArray##Size = sizeof(StringArray) / sizeof(std::string);\
    static_assert( StringArray##Size == (std::underlying_type<Enum>::type)(Enum::EnumMax));\
    const std::string& ToString(Enum value)\
    {\
        if ((std::underlying_type<Enum>::type)(value) < StringArray##Size )\
        {\
            return StringArray[(size_t)(std::underlying_type<Enum>::type)(value)];\
        }\
        static const std::string empty;\
        return empty;\
    }\
    template<>\
    Enum ToEnum(const std::string& value)\
    {\
        for (size_t i = 0; i < StringArray##Size ; ++i)\
        {\
            if (StringArray[i] == value)\
            {\
                return (Enum)(i);\
            }\
        }\
        return Enum::EnumMax;\
    }

#define ENUM_STRING_CONVERT_DECLARE(Enum)\
    const std::string& ToString(Enum value);\
    template<class T>\
    T ToEnum(const std::string& value);

#define ENUM_STRING_CONVERT(Enum, EnumMax, StringArray)\
    ENUM_STRING_CONVERT_DECLARE(Enum)\
    ENUM_STRING_CONVERT_DEFINE(Enum, EnumMax, StringArray)

#define _REPEAT_CASE_TEMPLATE_DEBUG_1(Offset, Func, ...)\
    case Offset: Func<Offset>(__VA_ARGS__);
#define _REPEAT_CASE_TEMPLATE_DEBUG_4(Offset, Func, ...)\
    _REPEAT_CASE_TEMPLATE_DEBUG_1(0 + Offset, Func, __VA_ARGS__ )\
    _REPEAT_CASE_TEMPLATE_DEBUG_1(1 + Offset, Func, __VA_ARGS__ )\
    _REPEAT_CASE_TEMPLATE_DEBUG_1(2 + Offset, Func, __VA_ARGS__ )\
    _REPEAT_CASE_TEMPLATE_DEBUG_1(3 + Offset, Func, __VA_ARGS__ )
#define _REPEAT_CASE_TEMPLATE_DEBUG_16(Offset, Func, ...)\
    _REPEAT_CASE_TEMPLATE_DEBUG_4(0 + Offset, Func, __VA_ARGS__ )\
    _REPEAT_CASE_TEMPLATE_DEBUG_4(4 + Offset, Func, __VA_ARGS__ )\
    _REPEAT_CASE_TEMPLATE_DEBUG_4(8 + Offset, Func, __VA_ARGS__ )\
    _REPEAT_CASE_TEMPLATE_DEBUG_4(12 + Offset, Func, __VA_ARGS__ )
#define _REPEAT_CASE_TEMPLATE_DEBUG_64(Offset, Func, ...)\
    _REPEAT_CASE_TEMPLATE_DEBUG_16(0 + Offset, Func, __VA_ARGS__ )\
    _REPEAT_CASE_TEMPLATE_DEBUG_16(16 + Offset, Func, __VA_ARGS__ )\
    _REPEAT_CASE_TEMPLATE_DEBUG_16(32 + Offset, Func, __VA_ARGS__ )\
    _REPEAT_CASE_TEMPLATE_DEBUG_16(48 + Offset, Func, __VA_ARGS__ )


#define REPEAT_CASE_TEMPLATE_DEBUG_64(Func, ...)\
    _REPEAT_CASE_TEMPLATE_DEBUG_64(0, Func, __VA_ARGS__ )
