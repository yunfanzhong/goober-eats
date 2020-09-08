# Route Optimization Program (Goober Eats) #

Given a list of delivery points, this program creates an optimized delivery route from point A -> B -> ... -> A.

It rearranges the list of delivery points to find the shortest route via simulated annealing (in DeliveryOptimizer.cpp), and creates an optimal route via an A* search algorithm of longitude and latitude points (in PointToPointRouter.cpp).

## Logistics ##

This project utilizes the Open Street Maps Project's geolocation data for latitude/longitude points and street names. The information is stored in mapdata.txt, which currently contains the information for Westwood (around UCLA).

deliveries.txt contains the list of delivery points, formatted in the following way: <br />
> Depot_Latitude Depot_Longitude <br />
> Delivery1_Latitude Delivery1_Longitude:food item to be delivered <br />
> Delivery2_Latitude Delivery2_Longitude:food item to be delivered <br />
> … <br />
> DeliveryN_Latitude DeliveryN_Longitude:food item to be delivered <br />

The depot is the start and end location of the food delivery (ie where the food is prepped and given to the delivery person).

The program's output will be formatted like the example below (this is a cut down version of an actual output; typically, it will be much longer):
> Starting at the depot... <br />
> Proceed north on Broxton Avenue for 0.08 miles <br />
> Turn left on Le Conte Avenue <br />
> Proceed west on Le Conte Avenue for 0.12 miles <br />
> DELIVER Pabst Blue Ribbon beer <br />
> Proceed east on Strathmore Place for 0.03 miles <br />
> Turn left on Charles E Young Drive West <br />
> Proceed north on Charles E Young Drive West for 0.19 miles <br />
> DELIVER Chicken tenders <br />
> Proceed southeast on Levering Avenue for 0.08 miles <br />
> You are back at the depot and your deliveries are done! <br />
> 0.5 miles travelled for all deliveries. <br />

ExpandableHashMap is a custom, flexibly sized hash table that I coded to store the map information.

main.cpp implements a command-line interface.

## Building and Running ##

Compile the code with main.cpp. Open a Windows/macOS command shell and switch to the directory that holds the compiled executable file (along with mapdata.txt and deliveries.txt).

Then, run the following command in the terminal:
```
GooberEats.exe \path\to\mapdata.txt \path\to\deliveries.txt
```

This is an example of what the line will look like on Windows.
```
C:\PATH\TO\CODE> GooberEats.exe c:\path\to\mapdata.txt c:\path\to\deliveries.txt
```
