#include "provided.h"
#include "ExpandableHashMap.h"
#include <vector>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
	const StreetMap* streetMap;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
	streetMap = sm;
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void getDistance(GeoCoord depot, vector<DeliveryRequest> deliveries, double& distance) {
	distance = 0;
	GeoCoord prevCoord = depot;
	auto it = deliveries.begin();
	while (it != deliveries.end()) { // add up all distances to find the old crow's distance
		GeoCoord nextCoord = (*it).location;
		distance += distanceEarthMiles(prevCoord, nextCoord);
		prevCoord = nextCoord;
		it++;
	}
	distance += distanceEarthMiles(prevCoord, depot);
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const { // simulated annealing optimization algorithm
	oldCrowDistance = 0;
	newCrowDistance = 0;
	
	getDistance(depot, deliveries, oldCrowDistance);
	newCrowDistance = oldCrowDistance;

	double temp = 0.5; // set "temperature" to 0.5, decrease exponentially by 0.9x every iteration
	bool changed = false;
	for (int k = 0; k < 25; k++) {
		changed = false;
		for (unsigned int i = 0, numChanges = 0; i <= 100 * deliveries.size() && numChanges <= 10; i++) { // make this many random changes
			int transportOrReverse = rand() % 2; // randomly decide whether to reverse or transport part of the map
			unsigned int startPath = rand() % deliveries.size(); // randomly pick start of section to transform
			unsigned int endPath = rand() % (deliveries.size() - startPath) + startPath; // randomly pick end of section to transform

			vector<DeliveryRequest> tempDeliveries = deliveries;

			if (transportOrReverse == 0) { // reverse segment
				reverse(tempDeliveries.begin() + startPath, tempDeliveries.begin() + endPath);
			}
			else {
				for (unsigned int j = startPath; j <= endPath; j++) { // transport segment to end
					DeliveryRequest temp = tempDeliveries[j];
					tempDeliveries.erase(tempDeliveries.begin() + j);
					tempDeliveries.push_back(temp);
				}
			}

			getDistance(depot, tempDeliveries, newCrowDistance);

			int randInt = rand() % 100;
			double r = double(randInt) / 100.0;
			// determine whether to actually do the transformation (if distance decreases or fxn is less than random [0, 1)
			if (newCrowDistance < oldCrowDistance || exp(-(newCrowDistance - oldCrowDistance) / temp) > r) {
				deliveries = tempDeliveries;
				numChanges++;
				changed = true;
			}
			else {
				newCrowDistance = oldCrowDistance; // no changes made
			}
		}
		temp = temp * 0.9; // exponentially decrease temperature
		if (changed == false) {
			k = 30; // end the loop, no changes were made in the last iteration
		}
	}
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
