#include "input_reader.h"
#include "geo.h"

#include <iostream>
#include <unordered_set>
#include <forward_list>

namespace transport_catalogue {
	namespace input_requests_processing {
		using namespace std::string_literals;

		namespace detail {
			std::string GetToken(std::string& line, const std::string& delimiter) {
				size_t pos = line.find(delimiter);
				std::string token = std::move(line.substr(0, pos));
				line.erase(0, pos == std::string::npos ? pos : pos + delimiter.length());
				return token;
			}

			double ComputeDistanceBetweenStopsInRoute(const TransportCatalogue& transportCatalogue,
				const std::string& prevStopName, const std::string& currStopName) {
				return geo::ComputeDistance(transportCatalogue.FindStop(prevStopName)->coords,
					transportCatalogue.FindStop(currStopName)->coords);
			}
		}

		void UpdateTransportCatalogue(TransportCatalogue& transportCatalogue, std::istream& is)	{
			// Stops processing
			std::forward_list<std::pair<std::string, std::string>> stopsRequests;
			std::forward_list<std::string> busRequests;
			std::string line;
			std::getline(is, line);
			size_t numberOfRequests = std::stoul(line);
			for (size_t i = 0; i < numberOfRequests; ++i) {
				std::getline(is, line);
				std::string requestType = detail::GetToken(line, " "s);
				if (requestType == "Stop"s) {
					std::string stopName = detail::GetToken(line, ": "s);
					double latitude = std::stod(detail::GetToken(line, ", "s));
					double longitude = std::stod(detail::GetToken(line, ", "s));
					transportCatalogue.AddStop(stopName, latitude, longitude);
					stopsRequests.emplace_front(std::move(stopName), std::move(line));
				}
				else if (requestType == "Bus"s) {
					busRequests.emplace_front(std::move(line));
				}
			}
			// Process distances between stops
			for (auto& stopRequest : stopsRequests)	{
				while (stopRequest.second.find("m "s) != std::string::npos)	{
					unsigned int distance = std::stoul(detail::GetToken(stopRequest.second, "m to "s));
					std::string toStop = detail::GetToken(stopRequest.second, ", "s);
					transportCatalogue.AddDistanceBetweenStops(stopRequest.first, toStop, distance);
				}
			}
			// Process buses and related stops
			for (std::string& busRequest : busRequests) {
				std::string busNumber = detail::GetToken(busRequest, ": "s);
				ERouteType routeType;
				std::string delimiter;
				std::string firstStopName;
				if (busRequest.find(" - "s) == std::string::npos) {
					routeType = ERouteType::Circular;
					delimiter = " > "s;
				}
				else  {
					routeType = ERouteType::Pendulum;
					delimiter = " - "s;
				}
				std::unordered_set<const Stop*> route;
				double geographicRouteLength = 0.0;
				unsigned int facticalRouteLength = 0;
				std::string prevStopName;
				std::string currStopName;
				size_t stopsOnRoute = 0;
				for (; busRequest.find(delimiter) != std::string::npos; ++stopsOnRoute)	{
					prevStopName = std::move(currStopName);
					currStopName = detail::GetToken(busRequest, delimiter);
					route.emplace(transportCatalogue.FindStop(currStopName));
					if (stopsOnRoute) {
						geographicRouteLength += detail::ComputeDistanceBetweenStopsInRoute(transportCatalogue, prevStopName, currStopName);
						facticalRouteLength += transportCatalogue.GetDistanceBetweenStops(prevStopName, currStopName);
						if (routeType == ERouteType::Pendulum) {
							facticalRouteLength += transportCatalogue.GetDistanceBetweenStops(currStopName, prevStopName);
						}
					}
					else
					{
						firstStopName = currStopName;
					}
				}

				if (routeType == ERouteType::Circular)
				{
					++stopsOnRoute;
					geographicRouteLength += detail::ComputeDistanceBetweenStopsInRoute(transportCatalogue, currStopName, firstStopName);
					facticalRouteLength += transportCatalogue.GetDistanceBetweenStops(currStopName, firstStopName);
				}
				else if (routeType == ERouteType::Pendulum)
				{
					stopsOnRoute = stopsOnRoute * 2 + 1;
					prevStopName = std::move(currStopName);
					currStopName = detail::GetToken(busRequest, delimiter);
					route.emplace(transportCatalogue.FindStop(currStopName));
					geographicRouteLength += detail::ComputeDistanceBetweenStopsInRoute(transportCatalogue, prevStopName, currStopName);
					facticalRouteLength += transportCatalogue.GetDistanceBetweenStops(prevStopName, currStopName) +
						transportCatalogue.GetDistanceBetweenStops(currStopName, prevStopName);
					geographicRouteLength *= 2;
				}

				transportCatalogue.AddBus(busNumber, routeType, route.size(), stopsOnRoute, facticalRouteLength,
					static_cast<double>(facticalRouteLength) / geographicRouteLength);

				for (const auto& stop : route)
				{
					transportCatalogue.AddBusRelatedToStop(stop, busNumber);
				}
			}
		}
	}
}
