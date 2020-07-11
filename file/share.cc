#include <filesystem>
#include <optional>
#include <fstream>
#include <string_view>
#include <string>
#include <vector>

#include <unistd.h> // For getuid()

using std::filesystem::path;

const char* arg0;

path arg0_path;

void SaveArg0(const char* a)
{
    arg0 = a;

    path p = arg0;
    arg0_path = std::filesystem::canonical(p).parent_path();
}

/*
std::pair<path, std::filesystem::file_status>
    FindShareFile(std::string_view file_to_find, std::initializer_list<std::string_view> extra_paths)
{
    return FindShareFile( std::filesystem::path(file_to_find), std::move(extra_paths));
}
*/

std::pair<path, std::filesystem::file_status>
    FindShareFile(const std::filesystem::path& file_to_find, std::initializer_list<std::string_view> extra_paths)
{
    using o = std::optional<std::pair<path, std::filesystem::file_status>>;
    auto try_path_fun = [](path test) -> o
    {
        auto status = std::filesystem::status(test);
        if(std::filesystem::exists(status))
            return o{ std::in_place_t{}, std::move(test), std::move(status) };
        return {};
    };

    #define try_path(s) \
        do { \
            auto temp = try_path_fun(s); \
            if(temp.has_value()) return std::move(temp).value(); \
        } while(0)

    try_path(arg0_path / "share" / file_to_find);

    path pff = path("that_terminal") / file_to_find;

    if(const char* homedir = std::getenv("HOME"))
        try_path(path(homedir)              / ".local/share" / pff);
    else if(const char* user = std::getenv("USER"))
        try_path(path("/home") / path(user) / ".local/share" / pff);

    try_path(path("/usr/local/share") / pff);
    try_path(path("/usr/share")       / pff);

    for(auto s: extra_paths)
        try_path(path(s) / file_to_find);

    // Default:
    return { file_to_find, std::filesystem::status(file_to_find) };

    #undef try_path
}

std::pair<path, std::filesystem::file_status>
    FindCacheFile(std::string_view file_to_find, bool is_file)
{
    using o = std::optional<std::pair<path, std::filesystem::file_status>>;
    auto try_path_fun = [is_file](path test, std::initializer_list<path> com) -> o
    {
        std::error_code err;
        /* Append all directory components */
        for(auto i = com.begin(); i != com.end(); ++i)
            if(!is_file || std::next(i) != com.end())
                test /= *i;
        std::filesystem::create_directories(test, err);
        auto status = std::filesystem::status(test);
        if(!err && std::filesystem::exists(status))
        {
            if(!is_file && std::filesystem::is_directory(status))
                return o{ std::in_place_t{}, std::move(test), std::move(status) };
            if(is_file)
            {
                /* Test if the file exists in this directory */
                auto test2 = test / *std::next(com.begin(), com.size()-1);
                status = std::filesystem::status(test2);
                if(std::filesystem::exists(status) && !std::filesystem::is_directory(status))
                    return o{ std::in_place_t{}, std::move(test2), std::move(status) };

                /* Test if the file could be created in this directory */
                try {
                    std::ofstream testfile(test2);
                    auto status2 = std::filesystem::status(test2);
                    /* Return the old status, before it existed */
                    if(std::filesystem::exists(status2))
                        return o{ std::in_place_t{}, std::move(test2), std::move(status) };
                }
                catch(...)
                {
                    /* Ignore errors */
                }
            }
        }
        return {};
    };

    #define try_path(s, specifics) \
        do { \
            auto temp = try_path_fun(s, specifics); \
            if(temp.has_value()) return std::move(temp).value(); \
        } while(0)

    path p("that_terminal");

    if(const char* homedir = std::getenv("HOME"))
        try_path(path(homedir)              / ".cache", (std::initializer_list<path>{p, file_to_find}) );
    else if(const char* user = std::getenv("USER"))
        try_path(path("/home") / path(user) / ".cache", (std::initializer_list<path>{p, file_to_find}) );

    std::string uid = std::to_string(getuid());
    path pu = path("that_terminal-" + uid);

    try_path(path("/run"),                           (std::initializer_list<path>{uid, file_to_find}) );
    try_path(std::filesystem::temp_directory_path(), (std::initializer_list<path>{pu, file_to_find}) );

    return { file_to_find, std::filesystem::status(file_to_find) };

    #undef try_path
}
