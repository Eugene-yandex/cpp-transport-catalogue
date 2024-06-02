#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <vector>
#include "geo.h"

namespace catalog {

	struct Stop {
		std::string name;
		Coordinates stop_coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> stops;
	};

	struct BusInformation {
		explicit operator bool() const {
			return count_stops == 0;
		}

		bool operator!() const {
			return !operator bool();
		}

		int count_stops = 0;
		int unique_stops = 0;
		int route_length_real = 0;
		double route_length_geographical_coordinates = 0.0;
	};

	struct PairStopsHasher {
		size_t operator() (const std::pair<Stop*, Stop*>& pair_stops) const;
	};

	class TransportCatalogue {
	public:
		TransportCatalogue() = default;

		void AddStop(const std::string& stop, const Coordinates& coordinates);
		void AddDistance(const std::string_view stop, const std::unordered_map<std::string, int>& all_distance);
		void AddBus(const std::string& bus, const std::vector<std::string_view>& stops);

		const Stop* FindStop(std::string_view stop) const;
		const Bus* FindBus(std::string_view bus) const;

		BusInformation GetBusInfo(std::string_view bus) const;
		const std::vector<std::string_view> GetStopInfo(std::string_view stop) const;


	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Stop*> search_stop_;
		std::unordered_map<std::string_view, Bus*> search_bus_;
		std::unordered_map<Stop*, std::vector<std::string_view>> buses_of_stop;
		std::unordered_map<std::pair<Stop*, Stop*>, int, PairStopsHasher> distance_stops;

		int ComputeDistanceRealDistance(Stop* from, Stop* to) const;
	};
}
