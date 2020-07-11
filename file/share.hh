#include <string>
#include <string_view>
#include <filesystem>

void
    SaveArg0(const char* arg0);

std::pair<std::filesystem::path, std::filesystem::file_status>
    FindShareFile(const std::filesystem::path& file_to_find, std::initializer_list<std::string_view> extra_paths = {});

std::pair<std::filesystem::path, std::filesystem::file_status>
    FindCacheFile(std::string_view file_to_find, bool is_file);
