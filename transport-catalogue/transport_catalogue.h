#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <vector>
#include "domain.h"


namespace catalog {

	

	struct PairStopsHasher {
		size_t operator() (const std::pair<domain::Stop*, domain::Stop*>& pair_stops) const;
	};

	class TransportCatalogue {
	public:
		TransportCatalogue() = default;

		void AddStop(const std::string& stop, const Coordinates& coordinates);
		void AddDistance(const std::string_view stop1, const std::string_view stop2, int distance);
		void AddBus(const std::string& bus, const std::vector<std::string_view>& stops, const std::optional<std::string_view>& end_stop);
		
		size_t GetStopsSize() const;
		size_t GetStopId(const domain::Stop* stop) const;
		const std::string& GetStopNamefromId(size_t id) const;
		bool AreBusesHaveStop(std::string_view stop) const;

		domain::Stop* FindStop(std::string_view stop) const;
		domain::Bus* FindBus(std::string_view bus) const;

		const std::vector<std::string_view> GetStopInfo(std::string_view stop) const;
		int GetRealDistanceStops(domain::Stop* from, domain::Stop* to) const;
		const std::deque<domain::Bus*>& GetNoEmptyBus() const;
		std::optional<domain::BusInformation> GetBusInfo(std::string_view bus) const;
	

	private:
		std::deque<domain::Stop> stops_;
		std::deque<domain::Bus> buses_;
		std::unordered_map<std::string_view, domain::Stop*> search_stop_;
		std::unordered_map<std::string_view, domain::Bus*> search_bus_;
		std::unordered_map<domain::Stop*, std::vector<std::string_view>> buses_of_stop_;
		std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, int, PairStopsHasher> distance_stops_;
		std::deque<domain::Bus*> noempty_bus_;

	};
}