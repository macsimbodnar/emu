#include <string>
#include "nes.hpp"
#include "log.hpp"


// MAIN
int main(int argc, char **argv) {

    std::string cartridge_file;

    if (argc > 1) {
        cartridge_file = std::string(argv[1]);
    } else {
        LOG_E("No cartridge provided");
        return 1;
    }

    NES nes;
    bool res = nes.insert_cartridge(cartridge_file);

    if (!res) {
        LOG_E("Failed to load the cartridge");
        return 1;
    }

    LOG_I("All good");
    return 0;
}