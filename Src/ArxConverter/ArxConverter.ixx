module;

export module ArxConverter;


export import ArxConverter.Logger;

	   import ArxConverter.ArxFile;

       import ArxConverter.ArxParser;

       import ArxConverter.ArxExporter;


export class ArxConverter final
{
    Logger m_logger;


    String m_inputPath;

    String m_outputBaseDir;


	Unique<ArxFile>     m_file;

	Unique<ArxParser>   m_parser;

	Unique<ArxExporter> m_exporter;

public:

    ArxConverter() = delete;

   ~ArxConverter() = default;


    explicit ArxConverter(Int32 argc, CString argv[]);


    Void start();
};