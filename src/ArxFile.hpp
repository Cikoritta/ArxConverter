#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "ArxExplode.hpp"


[[nodiscard]] inline std::vector<std::byte> readFile(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);

	if (!file)
	{
		throw std::runtime_error("Couldn't open file");
	}

	const std::streamsize size = file.tellg();

	if (size <= 0LL)
	{
		throw std::runtime_error("File is empty or error reading size");
	}

	file.seekg(0ULL, std::ios::beg);

	std::vector<std::byte> buffer(static_cast<std::size_t>(size));

	if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
	{
		throw std::runtime_error("Couldn't read file");
	}

	return buffer;
}

[[nodiscard]] inline std::vector<std::byte> decompressFile(const std::vector<std::byte>& compressed)
{
	auto decompressed = STD_Explode(std::span(compressed));

	if (decompressed.empty())
	{
		throw std::runtime_error("Couldn't decompress the file");
	}

	return decompressed;
}