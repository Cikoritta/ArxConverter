module;

#include <filesystem>

export module ArxConverter.ArxExporter;


import ArxConverter.Logger;
import ArxConverter.Container;
import ArxConverter.ArxHeaders;


export class ArxExporter final
{
	const FtlHeaders&  m_headers;

	const FtlFileData& m_data;


	String             m_baseOutputDirectory;

	Logger&            m_logger;

public:

	ArxExporter() = delete;

   ~ArxExporter() = default;


	explicit ArxExporter(const FtlHeaders& headers, const FtlFileData& data, const String& outputDir, Logger& logger) noexcept;


	Void exportAll();

private:

	Void createDirectories() const;


	Void exportJson() const;

	Void exportXml()  const;

	Void exportObjMtl() const;

	Void exportGltf()   const;
};