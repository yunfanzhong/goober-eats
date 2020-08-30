#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
	ExpandableHashMap<GeoCoord, vector<StreetSegment>>* streetMap;
};

StreetMapImpl::StreetMapImpl() {
	streetMap = new ExpandableHashMap<GeoCoord, vector<StreetSegment>>;
}

StreetMapImpl::~StreetMapImpl() {
	
	delete streetMap; // calls expandable hash map destructor, which frees memory
}

bool StreetMapImpl::load(string mapFile) {
	ifstream mapdata(mapFile);
	if (!mapdata) {
		std::cerr << "Unable to open " << mapFile << endl;
		return false;
	}

	string startLat, startLong, endLat, endLong, streetname;
	while (getline(mapdata, streetname)) {
		int numSegs;
		mapdata >> numSegs;
		mapdata.ignore(10000, '\n');
		for (int i = 0; i < numSegs; i++) {
			// get all the latitudes and longitudes from a line
			mapdata >> startLat;
			mapdata.ignore(10000, ' ');
			mapdata >> startLong;
			mapdata.ignore(10000, ' ');
			mapdata >> endLat;
			mapdata.ignore(10000, ' ');
			mapdata >> endLong;
			mapdata.ignore(10000, '\n');
			GeoCoord startCoord(startLat, startLong);
			GeoCoord endCoord(endLat, endLong);
			StreetSegment street(startCoord, endCoord, streetname); // create normal street
			StreetSegment reverseStreet(endCoord, startCoord, streetname); // create reverse of street
			
			// add to reversed street vector
			vector<StreetSegment>* endCoordVec = (*streetMap).find(endCoord);
			if (endCoordVec == nullptr) {
				endCoordVec = new vector<StreetSegment>;
			}
			(*endCoordVec).push_back(reverseStreet);
			(*streetMap).associate(endCoord, *endCoordVec);

			// add to normal street vector
			vector<StreetSegment>* startCoordVec = (*streetMap).find(startCoord);
			if (startCoordVec == nullptr) {
				startCoordVec = new vector<StreetSegment>;
			}
			(*startCoordVec).push_back(street);
			(*streetMap).associate(startCoord, *startCoordVec);
		}
	}
	mapdata.close();
	return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const {
	segs.clear();
	// find the vector containing segments starting with GeoCoord gc
	const vector<StreetSegment>* vec = const_cast<vector<StreetSegment>*>((*streetMap).find(gc));
	if (vec != nullptr) {
		auto it = vec->begin();
		while (it != vec->end()) {
			segs.push_back(*it); // push all these segments into segs vector
			it++;
		}
		return true;
	}
	return false;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
