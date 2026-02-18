module;

#include <filesystem>

module ArxConverter;


namespace fs = std::filesystem;


ArxConverter::ArxConverter(Int32 argc, CString argv[])
{
    m_logger.clear();

    m_logger.print<LogLevel::Info>("ArxConverter starting...");


    if (argc != 2 && argc != 3)
    {
        m_logger.print<LogLevel::Error>("Usage: ArxConverter.exe <file.ftl> [output_directory]");
    }


    m_inputPath = argv[1];

    if (!fs::exists(m_inputPath))
    {
        m_logger.print<LogLevel::Error>("Input file does not exist: \"{}\"", m_inputPath);
    }


    const fs::path inputPathObj{ m_inputPath };

    const String stemName = inputPathObj.stem().string();

    if (argc == 3)
    {
        m_outputBaseDir = (fs::path(argv[2]) / stemName).string();
    }
    else
    {
        m_outputBaseDir = (inputPathObj.parent_path() / stemName).string();
    }


    if (!fs::exists(m_outputBaseDir))
    {
        try
        {
            fs::create_directories(m_outputBaseDir);
        }
        catch (...)
        {
            m_logger.print<LogLevel::Error>("Failed to create output directory: \"{}\"", m_outputBaseDir);
        }
    }


    m_logger.print<LogLevel::Info>("Input file:  \"{}\"", m_inputPath);

    m_logger.print<LogLevel::Info>("Output dir:  \"{}\"", m_outputBaseDir);
}


Void ArxConverter::start()
{
    m_logger.print<LogLevel::Info>("Reading and Decompressing...");

    m_file = std::make_unique<ArxFile>(m_inputPath, m_logger);


    m_logger.print<LogLevel::Info>("Parsing Data...");

    m_parser = std::make_unique<ArxParser>(m_file->getDecompressed(), m_logger);


    m_logger.print<LogLevel::Info>("Exporting...");

    m_exporter = std::make_unique<ArxExporter>(m_parser->getHeaders(), m_parser->getData(),m_outputBaseDir, m_logger);


    m_exporter->exportAll();

    m_logger.print<LogLevel::Info>("Done.");
}