#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <iterator>
#include <cmath>
#include <climits>

namespace transport_router {
	struct RoutBusStopId {
		size_t stop_id;
		size_t all_distance;
		size_t distance_1ost;
	};

	template <typename Weight>
	struct EdgeHasher {
		size_t operator() (const graph::Edge<Weight>& edge) const {
			return edge.to * 43 * 71 * 31 + edge.from * 43 * 31 + edge.weight * 43 + std::hash<std::string_view>{}(edge.bus);
		}
	};

	template <typename Weight>
	class TransportRouter {
	public:
		explicit TransportRouter(const catalog::TransportCatalogue& tansport_catalogue);

		graph::DirectedWeightedGraph<Weight> MakeGraph(int bus_wait_time, double bus_velocity);

	private:
		std::unordered_set<graph::Edge<Weight>, EdgeHasher<Weight>> all_edge_;
		const catalog::TransportCatalogue& tansport_catalogue_;
		size_t vertex_count_ = 0;

		void AddEdgefromRoutBusStopId(const std::vector<RoutBusStopId>& roud_stopsid, std::string_view bus, double bus_velocity);

		template <typename It>
		void AddEdge(It begin_stop, It end_stop, int bus_wait_time, double bus_velocity, std::string_view bus);
	};

	template <typename Weight>
	TransportRouter<Weight>::TransportRouter(const catalog::TransportCatalogue& tansport_catalogue) :
		tansport_catalogue_(tansport_catalogue), vertex_count_(tansport_catalogue.GetStopsSize()) {

	}
	template <typename Weight>
	graph::DirectedWeightedGraph<Weight> TransportRouter<Weight>::MakeGraph(int bus_wait_time, double bus_velocity) {
		const auto& buses = tansport_catalogue_.GetNoEmptyBus();
		for (const auto& bus : buses) {
			const auto& stops = bus->stops;
			if (bus->last_stop) {
				size_t median = stops.size() / 2;
				AddEdge(stops.begin(), stops.begin() + median + 1, bus_wait_time, bus_velocity, bus->name);
				AddEdge(stops.begin() + median, stops.end(), bus_wait_time, bus_velocity, bus->name);
			}
			else {
				AddEdge(stops.begin(), stops.end(), bus_wait_time, bus_velocity, bus->name);
			}
		}
		graph::DirectedWeightedGraph<Weight> graph(vertex_count_ * 2);
		for (const auto& edge : all_edge_) {
			graph.AddEdge(edge);
		}
		return graph;
	}
	template <typename Weight>
	void TransportRouter<Weight>::AddEdgefromRoutBusStopId(const std::vector<RoutBusStopId>& roud_stopsid, std::string_view bus, double bus_velocity) {
		if (roud_stopsid.size() > 2) {
			for (int i = 0; i < roud_stopsid.size() - 1; ++i) {
				auto& first_roud_stopsid = roud_stopsid.at(i);
				size_t stops_count = 2;
				for (int x = i + 2; x < roud_stopsid.size(); ++x) {
					auto& next_roud_stopsid = roud_stopsid.at(x);
					size_t dis = next_roud_stopsid.all_distance - first_roud_stopsid.all_distance;
					if (i == 0 && x == roud_stopsid.size() - 1 && first_roud_stopsid.stop_id == next_roud_stopsid.stop_id) {
						continue;
					}
					all_edge_.insert(graph::Edge<Weight>{first_roud_stopsid.stop_id + vertex_count_, next_roud_stopsid.stop_id,
						dis / (bus_velocity * 1000 / 60), stops_count, bus });
					++stops_count;
				}
			}
		}
	}
	template<typename Weight>
	template<typename It>
	inline void TransportRouter<Weight>::AddEdge(It begin_stop, It end_stop, int bus_wait_time, double bus_velocity, std::string_view bus) {
		std::vector<RoutBusStopId> roud_stopsid(std::distance(begin_stop, end_stop));
		size_t all_distance = 0;
		auto stop_parce = std::prev(end_stop);
		size_t from = tansport_catalogue_.GetStopId(*begin_stop);
		roud_stopsid.at(0).stop_id = from;
		size_t stops_count = 1;
		for (auto it = begin_stop; it != stop_parce; ++it) {
			size_t to = tansport_catalogue_.GetStopId(*std::next(it));
			all_edge_.insert(graph::Edge<Weight>{from, from + vertex_count_, bus_wait_time * 1.0, 0, {}});
			size_t distance = static_cast<size_t>(tansport_catalogue_.GetRealDistanceStops(*it, *std::next(it)));
			all_distance += distance;
			roud_stopsid.at(stops_count).stop_id = to;
			roud_stopsid.at(stops_count).all_distance = all_distance;
			roud_stopsid.at(stops_count).distance_1ost = distance;
			++stops_count;
			all_edge_.insert(graph::Edge<Weight>{from + vertex_count_, to, distance / (bus_velocity * 1000 / 60), 1, bus });
			from = to;
		}
		AddEdgefromRoutBusStopId(roud_stopsid, bus, bus_velocity);
	}
}

	

