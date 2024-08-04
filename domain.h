#pragma once
#include <string>

#include "geo.h"
#include <vector>
#include <optional>


namespace domain {
	struct Stop {
		std::string name;
		Coordinates stop_coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> stops;
		std::optional<Stop*> last_stop;
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
}