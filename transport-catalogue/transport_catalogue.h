#pragma once
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <string_view>
#include "geo.h"
#include <numeric>
#include <algorithm>
#include <string>

namespace detail {

	class Bus;

	class Stop {
		friend Bus;
	public:
		Stop(const std::string& name, Coordinates coordinates);

		void AddBusToStop(const std::string& bus);

		std::unordered_set<std::string>* ShowBusesToStop();

	private:
		std::string name_;
		Coordinates coordinates_;
		std::unordered_set<std::string> buses_to_stop_;
	};

	class Bus {
	public:
		Bus(const std::string& name, const std::vector<Stop*>& route);

		size_t number_of_stops();

		size_t number_of_unique_stops();

		double count_route_length();

	private:
		std::string name_;
		std::vector<Stop*> route_;
	};
}

class TransportCatalogue {
	// Реализуйте класс самостоятельно
public:
	void AddStop(const std::string& name, Coordinates coordinates);

	void AddBus(const std::string& name, const std::vector<std::string_view>& route);

	detail::Stop* GetStopInfo(const std::string& name) const;

	detail::Bus* GetBusInfo(const std::string& name) const;

	bool FindBus(const std::string& name) const;

	bool FindStop(const std::string& name) const;

	/*Расстояний у нас еще не было. Они будут в следующем спринте*/

	~TransportCatalogue();

private:
	std::unordered_map<std::string, detail::Bus*> buses_list;
	std::unordered_map<std::string, detail::Stop*> stops_list;

	std::deque<std::string> all_stops;
	std::deque<std::string> all_buses;
};
