#include "CBC.h"

#ifdef CBC_LOG

#include <iostream>
#define CBCout (std::cout << "CBC LOG: ")

#endif

#ifdef CBC_DEBUG

#include <iostream>
#define CBCdbg (std::cout << "CBC DBG: ")

#endif

int CBC::sign(std::ifstream& fin, std::ofstream& fout, size_t block_size) {
    SHA512_CTX sha512_ctx;
    SHA512_Init(&sha512_ctx);

    size_t hslen = SHA512_DIGEST_LENGTH;
    unsigned char hash_buffer[block_size + hslen] = {0};
    unsigned char hash_sum[hslen] = {0};
    unsigned char buffer[block_size] = {0};
    size_t read_size;

#ifdef CBC_DEBUG
    size_t iteration_counter = 0;
#endif

    do {

#ifdef CBC_DEBUG
        ++iteration_counter;
        CBCdbg << "\n\titeration: " << iteration_counter << ";" << std::endl;
#endif

        fin.read(reinterpret_cast<char *>(buffer), block_size);

        size_t tmp_read_size = fin.gcount();
        if (tmp_read_size == 0) break;
        read_size = tmp_read_size;

        for (size_t i = read_size; i < block_size; ++i)
            buffer[i] = 0;

        std::copy(buffer, buffer+block_size, hash_buffer);
        std::copy(hash_sum, hash_sum+hslen, hash_buffer+block_size);

        SHA512_Update(&sha512_ctx, hash_buffer, block_size + hslen);
        SHA512_Final(hash_sum, &sha512_ctx);

        fout.write(reinterpret_cast<char *>(buffer), block_size);
        fout.write(reinterpret_cast<char *>(hash_sum), hslen);
    } while (!fin.eof());

#ifdef CBC_LOG
    CBCout << "last block has " << block_size - read_size
           << " dead bytes at the end" << std::endl;
#endif

    return 0;
}

int CBC::unsign(std::ifstream& fin, std::ofstream& fout, size_t block_size, size_t dead_bytes) {
    SHA512_CTX sha512_ctx;
    SHA512_Init(&sha512_ctx);

    size_t hslen = SHA512_DIGEST_LENGTH;
    unsigned char hash_buffer[block_size + hslen] = {0};
    unsigned char hash_sum[hslen] = {0};
    unsigned char buffer[block_size+hslen] = {0};
    size_t read_size;

#ifdef CBC_DEBUG
    size_t iteration_counter = 0;
#endif

#ifdef CBC_DEBUG
    ++iteration_counter;
    CBCdbg << "\n\titeration: " << iteration_counter << ";" << std::endl;
#endif

    fin.read(reinterpret_cast<char *>(buffer), block_size+hslen);
    read_size = fin.gcount();

    if (read_size == 0) return 2;

    if (block_size + hslen - read_size) {
#ifdef CBC_LOG
        CBCout << "ERROR: file not aligned" << std::endl;
#endif
#ifdef CBC_DEBUG
        CBCdbg << "\n\tread size: " << read_size << ";\n"
               << "\tbuffer: " << block_size + hslen << ";"
               << std::endl;
#endif
        return 1;
    }

    std::copy(buffer, buffer+block_size, hash_buffer);
    std::copy(hash_sum, hash_sum+hslen, hash_buffer+block_size);

    SHA512_Update(&sha512_ctx, hash_buffer, block_size + hslen);
    SHA512_Final(hash_sum, &sha512_ctx);

    for (size_t i = 0; i < hslen; ++i)
        if (buffer[block_size + i] != hash_sum[i]) {
#ifdef CBC_LOG
            CBCout << "ERROR: file has corrupted block" << std::endl;
#endif
#ifdef CBC_DEBUG
            CBCdbg << "\n\ti:" << i << ";\n"
                   << "\tbuffer char: " << (int)buffer[block_size + i] << ";\n"
                   << "\thash char:" << (int)hash_sum[i] << ";"
                   << std::endl;
#endif
            return 2;
        }
    fout.write(reinterpret_cast<char *>(hash_buffer), block_size - dead_bytes);


    do {

#ifdef CBC_DEBUG
        ++iteration_counter;
        CBCdbg << "\n\titeration: " << iteration_counter << ";" << std::endl;
#endif

        fin.read(reinterpret_cast<char *>(buffer), block_size+hslen);
        read_size = fin.gcount();

        if (read_size == 0) break;
        else
            fout.write((char *)(hash_buffer + (block_size - dead_bytes)), dead_bytes);

        if (block_size + hslen - read_size) {
#ifdef CBC_LOG
            CBCout << "ERROR: file not aligned" << std::endl;
#endif
#ifdef CBC_DEBUG
            CBCdbg << "\n\tread size: " << read_size << ";\n"
                      << "\tbuffer: " << block_size + hslen << ";"
                      << std::endl;
#endif
            return 1;
        }

        std::copy(buffer, buffer+block_size, hash_buffer);
        std::copy(hash_sum, hash_sum+hslen, hash_buffer+block_size);

        SHA512_Update(&sha512_ctx, hash_buffer, block_size + hslen);
        SHA512_Final(hash_sum, &sha512_ctx);

        for (size_t i = 0; i < hslen; ++i)
            if (buffer[block_size + i] != hash_sum[i]) {
#ifdef CBC_LOG
                CBCout << "ERROR: file has corrupted block" << std::endl;
#endif
#ifdef CBC_DEBUG
                CBCdbg << "\n\ti:" << i << ";\n"
                       << "\tbuffer char: " << (int)buffer[block_size + i] << ";\n"
                       << "\thash char:" << (int)hash_sum[i] << ";"
                       << std::endl;
#endif
                return 2;
            }
        fout.write(reinterpret_cast<char *>(hash_buffer), block_size - dead_bytes);
    } while (!fin.eof());

    return 0;
}

int CBC::check(std::ifstream& fin, size_t block_size) {
    SHA512_CTX sha512_ctx;
    SHA512_Init(&sha512_ctx);

    size_t hslen = SHA512_DIGEST_LENGTH;
    unsigned char hash_buffer[block_size + hslen] = {0};
    unsigned char hash_sum[hslen] = {0};
    unsigned char buffer[block_size+hslen] = {0};
    size_t read_size;

#ifdef CBC_DEBUG
    size_t iteration_counter = 0;
#endif

    do {

#ifdef CBC_DEBUG
        ++iteration_counter;
        CBCdbg << "\n\titeration: " << iteration_counter << ";" << std::endl;
#endif

        fin.read(reinterpret_cast<char *>(buffer), block_size+hslen);
        read_size = fin.gcount();
        if (!read_size) break;

        if (block_size + hslen - read_size) {
#ifdef CBC_LOG
            CBCout << "ERROR: file not aligned" << std::endl;
#endif
#ifdef CBC_DEBUG
            CBCdbg << "\n\tread size: " << read_size << ";\n"
                   << "\tbuffer: " << block_size + hslen << ";"
                   << std::endl;
#endif
            return 1;
        }

        std::copy(buffer, buffer+block_size, hash_buffer);
        std::copy(hash_sum, hash_sum+hslen, hash_buffer+block_size);

        SHA512_Update(&sha512_ctx, hash_buffer, block_size + hslen);
        SHA512_Final(hash_sum, &sha512_ctx);

        for (size_t i = 0; i < hslen; ++i)
            if (buffer[block_size + i] != hash_sum[i]) {
#ifdef CBC_LOG
                CBCout << "ERROR: file has corrupted block" << std::endl;
#endif
#ifdef CBC_DEBUG
                CBCdbg << "\ti:" << i << ";\n"
                       << "\tbuffer char: " << (int)buffer[block_size + i] << ";\n"
                       << "\thash char: " << (int)hash_sum[i] << ";"
                       << std::endl;
#endif
                return 2;
            }
    } while (!fin.eof());

    return 0;
}