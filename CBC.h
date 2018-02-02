#pragma once

#include <fstream>
#include <openssl/sha.h>

#define CBC_LOG

//#define CBC_DEBUG

namespace CBC {
    int sign(std::ifstream& fin, std::ofstream& fout, size_t block_size);

    int unsign(std::ifstream& fin, std::ofstream& fout, size_t block_size, size_t dead_bytes = 0);

    int check(std::ifstream& fin, size_t block_size);
};