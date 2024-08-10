#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <iostream>

#include "domain.h"
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"


namespace jreader{

    struct DescriptionCommandStop {
        std::string_view stop;
        Coordinates coordinates;
        std::unordered_map<std::string, int> distance;
    };

    class JSONReader {
    public:
        JSONReader(std::istream& input);
        void CreateDatabase(catalog::TransportCatalogue& catalogue);
        void PrintStat(const catalog::TransportCatalogue& tansport_catalogue, std::ostream& output);

    private:
        std::vector<json::Node> base_requests_;
        std::vector<json::Node> stat_requests_;
        json::Dict render_settings_;
        json::Dict routing_settings_;


        void ApplyCommandsStop([[maybe_unused]] catalog::TransportCatalogue& catalogue);
        void ApplyCommandsBus([[maybe_unused]] catalog::TransportCatalogue& catalogue);
        DescriptionCommandStop AddDescriptionCommandStop(const json::Dict& map_info);
        std::pair<std::vector<std::string_view>, std::optional<std::string_view>> AddRoute(const json::Array& stops, bool ring) ;
        json::Node PrintBus(const catalog::TransportCatalogue& tansport_catalogue, int id, std::string_view name) const;
        json::Node PrintStop(const catalog::TransportCatalogue& tansport_catalogue, int id, std::string_view name) const;
        svg::Color GetColorRenderSettings(const json::Node& node) const;
        renderer::MapRenderer MakeMapRenderer() const;
        json::Node PrintRenderMap(const catalog::TransportCatalogue& tansport_catalogue, int id) const;
        svg::Document RenderMap(const catalog::TransportCatalogue& tansport_catalogue) const;
        json::Node PrintRoute(const catalog::TransportCatalogue& tansport_catalogue, transport_router::TransportRouter<double>& transport_router,
            int id, std::string_view from, std::string_view to) const;

    };
}