#include <string>

template<typename T>
concept optionType = std::same_as<T, int> ||
                     std::same_as<T, bool> ||
                     std::same_as<T, std::string>;


template<optionType T>
T getOption(std::string_view argname, int argc, char* argv[]);
