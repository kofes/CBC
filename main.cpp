#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <list>

#define DEBUG

int set_input(
        const std::map<std::string, std::list<std::string>>& smap,
        std::ifstream& fin, const std::string& keyword
) {
    const std::list<std::string>& strvalues = smap.at(keyword);

    for (const std::string& value: strvalues) {
        fin.open(value);
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

    return !fin.is_open();
}

int set_output(
        const std::map<std::string, std::list<std::string>>& smap,
        std::ofstream& fout, const std::string& keyword
) {
    const std::list<std::string>& strvalues = smap.at(keyword);

    for (const std::string& value: strvalues) {
        fout.open(value);
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

    return !fout.is_open();
}

static const size_t ONE_MB = 1 << 20;

int set_bs(
        const std::map<std::string, std::list<std::string>>& smap,
        size_t& block_size, const std::string& keyword
) {
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
 * last option has higher priority (if it valueable)
 */

int main(int argc, char* argv[]) {
    ifstream fin;
    ofstream fout;
    size_t buffer_size;
    map<string, list<string>> smap;

    int err_code = serialize_args(argc, argv, smap) ||
        (smap.find("if") == smap.end() || set_input(smap, fin, "if")) ||
        (smap.find("of") == smap.end() || set_output(smap, fout, "of")) ||
        (smap.find("bs") == smap.end() || set_bs(smap, buffer_size, "bs"));
    if (err_code) return err_code;

    return 0;
}