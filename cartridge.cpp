#include "cartridge.hpp"
#include "log.hpp"


// iNES Format Header
struct ines_header_t {
    char name[4];
    uint8_t prg_rom_chunks;
    uint8_t chr_rom_chunks;
    uint8_t mapper1;
    uint8_t mapper2;
    uint8_t prg_ram_size;
    uint8_t tv_system1;
    uint8_t tv_system2;
    char unused[5];
};


Cartridge::Cartridge(const std::string &file) : valid(false), mirror(HORIZONTAL) {
    FILE *fp = fopen(file.c_str(), "rb");
    long size;
    ines_header_t header;

    if (fp == nullptr) {
        log_e("Can not open the file " + file);
        return;
    }

    // get the file size
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size <= sizeof(ines_header_t))  {
        log_e("Invalid cartridge file " + file);
        return;
    }

    // Read the header
    if (fread(&header, sizeof(ines_header_t), 1, fp) != sizeof(ines_header_t)) {
        log_e("Failed read the cartridge from file: " + file);
        return;
    }

    // If a "trainer" exists we just need to read past
    // it before we get to the good stuff
    if (header.mapper1 & 0x04) {
        fseek(fp, 0, 512);
    }

    // Determine Mapper ID
    mapper_id = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
    mirror = (header.mapper1 & 0x01) ? VERTICAL : HORIZONTAL;

    // "Discover" File Format
    uint8_t file_type = 1;

    switch (file_type) {
    case 1:
        prg_banks = header.prg_rom_chunks;
        prg_memory.resize(prg_banks * 16384);

        if (fread((uint8_t *) prg_memory.data(), sizeof(uint8_t), prg_memory.size(), fp) !=
                prg_memory.size()) {

            log_e("Failed read the cartridge program memory from file: " + file);
            return;
        }

        chr_banks = header.chr_rom_chunks;
        chr_memory.resize(chr_banks * 8192);

        if (fread((uint8_t *) chr_memory.data(), sizeof(uint8_t), chr_memory.size(), fp) !=
                chr_memory.size()) {

            log_e("Failed read the cartridge character memory from file: " + file);
            return;
        }

        break;

    case 0:
    case 2:
    default:
        log_e("Unexpected file type");
        return;
        break;
    }

    fclose(fp);

    // Load appropriate mapper
    switch (mapper_id) {
    case 0:
        mapper = Mapper_000(prg_memory, chr_banks);
        break;

    default:
        log_e("Unexpected mapper id");
        return;
    }

    valid = true;
    return;
}


Cartridge::~Cartridge()
{
    if (mapper) {
        delete mapper;
    }
}