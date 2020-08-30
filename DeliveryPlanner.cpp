#include "provided.h"
#include "ExpandableHashMap.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
	const StreetMap* streetMap;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
	streetMap = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl() {

}

void getDirectionToTravel(double direction, string& dirToTravel) {
	if (direction >= 0 && direction < 22.5) { // 0 <= angle < 22.5: east
		dirToTravel = "east";
	}
	else if (direction >= 22.5 && direction < 67.5) { // 22.5 <= angle < 67.5 : northeast
		dirToTravel = "northeast";
	}
	else if (direction >= 67.5 && direction < 112.5) { // 67.5 <= angle < 112.5 : north
		dirToTravel = "north";
	}
	else if (direction >= 112.5 && direction < 157.5) { // 112.5 <= angle < 157.5 : northwest
		dirToTravel = "northwest";
	}
	else if (direction >= 157.5 && direction < 202.5) { // 157.5 <= angle < 202.5 : west
		dirToTravel = "west";
	}
	else if (direction >= 202.5 && direction < 247.5) { // 202.5 <= angle < 247.5 : southwest
		dirToTravel = "southwest";
	}
	else if (direction >= 247.5 && direction < 292.5) { // 247.5 <= angle < 292.5 : south
		dirToTravel = "south";
	}
	else if (direction >= 292.5 && direction < 337.5) { // 292.5 <= angle < 337.5 : southeast
		dirToTravel = "southeast";
	}
	else { // angle >= 337.5 : east
		dirToTravel = "east";
	}
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
	totalDistanceTravelled = 0;
	DeliveryOptimizer delOp(streetMap);
	PointToPointRouter ptp(streetMap);
	vector<DeliveryRequest> optimizedDeliveries = deliveries;
	list<StreetSegment> route;
	double oldDist, newDist;
	delOp.optimizeDeliveryOrder(depot, optimizedDeliveries, oldDist, newDist);

	unsigned int numDeliveries = optimizedDeliveries.size();

	for (unsigned int i = 0; i <= numDeliveries; i++) {
		unsigned int index = i;
		DeliveryResult dr;
		if (i == 0) { // start from depot for first item
			dr = ptp.generatePointToPointRoute(depot, optimizedDeliveries[index].location, route, totalDistanceTravelled);
			if (dr != DELIVERY_SUCCESS) {
				return dr;
			}
		}
		else if (i < numDeliveries) { // start from prev delivery location for next items
			dr = ptp.generatePointToPointRoute(optimizedDeliveries[index - 1].location, optimizedDeliveries[index].location, route, totalDistanceTravelled);
			if (dr != DELIVERY_SUCCESS) {
				return dr;
			}
		}
		else { // go back to depot after last delivery has been made
			--index;
			dr = ptp.generatePointToPointRoute(optimizedDeliveries[index].location, depot, route, totalDistanceTravelled);
		}

		StreetSegment* prevSeg = nullptr;
		StreetSegment* currSeg = nullptr;
		string itemToDeliver = optimizedDeliveries[index].item;
		string prevStreet = ""; // name of previous street segment

		DeliveryCommand deliver;
		deliver.initAsDeliverCommand(itemToDeliver);

		auto it = route.begin();
		while (it != route.end()) { // go through all the route's steps
			DeliveryCommand proceed;
			DeliveryCommand turn;
			prevSeg = currSeg;
			currSeg = &*it;
			string streetName = currSeg->name;

			double direction = angleOfLine(*currSeg); // get direction to proceed on a street (number)
			string dirToTravel; // get direction to proceed on a street (string)
			double turnDir = 0; // check if need to turn
			double distAlongStreet = distanceEarthMiles((*currSeg).start, (*currSeg).end); // distance to go along street

			getDirectionToTravel(direction, dirToTravel); // find direction street is pointing

			if (prevSeg == nullptr) { // if it's the first segment, get the name of the street
				prevStreet = currSeg->name;
			}
			else { // if it's not the first segment, calculate turn angle
				turnDir = angleBetween2Lines(*prevSeg, *currSeg);
			}

			// 4 cases below for each new segment
			if (prevSeg == nullptr) { // case 1: it's the first segment
				proceed.initAsProceedCommand(dirToTravel, streetName, distAlongStreet);
				commands.push_back(proceed);
			}
			else if (prevSeg != nullptr && prevStreet == currSeg->name) { // case 2: not the first segment and same street as previous segment
				proceed = commands.back();
				commands.pop_back();
				proceed.increaseDistance(distAlongStreet);
				commands.push_back(proceed);
			}
			else if (turnDir >= 1 && turnDir <= 359) { // case 3: need to turn onto new street
				string dir;
				if (turnDir >= 1 && turnDir < 180) {
					dir = "left";
				}
				else {
					dir = "right";
				}
				turn.initAsTurnCommand(dir, streetName);
				commands.push_back(turn);
				proceed.initAsProceedCommand(dirToTravel, streetName, distAlongStreet);
				commands.push_back(proceed);
				prevStreet = currSeg->name;
			}
			else { // case 4: don't need to turn onto new street
				proceed.initAsProceedCommand(dirToTravel, streetName, distAlongStreet);
				commands.push_back(proceed);
				prevStreet = currSeg->name;
			}

			if ((*currSeg).end == optimizedDeliveries[index].location) { // deliver command
				commands.push_back(deliver);
			}
			it++;
		}
	}
	return DELIVERY_SUCCESS;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
