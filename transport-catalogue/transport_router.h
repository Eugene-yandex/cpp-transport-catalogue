#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <iterator>
#include <cmath>
#include <climits>
#include <variant>

const size_t METERS_IN_KM= 1000;
const size_t MIN_IN_HOUR = 60;

namespace transport_router {
	struct RouteBusStopId {
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
		struct RouteWait {
			std::string_view stop;
			int time = 0;
		};

		struct RouteBus {
			std::string_view bus;
			size_t span_count = 0;
			Weight time;
		};
		using Value = std::variant <RouteWait, RouteBus>;
		struct RoutsInfo {
			Weight weight;
			std::vector<Value> steps;
		};

		explicit TransportRouter(const catalog::TransportCatalogue& tansport_catalogue, int bus_wait_time, double bus_velocity);

		std::optional<RoutsInfo> FindRout(std::string_view from, std::string_view to) const;

	private:
		std::unordered_set<graph::Edge<Weight>, EdgeHasher<Weight>> all_edge_;
		const catalog::TransportCatalogue& tansport_catalogue_;
		size_t vertex_count_ = 0;
		int bus_wait_time_;
		double bus_velocity_;
		const graph::DirectedWeightedGraph<Weight> graph_;
		const graph::Router<Weight> router_;


		graph::DirectedWeightedGraph<Weight> MakeGraph();
		void AddEdgefromRouteBusStopId(const std::vector<RouteBusStopId>& roud_stopsid, std::string_view bus);

		template <typename It>
		void AddEdge(It begin_stop, It end_stop, std::string_view bus);

	};

	template <typename Weight>
	TransportRouter<Weight>::TransportRouter(const catalog::TransportCatalogue& tansport_catalogue, int bus_wait_time, double bus_velocity) :
		tansport_catalogue_(tansport_catalogue), vertex_count_(tansport_catalogue.GetStopsSize()), 
		bus_wait_time_(bus_wait_time), bus_velocity_(bus_velocity), graph_(MakeGraph()), router_(graph_) {
		
	}
	template <typename Weight>
	graph::DirectedWeightedGraph<Weight> TransportRouter<Weight>::MakeGraph() {
		const auto& buses = tansport_catalogue_.GetNoEmptyBus();
		for (const auto& bus : buses) {
			const auto& stops = bus->stops;
			if (bus->last_stop) {
				size_t median = stops.size() / 2;
				AddEdge(stops.begin(), stops.begin() + median + 1, bus->name);
				AddEdge(stops.begin() + median, stops.end(), bus->name);
			}
			else {
				AddEdge(stops.begin(), stops.end(), bus->name);
			}
		}
		graph::DirectedWeightedGraph<Weight> graph(vertex_count_ * 2);
		for (const auto& edge : all_edge_) {
			graph.AddEdge(edge);
		}
		return graph;
	}
	template <typename Weight>
	void TransportRouter<Weight>::AddEdgefromRouteBusStopId(const std::vector<RouteBusStopId>& roud_stopsid, std::string_view bus) {
		if (roud_stopsid.size() > 2) {
			for (int road_stops_index_begin = 0; road_stops_index_begin < roud_stopsid.size() - 1; ++road_stops_index_begin) {
				auto& first_roud_stopsid = roud_stopsid.at(road_stops_index_begin);
				size_t stops_count = 2;
				for (int road_stops_index_end = road_stops_index_begin + 2; road_stops_index_end < roud_stopsid.size(); ++road_stops_index_end) {
					auto& next_roud_stopsid = roud_stopsid.at(road_stops_index_end);
					size_t dis = next_roud_stopsid.all_distance - first_roud_stopsid.all_distance;
					if (road_stops_index_begin == 0 && road_stops_index_end == roud_stopsid.size() - 1 && first_roud_stopsid.stop_id == next_roud_stopsid.stop_id) {
						continue;
					}
					all_edge_.insert(graph::Edge<Weight>{first_roud_stopsid.stop_id + vertex_count_, next_roud_stopsid.stop_id,
						dis / (bus_velocity_ * METERS_IN_KM / MIN_IN_HOUR), stops_count, bus });
					++stops_count;
				}
			}
		}
	}
	template<typename Weight>
	template<typename It>
	inline void TransportRouter<Weight>::AddEdge(It begin_stop, It end_stop, std::string_view bus) {
		std::vector<RouteBusStopId> roud_stopsid(std::distance(begin_stop, end_stop));
		size_t all_distance = 0;
		auto stop_parce = std::prev(end_stop);
		size_t from = tansport_catalogue_.GetStopId(*begin_stop);
		roud_stopsid.at(0).stop_id = from;
		size_t stops_count = 1;
		for (auto it = begin_stop; it != stop_parce; ++it) {
			size_t to = tansport_catalogue_.GetStopId(*std::next(it));
			all_edge_.insert(graph::Edge<Weight>{from, from + vertex_count_, bus_wait_time_ * 1.0, 0, {}});
			size_t distance = static_cast<size_t>(tansport_catalogue_.GetRealDistanceStops(*it, *std::next(it)));
			all_distance += distance;
			roud_stopsid.at(stops_count).stop_id = to;
			roud_stopsid.at(stops_count).all_distance = all_distance;
			roud_stopsid.at(stops_count).distance_1ost = distance;
			++stops_count;
			all_edge_.insert(graph::Edge<Weight>{from + vertex_count_, to, distance / (bus_velocity_ * METERS_IN_KM / MIN_IN_HOUR), 1, bus });
			from = to;
		}
		AddEdgefromRouteBusStopId(roud_stopsid, bus);
	}
	template<typename Weight>
	inline std::optional<typename TransportRouter<Weight>::RoutsInfo> TransportRouter<Weight>::FindRout(std::string_view stop_from, std::string_view stop_to) const {
		auto route_from_stopid = router_.BuildRoute(tansport_catalogue_.GetStopId(tansport_catalogue_.FindStop(stop_from)),
			tansport_catalogue_.GetStopId(tansport_catalogue_.FindStop(stop_to)));
		if (route_from_stopid) {
			RoutsInfo route;
			route.weight = route_from_stopid.value().weight;
			size_t stops_count = tansport_catalogue_.GetStopsSize();
			for (const auto& edgeid : route_from_stopid.value().edges) {
				const auto& edge = graph_.GetEdge(edgeid);
				size_t id_stops = graph_.GetEdge(edgeid).from;
				if (graph_.GetEdge(edgeid).to >= stops_count) {
					RouteWait route_wait;
					route_wait.stop = tansport_catalogue_.GetStopNamefromId(id_stops);
					route_wait.time = bus_wait_time_;
					route.steps.push_back(route_wait);
				}
				else {
					RouteBus route_bus;
					route_bus.bus = edge.bus;
					route_bus.span_count = edge.stops_distatce;
					route_bus.time = edge.weight;
					route.steps.push_back(route_bus);
				}
			}
			return route;
		}
		return std::nullopt;
	}
}

	

