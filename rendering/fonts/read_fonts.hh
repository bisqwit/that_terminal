#include <map>
#include <string>
#include <vector>

using FontsInfo = std::map<std::pair<std::string/*filename*/, std::pair<unsigned/*x*/,unsigned/*y*/>>,
                           std::pair<std::string/*guessed encoding*/, std::vector<bool>/*supported characters*/>>;

FontsInfo ReadFontsList();
