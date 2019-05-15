#pragma once

#include <climits>
#include <ostream>

#include <string>
#include <vector>
#include <unordered_map>



/** Represents the JavaScript object over its lifetime.



 */
class JSObject {
public:

    JSObject(size_t id):
        id(id) {
    }

    bool shouldReport = false;

    /** ID of the object.
     */
    size_t id;

    /** For each property we keep how many times it was read and written to.
     */
    class PropertyRecord {
    public:
        size_t getCount;
        size_t setCount;
    };

    /** An epoch of an object is characterized by constant map.

        It contains the map, total number of events
    */
    class Epoch {
    public:
        MapChange * map;
        size_t events;
        int minIndex;
        int maxIndex;
        size_t getElementCount;
        size_t setElementCount;
        std::unordered_map<std::string, PropertyRecord> properties;
        size_t getPropertyCount;
        size_t setPropertyCount;

        Epoch(MapChange * map):
            map(map),
            events(0),
            minIndex(INT_MAX),
            maxIndex(INT_MIN),
            getElementCount(0),
            setElementCount(0),
            getPropertyCount(0),
            setPropertyCount(0) {
        }

        friend std::ostream & operator << (std::ostream & s, Epoch const & e) {
            s << e.map->id << "," << e.map->type << "," << e.events << "," << e.getElementCount << "," << e.setElementCount << "," << e.minIndex << "," << e.maxIndex << "," << e.getPropertyCount << "," << e.setPropertyCount << "," << e.properties.size();

            return s;
        }


    };

    bool isArray() {
        std::string const & t = epochs_.back().map->type;
        return (t == "JS_ARRAY_TYPE") || (t == "JS_TYPED_ARRAY_TYPE");
    }

    bool isObject() {
        std::string const & t = epochs_.back().map->type;
        return (t == "JS_OBJECT_TYPE");
    }

    void propertyRead(MapChange * map, GetProperty const & e) {
        Epoch & epoch = updateEpoch(map);
        ++epoch.events;
        ++epoch.getPropertyCount;
        epoch.properties[e.name].getCount += 1;


    }

    void propertyWrite(MapChange * map, SetProperty const & e) {
        Epoch & epoch = updateEpoch(map);
        ++epoch.events;
        ++epoch.setPropertyCount;
        epoch.properties[e.name].setCount += 1;
    }

    /** Updates the counters in the epoch.
     */
    void elementRead(MapChange * map, GetElement const & e) {
        Epoch & epoch = updateEpoch(map);
        ++epoch.events;
        ++epoch.getElementCount;
        if (e.index < epoch.minIndex)
            epoch.minIndex = e.index;
        else if (e.index > epoch.maxIndex)
            epoch.maxIndex = e.index;
    }

    /** Updates the counters in the epoch.
     */
    void elementWrite(MapChange * map, SetElement const & e) {
        Epoch & epoch = updateEpoch(map);
        ++epoch.events;
        ++epoch.setElementCount;
        if (e.index < epoch.minIndex)
            epoch.minIndex = e.index;
        else if (e.index > epoch.maxIndex)
            epoch.maxIndex = e.index;
    }

    friend std::ostream & operator << (std::ostream & s, JSObject const & obj) {
        size_t eidx = 0;
        for (Epoch const & e : obj.epochs_) {
            s << obj.id << "," << eidx << "," << e << '\n';
            ++eidx;
        }
        return s;
    }

private:

    Epoch & updateEpoch(MapChange * map) {
        if (epochs_.empty())
            epochs_.push_back(Epoch(map));
        else if (epochs_.back().map->id != map->id)
            epochs_.push_back(Epoch(map));
        // otherwise do nothing, the epoch we have is valid
        return epochs_.back();
    }

    std::vector<Epoch> epochs_;

};
