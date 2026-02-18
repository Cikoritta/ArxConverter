module;

#include <span>

export module ArxConverter.ArxParser;


import ArxConverter.Logger;
import ArxConverter.Container;
import ArxConverter.ArxHeaders;


export class ArxParser final
{
	const DynamicArray<Byte>& m_data;


	Logger& m_logger;


	FtlHeaders  m_headers;

	FtlFileData m_fileData;

public:

	ArxParser() = delete;

   ~ArxParser() = default;


	explicit ArxParser(const DynamicArray<Byte>& data, Logger& logger) noexcept;


	[[nodiscard]] const FtlHeaders&  getHeaders() const noexcept;

	[[nodiscard]] const FtlFileData& getData()    const noexcept;

private:

	void parse();

	void parseHeaders(Size& pos);

	void parseData(Size& pos);


	template <typename Type>
	const Type* mapRaw(Size offset) const;
};