#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>

namespace catalog {

	void TransportCatalogue::AddStop(const std::string& stop, const Coordinates& coordinates) {
		stops_.push_back({ stop,coordinates });
		search_stop_.insert({ stops_.back().name, &stops_.back() });
		buses_of_stop.insert({ &stops_.back(), {} });
	}

	void TransportCatalogue::AddBus(const std::string& bus, const std::vector<std::string_view>& stops) {
		std::vector<Stop*> result;
		buses_.push_back({ bus, result });
		for (auto stop : stops) {
			auto stop_ptr = search_stop_.at(stop);
			result.push_back(stop_ptr);
			if (std::find(buses_of_stop.at(stop_ptr).begin(), buses_of_stop.at(stop_ptr).end(), bus) == buses_of_stop.at(stop_ptr).end()) {
				buses_of_stop.at(stop_ptr).push_back(buses_.back().name);
			}
		}
		buses_.back().stops = std::move(result);
		search_bus_.insert({ buses_.back().name, &buses_.back() });

	}

	const Stop* TransportCatalogue::FindStop(std::string_view stop) const {
		return search_stop_.count(stop) > 0 ? search_stop_.at(stop) : nullptr;
	}

	const Bus* TransportCatalogue::FindBus(std::string_view bus) const {
		return search_bus_.count(bus) > 0 ? search_bus_.at(bus) : nullptr;
	}

	BusInformation TransportCatalogue::GetBusInfo(std::string_view bus) const {

		BusInformation result;
		auto bus_ptr = FindBus(bus);
		if (!bus_ptr) {
			return result;
		}
		result.count_stops = bus_ptr->stops.size();

		for (size_t stop_index = 0; stop_index < result.count_stops; ++stop_index) {
			auto stop = bus_ptr->stops[stop_index];
			auto poisk = std::find(bus_ptr->stops.begin(), bus_ptr->stops.begin() + stop_index, stop);
			if (poisk == bus_ptr->stops.begin() + stop_index) {
				++result.unique_stops;
			}
			if (stop_index == result.count_stops - 1) {
				continue;
			}
			auto next_stop = bus_ptr->stops[stop_index + 1];
			result.route_length += ComputeDistance(stop->stop_coordinates, next_stop->stop_coordinates);
		}

		return result;
	}

	std::vector<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stop) const {
		std::vector<std::string_view> result;
		using namespace std::literals;
		if (search_stop_.count(stop) == 0) {
			return result;
		}
		else if (buses_of_stop.at(search_stop_.at(stop)).empty()) {
			result.push_back("no buses"sv);
			return result;
		}
		else {
			result = buses_of_stop.at(search_stop_.at(stop));
			std::sort(result.begin(), result.end());
			return result;
		}
	}
}
