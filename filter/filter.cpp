
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_set>
#include <dirent.h>

#include "helpers/strings.h"

using namespace helpers;

std::string inputDir; // where to look for the logs
std::string outputDir; // where to put the filtered logs
std::unordered_set<std::string> events; // where to 



void ParseArguments(int argc, char * argv[]) {
    if (argc < 2)
        throw "Invalid number of arguments, usage: filter OUT {EVENT} [-o OUTPUT_DIR]";
    inputDir = argv[1];
    outputDir = argv[1];
    for (int i = 2; i < argc; ++i) {
        if (strncmp(argv[i], "-o", 3) == 0) {
            ++i;
            if (i == argc)
                throw "-o must be followed by output folder";
            outputDir = argv[i];
        } else {
            events.insert(argv[i]);
        }
    }
    std::cout << "Input dir:  " << inputDir << std::endl;
    std::cout << "Output dir: " << outputDir << std::endl;
    std::cout << "Events:    ";
    for (std::string const & e : events)
        std::cout << " " << e;
    std::cout << std::endl;
}

void FilterLog(std::string const & folder, std::string const & name) {
    long totalEvents = 0;
    long validEvents = 0;
    std::string path = inputDir + folder + "/" + name;
    // first make sure we can store the filtered log
    std::string outputPath = outputDir + folder + "/" +  name;
    if (path == outputPath) {
        outputPath += ".filtered";
    } else {
        std::string cmd = "mkdir -p ";
        cmd += outputDir + folder;
        system(cmd.c_str());
    }
    std::cout << "Input:  " << path << std::endl;
    std::cout << "Output: " << outputPath << std::endl;
    // do the filter
    std::ifstream f(path);
    std::ofstream of(outputPath);
    if (! f.good())
        std::cout << "ERR";
    std::string line;
    while (std::getline(f, line)) {
        ++totalEvents;
        for (std::string const & event : events) {
            if (startsWith(line, event)) {
                ++validEvents;
                of << line << '\n';
                break;
            }
        }
    }
    f.close();
    of.close();
    std::cout << "  total events: " << totalEvents << ", filtered: " << validEvents;
    if (totalEvents != 0)
        std::cout << ", %: " << (validEvents * 100 / totalEvents);
    std::cout << std::endl;
}

void FilterLogFiles(std::string const & folder = "") {
    std::string path = folder == "" ? inputDir : inputDir + folder;
    std::cout << path << std::endl;
    DIR * d = opendir(path.c_str());
    struct dirent * dp;
    while (dp = readdir(d)) {
        std::string name = dp->d_name;
        // if we are dealing with regular file, see if it is log and filter if so
        if (dp->d_type == DT_REG) {
            if (endsWith(name, ".log") && startsWith(name, "isolate-"))
                FilterLog(folder, name);
        // otherwise recurse into directories 
        } else if (dp->d_type == DT_DIR) {
            if (name == "." || name == "..") // ignore special dirs
                continue;
            FilterLogFiles(folder + "/" + name);
        }
    }
    closedir(d);
}


/** Usage:

    filter RAWDIR {E} [-o OUTPUT ]

    Where `RAWDIR` is the directory in which the logs should be found and `E` are names of events we want to keep. All other events will be discarded. The `-o` argument followed by a directory puts all the compacted logs into this directory.

    If output is not specified, the filtered logs are stored in the same directory they were taken from and have `.filtered` appended to their name. 
 */
int main(int argc, char * argv[]) {
    try {
        ParseArguments(argc, argv);
        FilterLogFiles();
    } catch (char const * e) {
        std::cerr << "ERROR: ";
        std::cerr << e << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
