#include <iostream>
#include <fstream>
#include <exception>
#include <unordered_map>

#include "lib/files.h"
#include "lib/events.h"
#include "lib/jsobject.h"
#include "helpers/csv-reader.h"
#include "helpers/strings.h"

using namespace helpers;


std::string inputDir; // where to look for the logs


std::ofstream logFile; // where we put logs of the entire run


void ParseArguments(int argc, char * argv[]) {
  if (argc < 2)
    throw "Invalid number of arguments, usage: hmarrays PATH";
  inputDir = argv[1];
  std::cout << "Input dir:  " << inputDir << std::endl;
}


/** Analyzes the given file.

*/
class Analyzer : public CSVReader<> {
  public:

    size_t mapChanges_ = 0;
    size_t allocations_ = 0;
    size_t getElements_ = 0;
    size_t setElements_ = 0;
    size_t getProperties_ = 0;
    size_t setProperties_ = 0;
    size_t reportedSetElements_ = 0;
    size_t reportedGetElements_ = 0;

    size_t parse(std::string const & filename) {
      reported_ = 0;
      rows_ = 0;
      mapChanges_ = 0;
      accs_.open(filename + ".accesses");
      objs_.open(filename + ".objects");
      CSVReader::parse(filename);

      size_t numObjs = objects_.size();

      report();

      logFile << rows_ << "," << mapChanges_ << "," << allocations_ << "," << getElements_ << "," << setElements_ <<  "," << getProperties_ << "," << setProperties_ << "," << reportedSetElements_ << "," << reportedGetElements_ << "," << numObjs << "," << reported_ <<  "," << filename << std::endl;

      return rows_;
    }

    void report() {
      clearObjects();
      clearMaps();
      std::cout << "    Reported: " << reported_ << std::endl;
    }

    ~Analyzer() {
      clearObjects(false);
      clearMaps();
    }
  protected:

    /** Accesses log.
    */
    std::ofstream accs_;

    /** Objects log.
    */
    std::ofstream objs_;

    int reported_;

    MapChange * getMap(size_t id) {
      auto i = maps_.find(id);
      assert(i != maps_.end() && "Map not found should never happen");
      return i->second;
    }


    JSObject & getObject(size_t id) {
      auto i = objects_.find(id);
      if (i != objects_.end())
        return * i->second;
      JSObject * result = new JSObject(id);
      objects_[id] = result;
      return * result;
    }

    void row(std::vector<std::string> & row) override {
      try {
        ++rows_;
        if (row[0] == "allocation-event") {
          AllocationEvent e(row);
          auto i = idConv_.find(e.id);
          if (i != idConv_.end())
            idConv_.erase(i);
          ++allocations_;
        } else if (row[0] == "get-property") {
          GetProperty e(row);
          e.id = convertId(e.id);
          e.mapId = convertId(e.mapId);
          JSObject & o = getObject(e.id);
          o.propertyRead(getMap(e.mapId), e);
          ++getProperties_;
        } else if (row[0] == "set-property") {
          SetProperty e(row);
          e.id = convertId(e.id);
          e.mapId = convertId(e.mapId);
          JSObject & o = getObject(e.id);
          o.propertyWrite(getMap(e.mapId), e);
          ++setProperties_;
        } else if (row[0] == "get-element") {
          GetElement e(row);
          e.id = convertId(e.id);
          e.mapId = convertId(e.mapId);
          JSObject & o = getObject(e.id);
          o.elementRead(getMap(e.mapId), e);
          if (o.isObject()) {
            o.shouldReport = true;
            ++reportedGetElements_;
            accs_ << rows_ << "," << e << '\n';
          }
          ++getElements_;
        } else if (row[0] == "set-element") {
          SetElement e(row);
          e.id = convertId(e.id);
          e.mapId = convertId(e.mapId);
          JSObject & o = getObject(e.id);
          o.elementWrite(getMap(e.mapId), e);
          if (o.isObject()) {
            o.shouldReport = true;
            ++reportedSetElements_;
            accs_ << rows_ << "," << e << '\n';
          }
          ++setElements_;
        } else if (row[0] == "map-change") {
          ++mapChanges_;
          MapChange x(row);
          x.id = convertId(x.id);
          auto i = maps_.find(x.id);
          if (i == maps_.end()) {
            maps_[x.id] = new MapChange(std::move(x));
          }
        } else {
          std::cerr << "Invalid row name: " << row[0] << " at row " << rows_ << std::endl;
        }
      } catch (char const * e) {
        std::cout << e << std::endl;
        for (auto & s : row)
          std::cout << s << ",";
        std::cout << std::endl;
      }
    }


    void clearObjects(bool report = true) {
      for (auto i : objects_) {
        if (report && i.second->shouldReport) {
          objs_ << (*i.second) << '\n';
          ++reported_;
        }
        delete i.second;
      }
      if (report)
        std::cout << "Total objects: " << objects_.size() << std::endl;
      objects_.clear();
    }

    void clearMaps() {
      for (auto i : maps_) {
        delete i.second;
      }
      maps_.clear();

    }

    size_t convertId(size_t id) {
      auto i = idConv_.find(id);
      if (i != idConv_.end())
        return i->second;
      size_t result = maxId_++;
      idConv_[id] = result;
      return result;
    }

  private:

    std::unordered_map<size_t, MapChange *> maps_;
    std::unordered_map<size_t, JSObject *> objects_;

    size_t rows_;

    size_t maxId_ = 0;
    std::unordered_map<size_t, size_t> idConv_;
};


class FileSeeker : public ForEachFile {
  protected:

    bool filter(std::string const & path, std::string const & filename) override {
      return endsWith(filename, ".log") && startsWith(filename, "isolate-");
    }

    void doStuff(std::string const & filename) override {
      try {
        Analyzer a;
        std::cout << "    rows: " << a.parse(rootPath_ + "/" + filename) << std::endl;
      } catch (std::exception const & e) {
        std::cout << "Error in file " << filename << std::endl;
        std::cout << e.what() << std::endl;
      }
    }
};


/** HMArrays

  For each file, build a representation of its objects and print any suspicious activity.

  This means that we load the events and we are only interested in:

  - property accesses of arrays
  - element accesses of non-arrays

  Other stuff should be summarized only
  */
int main(int argc, char * argv[]) {
  try {
    ParseArguments(argc, argv);
    logFile.open("log.csv");
    FileSeeker fs;
    size_t validFiles = fs.findAll(inputDir, true, true);
    std::cout << "Total analyzed logs: " << validFiles << std::endl;
  } catch (std::exception const &e) {
    std::cerr << "Exception caught: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (std::string const &s) {
    std::cerr << "String caught: " << s << std::endl;
    return EXIT_FAILURE;
  } catch (char const *s) {
    std::cerr << "C String caught: " << s << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
