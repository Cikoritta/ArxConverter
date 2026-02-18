module;

export module ArxConverter.ArxFile;


import ArxConverter.Logger;
import ArxConverter.Container;


export class ArxFile final
{
	const String& m_inputFile;

	Logger&       m_logger;


	DynamicArray<Byte> m_decompressed;

public:

	ArxFile() = delete;

   ~ArxFile() = default;


	explicit ArxFile(const String& inputFile, Logger& logger) noexcept;


	[[nodiscard]] const DynamicArray<Byte>& getDecompressed() const noexcept;

private:

	[[nodiscard]] DynamicArray<Byte> read();

	[[nodiscard]] DynamicArray<Byte> decompress(const DynamicArray<Byte>& compressed);
};