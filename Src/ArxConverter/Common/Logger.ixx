module;

#include <cstdlib>
#include "fmt/color.h"

#ifdef _WIN32

#include "Windows.h"

#endif

export module ArxConverter.Logger;


export import ArxConverter.Container;


export using fmt::format;


export class Logger final
{
    struct Preset final
    {
       String   label;

       fmt::rgb color;
    };

    const Array<Preset, 2ULL> m_preset =
    {{
       { "INFO",  { 0xB4, 0xE1, 0xB4 } },
       { "ERROR", { 0xE1, 0xB4, 0xB4 } }
    }};

public:

    Logger();

   ~Logger() = default;


    enum class Level : UInt8
    {
       Info    = 0U,
       Error   = 1U
    };

    template<Level level, typename... Args>
    Void print(fmt::format_string<Args...> format, Args&&... args) const;

    Void clear() const;
};


export using LogLevel = Logger::Level;


Logger::Logger()
{
    #ifdef _WIN32

    const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0UL;


    GetConsoleMode(hOut, &dwMode);


    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;


    SetConsoleMode(hOut, dwMode);

    #endif
}


Void Logger::clear() const
{
    #ifdef _WIN32

    system("cls");

    #else

    system("clear");

    #endif
}


template<LogLevel level, typename ... Args>
Void Logger::print(fmt::format_string<Args...> format, Args&&... args) const
{
    const String message = fmt::format(format, std::forward<Args>(args)...);

    const auto& [ label, color ] = m_preset.at(static_cast<Index>(level));


    fmt::print(fmt::fg(color), "[{}] {}\n", label, message);


    if constexpr (level == Level::Error)
    {
       throw;
    }
}