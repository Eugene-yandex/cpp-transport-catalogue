#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>

namespace catalog {

	void TransportCatalogue::AddStop(std::string stop, Coordinates coordinates) {
		stops_.push_back({ std::move(stop),std::move(coordinates) });
		search_stop_.insert({ stops_.back().name, &stops_.back() });
		buses_of_stop.insert({ &stops_.back(), {} });
	}

	void TransportCatalogue::AddBus(std::string bus, std::vector<std::string_view> stops) {
		std::vector<Stop*> result;
		buses_.push_back({ std::move(bus), result });
		for (auto stop : stops) {
			result.push_back(search_stop_.at(stop));
			buses_of_stop.at(search_stop_.at(stop)).push_back(&buses_.back());
		}
		buses_.back().marshrut = std::move(result);
		search_bus_.insert({ buses_.back().name, &buses_.back() });

	}

	TransportCatalogue::Stop& TransportCatalogue::FindStop(std::string_view stop) const {
		assert(search_stop_.count(stop) > 0);
		return *search_stop_.at(stop);
	}

	TransportCatalogue::Bus& TransportCatalogue::FindBus(std::string_view bus) const {
		assert(search_bus_.count(bus) > 0);
		return *search_bus_.at(bus);
	}

	std::vector<double> TransportCatalogue::BusInfo(std::string_view bus) const {
		std::vector<double> result;

		if (search_bus_.count(bus) == 0) {
			return result;
		}
		result.push_back(FindBus(bus).marshrut.size() * 1.0);

		int unikal_stop = 0;
		double route_length = 0.0;
		for (size_t x = 0; x < FindBus(bus).marshrut.size(); ++x) {
			auto stop = FindBus(bus).marshrut[x];
			auto poisk = std::find(FindBus(bus).marshrut.begin(), FindBus(bus).marshrut.begin() + x, stop);
			if (poisk == FindBus(bus).marshrut.begin() + x) {
				++unikal_stop;
			}
			if (x == FindBus(bus).marshrut.size() - 1) {
				continue;
			}
			auto next_stop = FindBus(bus).marshrut[x + 1];
			route_length += ComputeDistance(stop->stops_coordinat, next_stop->stops_coordinat);
		}
		result.push_back(unikal_stop);
		result.push_back(route_length);

		return result;
	}
	std::vector<std::string> TransportCatalogue::StopInfo(std::string_view stop) const {
		std::vector<std::string> result;
		using namespace std::literals;
		if (search_stop_.count(stop) == 0) {
			return result;
		}
		else if (buses_of_stop.at(search_stop_.at(stop)).empty()) {
			result.push_back("no buses"s);
		}
		else {
			for (auto x : buses_of_stop.at(search_stop_.at(stop))) {
				if (std::find(result.begin(), result.end(), x->name) == result.end()) {
					result.push_back(x->name);
				}
			}

			std::sort(result.begin(), result.end());
		}
		return result;
	}
}
