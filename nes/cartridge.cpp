#include "log.hpp"
#include "cartridge.hpp"


#define NES_PRG_BANK_SIZE       16384
#define NES_CHR_BANK_SIZE       8192


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


bool Cartridge::load_cartridge(const std::string &cartridge_file) {

    // Cleanup the mem
    PRG_memory.clear();
    CHR_memory.clear();

    const char *c_file = cartridge_file.c_str();

    FILE *fp = fopen(c_file, "rb");
    long size;
    ines_header_t header;

    if (fp == nullptr) {
        LOG_E("Can not open the file %s", c_file);
        fclose(fp);
        return false;
    }

    // get the file size
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if ((size_t)size <= sizeof(ines_header_t))  {
        LOG_E("Invalid cartridge file %s", c_file);
        fclose(fp);
        return false;
    }

    // Read the header
    size_t b_read = fread((char *)&header, sizeof(char), sizeof(ines_header_t), fp);

    if (b_read != sizeof(ines_header_t)) {
        LOG_E("Failed read the cartridge from file %s", c_file);
        fclose(fp);
        return false;
    }

    // If a "trainer" exists we just need to read past
    // it before we get to the good stuff
    if (header.mapper1 & 0x04) {
        fseek(fp, 0, 512);
    }

    // Determine Mapper ID

    mapper_id = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
    mirror = (header.mapper1 & 0x01) ? mirror_t::VERTICAL : mirror_t::HORIZONTAL;

    switch (mapper_id) {
    case 0:
        mapper = new Mapper_000(PRG_banks, CHR_banks);
        break;

    default:
        LOG_E("Unexpected mapper in the cartridge %s", c_file);
        fclose(fp);
        return false;
    }


    // TODO(max): discover file format, there are 3
    uint8_t file_format = 1;

    switch (file_format) {
    case 1: {
        PRG_banks = header.prg_rom_chunks;
        PRG_memory.resize(PRG_banks * NES_PRG_BANK_SIZE);
        size_t read_res = fread((uint8_t *) PRG_memory.data(), sizeof(uint8_t), PRG_memory.size(), fp);

        if (read_res != PRG_memory.size()) {
            LOG_E("Failed read the cartridge program memory from file %s", c_file);
            fclose(fp);
            return false;
        }

        CHR_banks = header.chr_rom_chunks;
        CHR_memory.resize(CHR_banks * NES_CHR_BANK_SIZE);


        read_res = fread((uint8_t *) CHR_memory.data(), sizeof(uint8_t), CHR_memory.size(), fp);

        if (read_res != CHR_memory.size()) {
            LOG_E("Failed read the cartridge character memory from file %s", c_file);
            fclose(fp);
            return false;
        }
    }
    break;

    case 0:
    case 2:
    default:
        LOG_E("Unsupported file type");
        fclose(fp);
        return false;
    }

    fclose(fp);

    return true;
}


bool Cartridge::cpu_read(const uint16_t address, uint8_t &data) {

    uint32_t mapped_address = 0;

    // TODO(max): check if not nullptr
    if (mapper->cpu_map_read(address, mapped_address)) {
        data = PRG_memory[mapped_address];
        return true;
    }

    return false;
}


bool Cartridge::cpu_write(const uint16_t address, const uint8_t data) {

    uint32_t mapped_address = 0;

    if (mapper->cpu_map_write(address, mapped_address)) {
        PRG_memory[mapped_address] = data;
        return true;
    }

    return false;
}


bool Cartridge::ppu_read(const uint16_t address, uint8_t &data) {

    uint32_t mapped_address = 0;

    // TODO(max): check if not nullptr
    if (mapper->ppu_map_read(address, mapped_address)) {
        data = CHR_memory[mapped_address];
        return true;
    }

    return false;
}


bool Cartridge::ppu_write(const uint16_t address, const uint8_t data) {

    uint32_t mapped_address = 0;

    if (mapper->ppu_map_write(address, mapped_address)) {
        CHR_memory[mapped_address] = data;
        return true;
    }

    return false;
}
