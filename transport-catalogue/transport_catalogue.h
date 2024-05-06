#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cassert>
#include <vector>
#include "geo.h"

namespace catalog {
	class TransportCatalogue {
	public:
		TransportCatalogue() = default;

		struct Stop {
			std::string name;
			Coordinates stops_coordinat;
		};

		struct Bus {
			std::string name;
			std::vector<Stop*> marshrut;
		};

		void AddStop(std::string stop, Coordinates coordinats);
		void AddBus(std::string bus, std::vector<std::string_view> stops);

		Stop& FindStop(std::string_view stop) const;
		Bus& FindBus(std::string_view bus) const;

		std::vector<double> BusInfo(std::string_view bus) const;
		std::vector<std::string> StopInfo(std::string_view stop) const;


	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Stop*> search_stop_;
		std::unordered_map<std::string_view, Bus*> search_bus_;
		std::unordered_map<Stop*, std::vector<Bus*>> buses_of_stop;

	};
}
