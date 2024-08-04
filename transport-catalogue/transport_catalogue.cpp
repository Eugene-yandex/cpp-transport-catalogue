#include "transport_catalogue.h"
#include "domain.h"


#include <algorithm>

namespace catalog {

	size_t PairStopsHasher::operator() (const std::pair<domain::Stop*, domain::Stop*>& pair_stops) const {
		std::hash<const void*> stop_hasher;
		auto h1 = stop_hasher(pair_stops.first);
		auto h2 = stop_hasher(pair_stops.second);
		return h1 * 43 + h2;
	}

	void TransportCatalogue::AddStop(const std::string& stop, const Coordinates& coordinates) {
		stops_.push_back({ stop,coordinates });
		search_stop_.insert({ stops_.back().name, &stops_.back() });
		buses_of_stop_.insert({ &stops_.back(), {} });
	}

	void TransportCatalogue::AddDistance(const std::string_view stop1, const std::string_view stop2, int distance) {
		if (search_stop_.count(stop1) > 0 && search_stop_.count(stop2) > 0) {
			distance_stops_.insert({ {search_stop_.at(stop1), search_stop_.at(stop2)},distance });
		}
	}


	void TransportCatalogue::AddBus(const std::string& bus, const std::vector<std::string_view>& stops, const std::optional<std::string_view>& end_stop) {
		std::vector<domain::Stop*> result;
		buses_.push_back({ bus, result,std::nullopt });
		if (!stops.empty()) {
			noempty_bus_.push_back(&buses_.back());
			for (auto stop : stops) {
				auto stop_ptr = search_stop_.at(stop);
				if (end_stop == stop && buses_.back().last_stop == std::nullopt) {
					buses_.back().last_stop = stop_ptr;
				}
				result.push_back(stop_ptr);
				if (std::find(buses_of_stop_.at(stop_ptr).begin(), buses_of_stop_.at(stop_ptr).end(), bus) == buses_of_stop_.at(stop_ptr).end()) {
					buses_of_stop_.at(stop_ptr).push_back(buses_.back().name);
				}
			}
			buses_.back().stops = std::move(result);
		}
		search_bus_.insert({ buses_.back().name, &buses_.back() });

	}

	size_t TransportCatalogue::GetStopsSize() const {
		return stops_.size();
	}

	size_t TransportCatalogue::GetStopId(const domain::Stop* stop) const {
		auto it = std::find_if(stops_.begin(),
			stops_.end(),
			[stop](const domain::Stop& ostanovka) {return stop == &ostanovka; });

		return std::distance(stops_.begin(), it);
	}

	const std::string& TransportCatalogue::GetStopNamefromId(size_t id) const {
		return stops_.at(id).name;
	}

	domain::Stop* TransportCatalogue::FindStop(std::string_view stop) const {
		return search_stop_.count(stop) > 0 ? search_stop_.at(stop) : nullptr;
	}

	domain::Bus* TransportCatalogue::FindBus(std::string_view bus) const {
		return search_bus_.count(bus) > 0 ? search_bus_.at(bus) : nullptr;
	}
	bool TransportCatalogue::AreBusesHaveStop(std::string_view str) const {
		auto stop = search_stop_.count(str) > 0 ? search_stop_.at(str) : nullptr;
		if (stop != nullptr) {
			return buses_of_stop_.count(stop) > 0;
		}
		return false;
	}
	int TransportCatalogue::GetRealDistanceStops(domain::Stop* from, domain::Stop* to) const { 
		if (distance_stops_.count({ from , to }) > 0) {
			return distance_stops_.at({ from , to });
		}
		else {
			return distance_stops_.at({ to , from });
		}
	}

	const std::deque<domain::Bus*>& TransportCatalogue::GetNoEmptyBus() const {
		return noempty_bus_;
	}


	const std::vector<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stop) const {
		std::vector<std::string_view> result;
		using namespace std::literals;
		if (search_stop_.count(stop) == 0) {
			return result;
		}
		else if (buses_of_stop_.at(search_stop_.at(stop)).empty()) {
			result.push_back("no buses"sv);
			return result;
		}
		else {
			result = buses_of_stop_.at(search_stop_.at(stop));
			std::sort(result.begin(), result.end());
			return result;
		}
	}

	std::optional<domain::BusInformation> TransportCatalogue::GetBusInfo(std::string_view bus) const {

		domain::BusInformation result;
		auto bus_ptr = FindBus(bus);
		if (!bus_ptr) {
			return std::nullopt;
		}
		result.count_stops = bus_ptr->stops.size();

		for (int stop_index = 0; stop_index < result.count_stops; ++stop_index) {
			auto stop = bus_ptr->stops[stop_index];
			auto poisk = std::find(bus_ptr->stops.begin(), bus_ptr->stops.begin() + stop_index, stop);
			if (poisk == bus_ptr->stops.begin() + stop_index) {
				++result.unique_stops;
			}
			if (stop_index == result.count_stops - 1) {
				continue;
			}
			auto next_stop = bus_ptr->stops[stop_index + 1];
			result.route_length_real += GetRealDistanceStops(stop, next_stop);
			result.route_length_geographical_coordinates += ComputeDistanceGeographicalCoordinates(stop->stop_coordinates, next_stop->stop_coordinates);
		}

		return result;
	}
}