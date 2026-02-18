#include "../../renderer/include/types.h"
#include "utils.h"

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
        }
    }
    if constexpr(std::same_as<T, bool>) {
        return false; //dont expect argument
    }

    throw invalid_argument("argument not found");
}
