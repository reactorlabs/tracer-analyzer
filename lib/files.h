#pragma once

#include <dirent.h>
#include <string>
#include <iostream>

/** Executes a method for each file in given path.
 */
class ForEachFile {
public:

    size_t findAll(std::string const & rootPath, bool recursive, bool verbose = true) {
        rootPath_ = rootPath;
        verbose_ = verbose;
        return analyzeDir("", recursive);
    }

protected:
    /** By default accept all files.
     */
    virtual bool filter(std::string const & path, std::string const & filename) {
        return true;
    }

    /** This will be called for each valid file.

        Must be overwritten in children. 
     */
    virtual void doStuff(std::string const & filename) = 0;

    /** Path at which the search for files starts.
     */
    std::string rootPath_;

    /** Whether extra stuff is displayed.
     */
    bool verbose_;

private:

    /** Analyzes the given directory and returns the number of valid files.
     */
    size_t analyzeDir(std::string const & dirPath, bool recursive) {
        size_t validFiles = 0;
        std::string folder = rootPath_ + dirPath;
        if (verbose_)
            std::cout << folder << std::endl;
        DIR * d = opendir(folder.c_str());
        struct dirent * dp;
        while (dp = readdir(d)) {
            std::string filename = dp->d_name;
            if (dp->d_type == DT_REG) {
                if (filter(dirPath, filename)) {
                    if (verbose_)
                        std::cout << "  " << filename << std::endl;
                    doStuff(dirPath + "/" + filename);
                    ++validFiles;
                }
            } else if (recursive && dp->d_type == DT_DIR) {
                // ignore these, obviously
                if (filename == "." || filename == "..")
                    continue;
                validFiles += analyzeDir(dirPath + "/" + filename, true);
            }
        }
        closedir(d);
        return validFiles;
    }
    
};
