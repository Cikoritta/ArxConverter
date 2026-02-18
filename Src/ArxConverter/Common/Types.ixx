module;

#include <string>
#include <cstddef>
#include <cstdint>

export module ArxConverter.Types;


export using Int8  = std::int8_t;

export using Int16 = std::int16_t;

export using Int32 = std::int32_t;

export using Int64 = std::int64_t;


export using UInt8  = std::uint8_t;

export using UInt16 = std::uint16_t;

export using UInt32 = std::uint32_t;

export using UInt64 = std::uint64_t;


export using Float32 = float;

export using Float64 = double;


export using Size  = std::size_t;

export using Count = Size;

export using Index = Size;


export using Char8   = char;

export using CString = Char8*;


export using String      = std::string;

export using StringView  = std::string_view;


export using Bool = bool;


export using Byte = std::byte;


export using Void = void;