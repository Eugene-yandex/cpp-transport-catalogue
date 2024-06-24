#include "request_handler.h"
#include "transport_catalogue.h"
#include <algorithm>
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

RequestHandler::RequestHandler(const catalog::TransportCatalogue& db) :
	db_(db) {
}

std::optional<domain::BusInformation> RequestHandler::GetBusInfo(std::string_view bus) const {

	domain::BusInformation result;
	auto bus_ptr = db_.FindBus(bus);
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
		result.route_length_real += db_.ComputeDistanceRealDistance(stop, next_stop);
		result.route_length_geographical_coordinates += ComputeDistanceGeographicalCoordinates(stop->stop_coordinates, next_stop->stop_coordinates);
	}

	return result;
}

const std::vector<std::string_view> RequestHandler::GetStopInfo(std::string_view stop) const {
	return db_.GetStopInfo(stop);
}