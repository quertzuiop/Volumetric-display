#include "../../renderer/include/types.h"
#include <stdexcept>
#include <string>
#include <string_view>


template<typename T>
concept optionType = std::same_as<T, int> ||
                     std::same_as<T, bool> ||
                     std::same_as<T, std::string> ||
                     std::same_as<T, float>;

template<optionType T>
T getOption(std::string_view argname, int argc, char* argv[]) {
    for (int i = 0; i < argc - 1; i++) {
        if (argv[i] == argname) {
            if constexpr(std::same_as<T, int>) {
                if (i + 1 < argc) {
                    return atoi(argv[i+1]);
                }
            } 
            else if constexpr(std::same_as<T, bool>) {
                return true;
            }
            else if constexpr(std::same_as<T, std::string>) {
                if (i + 1 < argc) {
                    return argv[i+1];
                }
            }
            if constexpr(std::same_as<T, float>) {
                if (i + 1 < argc) {
                    return atof(argv[i+1]);
                }
            } 
        }
    }
    if constexpr(std::same_as<T, bool>) {
        return false; //dont expect argument
    }

    throw std::invalid_argument("argument not found: " + std::string(argname));
}
