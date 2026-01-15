#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "pklib/pklib.h"


struct ExplodeParam
{
    std::span<const std::byte>  source;

    std::vector<std::byte>      destination;

    std::size_t                 sourceOffset = 0ULL;

    bool                        failed = false;
};


[[nodiscard]] inline std::uint32_t ReadCompressed(char* buffer, std::uint32_t* size, void* userParam)
{
    auto& p = *static_cast<ExplodeParam*>(userParam);

    if (p.sourceOffset >= p.source.size())
    {
        return 0UI32;
    }

    const std::size_t remaining = p.source.size() - p.sourceOffset;

    const std::size_t toCopy = std::min<std::size_t>(*size, remaining);


    std::memcpy(buffer, p.source.data() + p.sourceOffset, toCopy);


    p.sourceOffset += toCopy;

    *size = static_cast<std::uint32_t>(toCopy);


    return *size;
}

inline void WriteUncompressed(char* buffer, std::uint32_t* size, void* userParam)
{
    auto& p = *static_cast<ExplodeParam*>(userParam);

    if (p.failed || *size == 0UI32)
    {
        return;
    }

    const std::size_t newSize = p.destination.size() + *size;

    try
    {
        p.destination.resize(newSize);
    }
    catch (...)
    {
        p.failed = true;

        return;
    }

    std::memcpy(p.destination.data() + newSize - *size, buffer, *size);
}


[[nodiscard]] inline std::vector<std::byte> STD_Explode(std::span<const std::byte> input)
{
    if (input.empty())
    {
        return {};
    }

    char workBuffer[CMP_BUFFER_SIZE];


    ExplodeParam param{};

    param.source = input;


    if (const std::uint32_t err = explode(ReadCompressed, WriteUncompressed, workBuffer, &param); err != 0UI32 || param.failed)
    {
        return {};
    }

    return std::move(param.destination);
}