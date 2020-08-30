#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <queue>
#include <map>
#include <set>
#include <stack>
using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
	StreetMap* streetMap;
	typedef pair<double, GeoCoord> Pair;
	struct cmpStruct {
		bool operator()(const pair<double, GeoCoord>& lhs, const pair<double, GeoCoord>& rhs) const {
			return lhs.first < rhs.first;
		}
	};
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm) {
	streetMap = const_cast<StreetMap*>(sm);
}

PointToPointRouterImpl::~PointToPointRouterImpl() {

}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const {
	map<GeoCoord, pair<GeoCoord, string>> moveMap;
	ExpandableHashMap<GeoCoord, double> fMap;
	ExpandableHashMap<GeoCoord, double> gMap;
	set <pair<double, GeoCoord>, cmpStruct> openList;
	set <GeoCoord> closedList;
	vector<StreetSegment> segs;
	// if the start or end doesn't exist in the map, return
	if (!(*streetMap).getSegmentsThatStartWith(end, segs) || !(*streetMap).getSegmentsThatStartWith(start, segs)) {
		return BAD_COORD;
	}
	openList.insert(make_pair(0.0, start)); // insert first coord into open list
	// record the first coord's values
	fMap.associate(start, 0);
	gMap.associate(start, 0);

	while (!openList.empty()) {
		auto it = openList.begin();
		GeoCoord q = (*it).second; // get the first item in the open list (smallest f value)
		openList.erase(it);
		closedList.insert(q);

		double gNew, hNew, fNew;
		(*streetMap).getSegmentsThatStartWith(q, segs);
		for (unsigned int i = 0; i < segs.size(); ++i) { // go through all children of q
			GeoCoord succ = segs[i].end;
			string streetName = segs[i].name;
			if (succ.latitude == end.latitude && succ.longitude == end.longitude) { // arrived at destination
				moveMap.insert(make_pair(succ, make_pair(segs[i].start, segs[i].name)));
				route.clear();
				GeoCoord currCoord = end;
				// retrace path into route
				while (!(currCoord.latitude == start.latitude && currCoord.longitude == start.longitude)) {
					GeoCoord prev = moveMap.at(currCoord).first;
					string name = moveMap.at(currCoord).second;
					totalDistanceTravelled += distanceEarthMiles(prev, currCoord);
					StreetSegment s(prev, currCoord, name);
					route.push_front(s);
					currCoord = prev;
				}
				return DELIVERY_SUCCESS;
			}

			if (closedList.find(succ) == closedList.end()) { // check if in closed list
				// calculate values
				double* fOld = fMap.find(succ);

				double* oldG = gMap.find(q);

				if (oldG != nullptr) {
					gNew = *oldG + distanceEarthMiles(q, succ);
				}
				else {
					gNew = distanceEarthMiles(q, succ);
				}
				hNew = distanceEarthMiles(end, succ);;
				fNew = gNew + hNew;

				bool skip = false;
				auto it = openList.begin(); // check if in open list
				while (it != openList.end()) {
					if (succ == (*it).second && fOld != nullptr && *fOld < fNew) {
						skip = true;
						break;
					}
					it++;
				}

				if (!skip) { // if the new coord is better/doesn't yet exist, insert it and its assocations into the list
					openList.insert(make_pair(fNew, succ));
					moveMap.insert(make_pair(succ, make_pair(segs[i].start, segs[i].name)));
					fMap.associate(succ, fNew);
					gMap.associate(succ, gNew);
				}
			}
		}
	}
	return NO_ROUTE;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
