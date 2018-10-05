#pragma once

#include <cassert>
#include <vector>
#include <string>



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
        assert(row.size() == 3);
        id = std::stoi(row[1]);
        size = std::stoi(row[2]);
    }
};

class GetProperty : public Event {
public:
    int id;
    int mapId;
    std::string name;
    Location location;

    GetProperty(std::vector<std::string> const & row) {
        assert(row[0] == "get-property");
        assert(row.size() == 5);
        id = std::stoi(row[1]);
        mapId = std::stoi(row[2]);
        name = row[3];
        location = Location(row[4]);
    }
};

class SetProperty : public Event {
public:
    int id;
    int mapId;
    std::string name;
    Location location;

    SetProperty(std::vector<std::string> const & row) {
        assert(row[0] == "set-property");
        assert(row.size() == 5);
        id = std::stoi(row[1]);
        mapId = std::stoi(row[2]);
        name = row[3];
        location = Location(row[4]);
    }
};

class GetElement : public Event {
public:
    int id;
    int mapId;
    int index;
    Location location;

    GetElement(std::vector<std::string> const & row) {
        assert(row[0] == "get-element");
        assert(row.size() == 5);
        id = std::stoi(row[1]);
        mapId = std::stoi(row[2]);
        index = std::stoi(row[3]);
        location = Location(row[4]);
    }
};

class SetElement : public Event {
public:
    int id;
    int mapId;
    int index;
    Location location;

    SetElement(std::vector<std::string> const & row) {
        assert(row[0] == "set-element");
        assert(row.size() == 5);
        id = std::stoi(row[1]);
        mapId = std::stoi(row[2]);
        index = std::stoi(row[3]);
        location = Location(row[4]);
    }
};

class MapChange : public Event {
public:
    int id;
    std::string type;
    int elementType;
    std::unordered_map<std::string, char> fields;

    MapChange(std::vector<std::string> const & row) {
        assert(row[0] == "map-change");
        id = std::stoi(row[1]);
        type = row[2];
        elementType = std::stoi(row[3]);
        int numFields = std::stoi(row[4]);
        assert(row.size() = numFields + 5);
        for (size_t i = 5, e = row.size(); i != e; ++i) {
            auto x = 

            
        }
    }
    
    

    
};

    
