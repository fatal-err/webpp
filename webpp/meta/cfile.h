#ifndef WEBPP_META_CFILE
#define WEBPP_META_CFILE

#include <array>
#include <exception>
#include <string_view>
#include <type_traits>

/**
 * @brief The file class
 * @details This class is designed for constant files that will be produced from
 * source files on build-time. This class's only purpose is to work at
 * compile-time.
 */
class const_file {
    std::string_view _file_path;
    std::string_view _content;

  public:
    constexpr const_file(std::string_view const& file_path,
                         std::string_view const& content) noexcept
        : _file_path(file_path), _content(content) {}

    constexpr auto const& path() const noexcept { return _file_path; }

    constexpr auto const& content() const noexcept { return _content; }

    static constexpr const_file search(std::string_view const& path) noexcept {
        constexpr std::array<const_file, 2> files{
            const_file("file.json", "{\"value\": 10}"),
            const_file("config.json", "{}")};

        for (auto const& f : files)
            if (f.path() == path)
                return f;

        // default file
        return const_file(path, "");
    }
};

#endif // WEBPP_META_CFILE