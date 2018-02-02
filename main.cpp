#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <list>

//#define DEBUG

enum class ERROR_CODE: int {
    NORM = 0,

    NOT_SET_INPUT = 1 << 0,
    WRONG_INPUT = 1 << 1,

    NOT_SET_OUTPUT = 1 << 2,
    WRONG_OUTPUT = 1 << 3,
};

int serialize_args(int argc, char* argv[], std::map<std::string, std::list<std::string>>& smap) {
#ifdef DEBUG
    std::cout << "DEBUG: argc = " << argc << "; argv = [" << argv[0];
    for (size_t i = 1; i < argc; ++i)
        std::cout << ", " << argv[i];
    std::cout << ']' << std::endl;
#endif

    smap.clear();
    for (size_t i = 1; i < argc; ++i) {
        std::string argue = argv[i];

        size_t eqpos = argue.find('=');
        if (eqpos == std::string::npos) continue;

        std::string key = argue.substr(0, eqpos);
        std::string value = argue.substr(eqpos+1);

        smap[key].emplace_front(value);
#ifdef DEBUG
        std::cout << "DEBUG: " << "{key: " << key << ", value: " << value << "}" << std::endl;
#endif
    }

    return 0;
}

ERROR_CODE set_input(
        const std::map<std::string, std::list<std::string>>& smap,
        std::ifstream& fin, const std::string& keyword
) {
    bool keyword_exits = smap.find(keyword) != smap.end();
    if (!keyword_exits) return ERROR_CODE::NOT_SET_INPUT;

    const std::list<std::string>& strvalues = smap.at(keyword);

    for (const std::string& value: strvalues) {
        fin.open(value, std::ios::binary);
        if (fin.is_open()) {
#ifdef DEBUG
            std::cout << "DEBUG: input filename: " << value << std::endl;
#endif
            break;
        }
#ifdef DEBUG
        std::cout << "DEBUG: can't open file " << value << " to read" << std::endl;
#endif
    }

    return (!fin.is_open() ? ERROR_CODE::WRONG_INPUT : ERROR_CODE::NORM);
}

ERROR_CODE set_output(
        const std::map<std::string, std::list<std::string>>& smap,
        std::ofstream& fout, const std::string& keyword
) {
    bool keyword_exits = smap.find(keyword) != smap.end();
    if (!keyword_exits) return ERROR_CODE::NOT_SET_OUTPUT;

    const std::list<std::string>& strvalues = smap.at(keyword);

    for (const std::string& value: strvalues) {
        fout.open(value, std::ios::binary);
        if (fout.is_open()) {
#ifdef DEBUG
            std::cout << "DEBUG: output filename: " << value << std::endl;
#endif
            break;
        }
#ifdef DEBUG
        std::cout << "DEBUG: can't open file " << value << " to write" << std::endl;
#endif
    }

    return (!fout.is_open() ? ERROR_CODE::WRONG_OUTPUT : ERROR_CODE::NORM);
}

static const size_t ONE_MB = 1 << 20;

int set_bs(
        const std::map<std::string, std::list<std::string>>& smap,
        size_t& block_size, const std::string& keyword
) {
    block_size = ONE_MB;

    bool keyword_exits = smap.find(keyword) != smap.end();
    if (!keyword_exits) return 0;

    const std::list<std::string>& strvalues = smap.at(keyword);

    for (const std::string& value: strvalues) {
        std::stringstream buffer(value);
        buffer >> block_size;
        if (block_size) {
#ifdef DEBUG
            std::cout << "DEBUG: block size: " << block_size << std::endl;
#endif
            break;
        }
#ifdef DEBUG
        std::cout << "DEBUG: zero block size" << std::endl;
#endif
    }

    if (!block_size) block_size = ONE_MB;

    return 0;
}

enum class CBC_MODE: char {
    SIGN,
    UNSIGN,
    CHECK
};

int set_mode(
        const std::map<std::string, std::list<std::string>>& smap,
        CBC_MODE& mode, const std::string& keyword
) {
    mode = CBC_MODE::SIGN;

    bool keyword_exits = smap.find(keyword) != smap.end();
    if (!keyword_exits) return 0;

    const std::list<std::string>& strvalues = smap.at(keyword);

    for (const std::string& value: strvalues) {
        if (value == "sign") {
            mode = CBC_MODE::SIGN;
#ifdef DEBUG
            std::cout << "DEBUG: CBC MODE: " << value << std::endl;
#endif
            break;
        } else if (value == "unsign") {
            mode = CBC_MODE::UNSIGN;
#ifdef DEBUG
            std::cout << "DEBUG: CBC MODE: " << value << std::endl;
#endif
            break;
        } else if (value == "check") {
            mode = CBC_MODE::CHECK;
#ifdef DEBUG
            std::cout << "DEBUG: CBC MODE: " << value << std::endl;
#endif
            break;
        }
#ifdef DEBUG
        std::cout << "DEBUG: wrong mode: " << value << std::endl;
#endif
    }

    return 0;
}

int set_deadcount(
        const std::map<std::string, std::list<std::string>>& smap,
        size_t& dead_count, const std::string& keyword
) {
    dead_count = 0;

    bool keyword_exits = smap.find(keyword) != smap.end();
    if (!keyword_exits) return 0;

    const std::list<std::string>& strvalues = smap.at(keyword);

    for (const std::string& value: strvalues) {
        std::stringstream buffer(value);
        buffer >> dead_count;
        if (dead_count) {
#ifdef DEBUG
            std::cout << "DEBUG: block size: " << dead_count << std::endl;
#endif
            break;
        }
#ifdef DEBUG
        std::cout << "DEBUG: zero block size" << std::endl;
#endif
    }

    return 0;
}

#include "CBC.h"

using namespace std;

/**
 * options:
 * [must have]
 * if=%filename% - set input file
 * of=%filename% - set output file
 * [optional]
 * bs=%block size% - set block size //default value is 1MB
 * mode=%mode% - set mode for program //default value is sign
 * - sign - sign input file & write it to output file
 * - check - check input file for sign
 * - unsign - unsign input file & write it to output file
 * dc=%count dead bytes% - set count of not used bytes at end of signed file
 * last option[non zero] has higher priority (if it valueable)
 */

int main(int argc, char* argv[]) {
    ifstream fin;
    ofstream fout;
    size_t buffer_size;
    size_t dead_count;
    CBC_MODE mode;
    map<string, list<string>> smap;

    int err_code = serialize_args(argc, argv, smap) |
            set_mode(smap, mode, "mode") |
            (int)set_input(smap, fin, "if") |
            set_bs(smap, buffer_size, "bs");
    if (mode != CBC_MODE::CHECK)
        err_code |= (int)set_output(smap, fout, "of");
    if (mode == CBC_MODE::UNSIGN)
        err_code |= set_deadcount(smap, dead_count, "dc");

    if (err_code) {
        if (err_code & (int)ERROR_CODE::NOT_SET_OUTPUT)
            std::cout << "output file not set" << std::endl;
        if (err_code & (int)ERROR_CODE::NOT_SET_INPUT)
            std::cout << "input file not set" << std::endl;
        if (err_code & (int)ERROR_CODE::WRONG_INPUT)
            std::cout << "wrong input filename set" << std::endl;
        if (err_code & (int)ERROR_CODE::WRONG_OUTPUT)
            std::cout << "wrong output filename set" << std::endl;
        return err_code;
    }

    switch (mode) {
        case CBC_MODE::SIGN: return CBC::sign(fin, fout, buffer_size);
        case CBC_MODE::CHECK: return CBC::check(fin, buffer_size);
        case CBC_MODE::UNSIGN: return CBC::unsign(fin, fout, buffer_size, dead_count);
    }
}