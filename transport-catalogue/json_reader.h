#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <iostream>

#include "domain.h"
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

namespace jreader{

    struct DescriptionCommandStop {
        std::string_view stop;
        Coordinates coordinates;
        std::unordered_map<std::string, int> distance;
    };

    class JsonInformation {
    public:
        JsonInformation(std::istream& input);
        void CreateDatabase(catalog::TransportCatalogue& catalogue);
        void PrintStat(const catalog::TransportCatalogue& tansport_catalogue, std::ostream& output);

    private:
        std::vector<json::Node> base_requests_;
        std::vector<json::Node> stat_requests_;
        json::Dict render_settings_;

        void ApplyCommandsStop([[maybe_unused]] catalog::TransportCatalogue& catalogue);
        void ApplyCommandsBus([[maybe_unused]] catalog::TransportCatalogue& catalogue);
        DescriptionCommandStop AddDescriptionCommandStop(const json::Dict& map_info);
        std::pair<std::vector<std::string_view>, std::optional<std::string_view>> AddRoute(const json::Array& stops, bool ring);
        json::Dict PrintBus(const catalog::TransportCatalogue& tansport_catalogue, int id, std::string_view name);
        json::Dict PrintStop(const catalog::TransportCatalogue& tansport_catalogue, int id, std::string_view name);
        svg::Color GetColorRenderSettings(const json::Node& node) const;
        renderer::MapRenderer MakeMapRenderer() const;
        void RenderLine(const renderer::MapRenderer& maprender, const std::deque<domain::Bus*>& buses,
            const renderer::SphereProjector& proj, svg::Document& doc) const;
        void RenderBusLabels(const renderer::MapRenderer& maprender, const std::deque<domain::Bus*>& buses,
            const renderer::SphereProjector& proj, svg::Document& doc) const;
        void RenderStopPoints(const renderer::MapRenderer& maprender, const std::vector<domain::Stop*>& stop,
            const renderer::SphereProjector& proj, svg::Document& doc) const;
        void RenderStopLabels(const renderer::MapRenderer& maprender, const std::vector<domain::Stop*>& stop,
            const renderer::SphereProjector& proj, svg::Document& doc) const; 
        svg::Document RenderMap(const catalog::TransportCatalogue& tansport_catalogue) const;
        json::Dict PrintRenderMap(const catalog::TransportCatalogue& tansport_catalogue, int id) const;

    };
}