module;

#include <span>
#include <cstring>
#include <utility>
#include <algorithm>

export module ArxConverter.ArxExplode;


import ArxConverter.Logger;

import ArxConverter.Container;


export class ArxExplode final
{
public:

	struct State final
	{
		static constexpr Size WindowSize = 0x2000ULL;

		static constexpr Size WindowMask = WindowSize - 1ULL;


		const Byte* inputPtr = nullptr;

		const Byte* inputEnd = nullptr;


		UInt32 bit_buff   = 0U;

		UInt32 extra_bits = 0U;


		UInt32 type       = 0U;

		UInt32 dsize_bits = 0U;

		UInt32 dsize_mask = 0U;


		Array<Byte, WindowSize> window{};

		Size windowHead = 0ULL;


		Array<UInt8, 0x100> LengthCodes{};

		Array<UInt8, 0x100> DistPosCodes{};


		Array<UInt8, 0x100> Offs2C34{};

		Array<UInt8, 0x100> Offs2D34{};

		Array<UInt8, 0x100> Offs2E34{};

		Array<UInt8, 0x100> Offs2EB4{};

		Array<UInt8, 0x100> ChBitsAsc{};


		Array<UInt8, 0x40> DistBits{};

		Array<UInt8, 0x10> LenBits{};

		Array<UInt8, 0x10> ExLenBits{};

		Array<UInt16, 0x10> LenBase{};
	};


private:

	const DynamicArray<Byte>& m_compressed;

	DynamicArray<Byte>        m_decompressed;


	Logger& m_logger;

public:

	ArxExplode() = delete;

   ~ArxExplode() = default;


	explicit ArxExplode(const DynamicArray<Byte>& compressed, Logger& logger) noexcept;


	[[nodiscard]] DynamicArray<Byte> releaseDecompressed() noexcept;


private:

	Void decompress();

	Void generateDecodeTables(std::span<UInt8> positions, std::span<const UInt8> startIndexes, std::span<const UInt8> lengthBits);

	Void generateAsciiTables(State& state);
};