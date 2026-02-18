module;

#include <fstream>
#include <filesystem>

module ArxConverter.ArxFile;


import ArxConverter.ArxExplode;


ArxFile::ArxFile(const String& inputFile, Logger& logger) noexcept : m_inputFile{ inputFile }, m_logger{ logger }
{
	if (const auto rawData = read(); !rawData.empty())
    {
        m_decompressed = decompress(rawData);
    }
}


const DynamicArray<Byte>& ArxFile::getDecompressed() const noexcept
{
    return m_decompressed;
}


DynamicArray<Byte> ArxFile::read()
{
	std::ifstream file(m_inputFile, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        m_logger.print<LogLevel::Error>("Couldn't open file: \"{}\"", m_inputFile);
    }

    const auto size = file.tellg();

    if (size <= 0)
    {
        m_logger.print<LogLevel::Error>("File is empty or invalid size: \"{}\"", m_inputFile);
    }

    file.seekg(0, std::ios::beg);

    DynamicArray<Byte> buffer;

    try
    {
        buffer.resize(size);
    }
    catch (...)
    {
        m_logger.print<LogLevel::Error>("Memory allocation failed for reading \"{}\"", m_inputFile);
    }

    if (!file.read(reinterpret_cast<Char8*>(buffer.data()), size))
    {
        m_logger.print<LogLevel::Error>("Failed to read content of file: \"{}\"", m_inputFile);
    }

    return buffer;
}

DynamicArray<Byte> ArxFile::decompress(const DynamicArray<Byte>& compressed)
{
    ArxExplode exploder(compressed, m_logger);

    return exploder.releaseDecompressed();
}