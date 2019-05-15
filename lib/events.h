#pragma once

#include <cassert>
#include <vector>
#include <string>
#include <unordered_map>
#include <ostream>
#include <sstream>

#include "helpers/strings.h"

inline std::string stripStringPrefixes(std::string const & str) {
    if (str[str.size() - 1] != '"')
        return str;
    std::string result = str.substr(str.find("\"") + 1);
    result = result.substr(0, result.size() -1);
    return result;
}


class Location {
public:
    std::string filename;
    int line;

    Location() = default;

    Location(std::string const & filename, int line):
        filename(filename),
        line(line) {
    }

    /** Obtains location from the filename:line notation.
     */
    Location(std::string const & loc) {
        auto x = helpers::split(loc, ':');
        assert(x.size() == 2);
        filename = stripStringPrefixes(x[0]);
        line = std::stoi(x[1]);

    }

};

class Event {
public:
    virtual ~Event() {}
};

class AllocationEvent : public Event {
public:
    int id;
    int size;

    AllocationEvent(std::vector<std::string> const & row) {
        assert(row[0] == "allocation-event");
        if (row.size() != 3)
            throw "Invalid format of allocation-event record";
        id = std::stoi(row[1]);
        size = std::stoi(row[2]);
    }
};

class GetProperty : public Event {
public:
    size_t id;
    size_t mapId;
    std::string name;
    Location location;

    GetProperty(std::vector<std::string> const & row) {
        assert(row[0] == "get-property");
        if (row.size() != 5)
            throw "Invalid format of get-property record";
        id = std::stoi(row[1]);
        mapId = std::stoi(row[2]);
        name = row[3];
        location = Location(row[4]);
    }
};

class SetProperty : public Event {
public:
    size_t id;
    size_t mapId;
    std::string name;
    std::string value;
    Location location;

    SetProperty(std::vector<std::string> const & row) {
        assert(row[0] == "set-property");
        if (row.size() != 6)
            throw "Invalid format of set-property record";
        id = std::stoi(row[1]);
        mapId = std::stoi(row[2]);
        name = row[3];
        value = row[4];
        location = Location(row[5]);
    }
};

class GetElement : public Event {
public:
    size_t id;
    size_t mapId;
    int index;
    Location location;

    GetElement(std::vector<std::string> const & row) {
        assert(row[0] == "get-element");
        if (row.size() != 5)
            throw "Invalid format of get-element record";
        id = std::stoi(row[1]);
        mapId = std::stoi(row[2]);
        index = std::stoi(row[3]);
        location = Location(row[4]);
    }

    friend std::ostream & operator << (std::ostream & s, GetElement const & e) {
        s << "get-element," << e.id << "," << e.mapId << "," << e.index << "," << e.location.line << "," << e.location.filename;
        return s;
    }

};

class SetElement : public Event {
public:
    size_t id;
    size_t mapId;
    int index;
    std::string value;
    Location location;

    SetElement(std::vector<std::string> const & row) {
        assert(row[0] == "set-element");
        if (row.size() != 6)
            throw "Invalid format of set-element record";
        id = std::stoi(row[1]);
        mapId = std::stoi(row[2]);
        index = std::stoi(row[3]);
        value = row[4];
        location = Location(row[5]);
    }

    friend std::ostream & operator << (std::ostream & s, SetElement const & e) {
        s << "set-element," << e.id << "," << e.mapId << "," << e.index << "," << e.location.line << "," << e.location.filename;
        return s;
    }
};

class MapChange : public Event {
public:
    size_t id;
    std::string type;
    size_t prototype;
    int elementType;
    std::unordered_map<std::string, char> fields;

    MapChange(std::vector<std::string> const & row) {
        assert(row[0] == "map-change");
        if (row.size() < 6) {
          throw "Invalid format of map-change record";
        }
        id = std::stoi(row[1]);
        prototype = std::stoi(row[2]);
        type = row[3];
        elementType = std::stoi(row[4]);
        int numFields = std::stoi(row[5]);
        if (row.size() != 2 * numFields + 6)
            throw "Invalid format of map-change record";
        for (size_t i = 6, e = row.size(); i < e; i++) {
          auto name = row[i++];
          if (row[i].size() != 1) {
            std::ostringstream buffer;
            buffer << "Mnemonic must be one char; got " << row[i];
            throw std::runtime_error(buffer.str());
          }
          fields[name] = row[i][0];
        }
    }

    friend std::ostream & operator << (std::ostream & s, MapChange const & m) {
        // describe the map in some useful way (???)
        // TODO
        return s;
    }

};

