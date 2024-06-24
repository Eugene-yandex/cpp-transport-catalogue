#include "json_reader.h"
#include "request_handler.h"


#include <algorithm>
#include <cassert>
#include <iterator>
#include <unordered_map>
#include <string>
#include <sstream>

namespace jreader {
    using namespace std::literals;

    JsonInformation::JsonInformation(std::istream& input) {
        json::Document tmp = json::Load(input);
        if (tmp.GetRoot().AsMap().count("base_requests"s) > 0) {
            base_requests_ = tmp.GetRoot().AsMap().at("base_requests"s).AsArray();
        }
        if (tmp.GetRoot().AsMap().count("stat_requests"s) > 0) {
            stat_requests_ = tmp.GetRoot().AsMap().at("stat_requests"s).AsArray();
        }
        if (tmp.GetRoot().AsMap().count("render_settings"s) > 0) {
            render_settings_ = tmp.GetRoot().AsMap().at("render_settings"s).AsMap();
        }
    }

    void JsonInformation::CreateDatabase(catalog::TransportCatalogue& catalogue) {
        if (!base_requests_.empty()) {
            ApplyCommandsStop(catalogue);
            ApplyCommandsBus(catalogue);
        }
        
    }

    void JsonInformation::ApplyCommandsStop([[maybe_unused]] catalog::TransportCatalogue& catalogue) {
        // Реализуйте метод самостоятельно
        std::vector<DescriptionCommandStop> description_stops;
        for (const auto& data : base_requests_) {
            if (data.AsMap().at("type"s) == "Stop"s) {
                DescriptionCommandStop description_stop = AddDescriptionCommandStop(data.AsMap());
                catalogue.AddStop(data.AsMap().at("name").AsString(), description_stop.coordinates);
                description_stops.push_back(std::move(description_stop));
            }
        }

        for (const auto& description_stop : description_stops) {
            for (const auto& [any_stop, distatnce] : description_stop.distance) {
                catalogue.AddDistance(description_stop.stop, any_stop, distatnce);
            }
        }
    }

    void JsonInformation::ApplyCommandsBus([[maybe_unused]] catalog::TransportCatalogue& catalogue) {
        // Реализуйте метод самостоятельно

        for (const auto& data : base_requests_) {
            if (data.AsMap().at("type"s) == "Bus"s) {
                std::pair<std::vector<std::string_view>, std::optional<std::string_view>> stops = AddRoute(data.AsMap().at("stops"s).AsArray(), data.AsMap().at("is_roundtrip"s).AsBool());
                catalogue.AddBus(data.AsMap().at("name").AsString(), stops);
            }
        }
    }

    DescriptionCommandStop JsonInformation::AddDescriptionCommandStop(const json::Dict& map_info) {
        DescriptionCommandStop result;
        result.stop = map_info.at("name").AsString();
        result.coordinates = { map_info.at("latitude").AsDouble(),map_info.at("longitude").AsDouble()};
        std::unordered_map<std::string, int> distance;
        if (map_info.count("road_distances") > 0) {
            auto road_distances = map_info.at("road_distances").AsMap();
            for (const auto& [stop, length] : road_distances) {
                distance.insert({ stop ,length.AsInt() });
            }
            result.distance = std::move(distance);
        }
        return result;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::pair<std::vector<std::string_view>, std::optional<std::string_view>> JsonInformation::AddRoute(const json::Array& stops, bool ring) {
        std::vector<std::string_view> results;
        if (stops.empty()) {
            return std::make_pair(results, std::nullopt);
        }
        for (const auto& stop : stops) {
            results.push_back(stop.AsString());
        }
        if (!ring) {
            std::string_view last_stop;
            last_stop = stops.rbegin()->AsString();
            results.insert(results.end(), std::next(results.rbegin()), results.rend());
            if (last_stop == *results.begin()) {
                return std::make_pair(results, std::nullopt);
            }
            return std::make_pair(results, last_stop);
        }
        return std::make_pair(results, std::nullopt);
       
    }

    json::Dict JsonInformation::PrintBus(const catalog::TransportCatalogue& tansport_catalogue,
        int id, std::string_view name) {
        RequestHandler request_handler(tansport_catalogue);
        json::Dict result;
        result.insert({ "request_id"s, id });
        auto businfo = request_handler.GetBusInfo(name);
        if (!businfo) {
            result.insert({ "error_message"s, json::Node{"not found"s} });
        }
        else {
            result.insert({ "curvature"s, businfo.value().route_length_real / businfo.value().route_length_geographical_coordinates });
            result.insert({ "route_length"s, businfo.value().route_length_real });
            result.insert({ "stop_count"s, businfo.value().count_stops });
            result.insert({ "unique_stop_count"s, businfo.value().unique_stops });
        }
        return result;
    }

    json::Dict JsonInformation::PrintStop(const catalog::TransportCatalogue& tansport_catalogue,
        int id, std::string_view name) {
        RequestHandler request_handler(tansport_catalogue);
        json::Dict result;
        result.insert({ "request_id"s, id });
        auto stops = request_handler.GetStopInfo(name);
        json::Array result_stop;
        if (stops.size() == 0) {
            result.insert({ "error_message"s, json::Node{"not found"s} });
        }
        else if (stops.size() == 1 && stops[0] == "no buses"sv) {
            result.insert({ "buses"s, result_stop });

        }
        else {
            for (const auto& stop : stops) {
                result_stop.emplace_back(json::Node(std::string(stop)));
            }
            result.insert({ "buses"s, result_stop });

        }
        return result;
    }

    svg::Color JsonInformation::GetColorRenderSettings(const json::Node& node) const {
        svg::Color color;
        if (node.IsString()) {
            color = node.AsString();
        }
        else {
            json::Array node_color = node.AsArray();

            if (node_color.size() < 4) {
                color = svg::Rgb{ static_cast<uint8_t>(node_color[0].AsInt()),static_cast<uint8_t>(node_color[1].AsInt()),
                    static_cast<uint8_t>(node_color[2].AsInt()) };
            }
            else {
                color = svg::Rgba{ static_cast<uint8_t>(node_color[0].AsInt()),static_cast<uint8_t>(node_color[1].AsInt()),
                    static_cast<uint8_t>(node_color[2].AsInt()), node_color[3].AsDouble() };
            }
        }
        return color;
    }

    void JsonInformation::PrintStat(const catalog::TransportCatalogue& tansport_catalogue, std::ostream& output) {

        using namespace std::literals;
        if (!stat_requests_.empty()) {
            json::Array results; 
            for (const auto& data : stat_requests_) {
                int id = data.AsMap().at("id"s).AsInt();
                if (data.AsMap().at("type"s) == "Bus"s) {
                    std::string name = data.AsMap().at("name"s).AsString();
                    results.emplace_back(PrintBus(tansport_catalogue, id, name));
                }
                else if (data.AsMap().at("type"s) == "Stop"s) {
                    std::string name = data.AsMap().at("name"s).AsString();
                    results.emplace_back(PrintStop(tansport_catalogue, id, name));
                }
                else if (data.AsMap().at("type"s) == "Map"s) {
                    results.emplace_back(PrintRenderMap(tansport_catalogue,id));
                }
            }
            json::Print(json::Document{ results }, output);
        }
    }

    json::Dict JsonInformation::PrintRenderMap(const catalog::TransportCatalogue& tansport_catalogue, int id) const{
        json::Dict result;
        result.insert({ "request_id"s, id }); 
        std::ostringstream out;
        svg::Document doc = RenderMap(tansport_catalogue);
        doc.Render(out);
        result.insert({ "map"s, out.str() });
        return result;
    }

    renderer::MapRenderer JsonInformation::MakeMapRenderer() const {
        renderer::MapParameters result;
        result.width = render_settings_.at("width"s).AsDouble();
        result.height = render_settings_.at("height"s).AsDouble();
        result.padding = render_settings_.at("padding"s).AsDouble();
        result.line_width = render_settings_.at("line_width"s).AsDouble();
        result.stop_radius = render_settings_.at("stop_radius"s).AsDouble();
        result.bus_label_font_size = render_settings_.at("bus_label_font_size"s).AsInt();
        result.stop_label_font_size = render_settings_.at("stop_label_font_size"s).AsInt();
        std::vector<double> bus_label_offset;
        for (const auto& x : render_settings_.at("bus_label_offset"s).AsArray()) {
            bus_label_offset.push_back(x.AsDouble());
        }
        result.bus_label_offset = bus_label_offset;
        std::vector<double> stop_label_offset;
        for (const auto& x : render_settings_.at("stop_label_offset"s).AsArray()) {
            stop_label_offset.push_back(x.AsDouble());
        }
        result.stop_label_offset = stop_label_offset;
        result.underlayer_color = GetColorRenderSettings(render_settings_.at("underlayer_color"s));

        result.underlayer_width = render_settings_.at("underlayer_width"s).AsDouble();
        std::vector<svg::Color> color_palette;
        for (const auto& x : render_settings_.at("color_palette"s).AsArray()) {
            color_palette.push_back(GetColorRenderSettings(x));
        }
        result.color_palette = color_palette;
        return renderer::MapRenderer(std::move(result));
    }

    svg::Document JsonInformation::RenderMap(const catalog::TransportCatalogue& tansport_catalogue) const {
         renderer::MapRenderer maprender = MakeMapRenderer();
         std::deque<domain::Bus*> buses = tansport_catalogue.GetNoEmptyBus();
        std::sort(buses.begin(), buses.end(),[](domain::Bus* lhs, domain::Bus* rhs) {
            return lhs->name < rhs->name;
            });
        std::vector<domain::Stop*> stops;
       
        for (const auto& bus : buses) {
            if (!buses.empty()) {
                for (const auto& stop : bus->stops) {
                    if (std::find(stops.begin(), stops.end(), stop)==stops.end())  {
                        stops.push_back(stop);
                    }
                }
            }
        }
        const renderer::SphereProjector proj{
        stops.begin(), stops.end(), maprender.GetWidth(), maprender.GetHeight(), maprender.GetPadding()
        };
        svg::Document doc;
        RenderLine(maprender, buses, proj, doc);
        RenderBusLabels(maprender, buses, proj, doc);
        std::sort(stops.begin(), stops.end(), [](domain::Stop* lhs, domain::Stop* rhs) {
            return lhs->name < rhs->name;
            });
        RenderStopPoints(maprender, stops, proj, doc);
        RenderStopLabels(maprender, stops, proj, doc);
        return doc;
    }

    void JsonInformation::RenderLine(const renderer::MapRenderer& maprender, const std::deque<domain::Bus*>& buses, 
        const renderer::SphereProjector& proj, svg::Document& doc) const {
       
        int i = 0;
        int max_color_palette = maprender.GetColorPalette().size();
        for (const auto& bus : buses) {
            svg::Polyline result;
            if (i >= max_color_palette) {
                i = 0;
            }
            for (const auto& stop : bus->stops) {
                result.AddPoint(proj(stop->stop_coordinates));
            }
            result.SetStrokeColor(maprender.GetColorPalette()[i]).SetStrokeWidth(maprender.GetLine_width()).
                SetFillColor(svg::NoneColor).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            doc.Add(std::move(result));
            ++i;
        }
        
    }

    void JsonInformation::RenderBusLabels(const renderer::MapRenderer& maprender, const std::deque<domain::Bus*>& buses,
        const renderer::SphereProjector& proj, svg::Document& doc) const {
        int i = 0;
        int max_color_palette = maprender.GetColorPalette().size();
        for (const auto& bus : buses) {
            svg::Text substrate;
            substrate.SetPosition(proj(bus->stops[0]->stop_coordinates)).SetOffset({ maprender.GetBusLabelOffset()[0],maprender.GetBusLabelOffset()[1] }).
                SetFontSize(maprender.GetBusLabelFontSize()).SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetData(bus->name);
            svg::Text text = substrate;
            substrate.SetFillColor(maprender.GetUnderlayerColor()).SetStrokeColor(maprender.GetUnderlayerColor()).
                SetStrokeWidth(maprender.GetUnderlayerWidth()).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            text.SetFillColor(maprender.GetColorPalette()[i % max_color_palette]);
            if (bus->last_stop != std::nullopt) {
                svg::Text substrate_end_stop = substrate;
                svg::Text text_end_stop = text;
                Coordinates point = bus->last_stop.value()->stop_coordinates;
                substrate_end_stop.SetPosition(proj(point));
                text_end_stop.SetPosition(proj(point));
                doc.Add(std::move(substrate));
                doc.Add(std::move(text));
                doc.Add(std::move(substrate_end_stop));
                doc.Add(std::move(text_end_stop));
            }
            else {
                doc.Add(std::move(substrate));
                doc.Add(std::move(text));
            }
            ++i;
        }
    }
    void JsonInformation::RenderStopPoints(const renderer::MapRenderer& maprender, const std::vector<domain::Stop*>& stops,
        const renderer::SphereProjector& proj, svg::Document& doc) const {
        for (const auto& stop : stops) {
            svg::Circle circle;
            circle.SetCenter(proj(stop->stop_coordinates)).SetRadius(maprender.GetStop_radius()).SetFillColor("white"s);
            doc.Add(std::move(circle));
        }
    }

    void JsonInformation::RenderStopLabels(const renderer::MapRenderer& maprender, const std::vector<domain::Stop*>& stops,
            const renderer::SphereProjector& proj, svg::Document& doc) const {
        for (const auto& stop : stops) {
            svg::Text substrate;
            substrate.SetPosition(proj(stop->stop_coordinates)).SetOffset({ maprender.GetStopLabelOffset()[0],maprender.GetStopLabelOffset()[1] }).
                SetFontSize(maprender.GetStopLabelFontSize()).SetFontFamily("Verdana"s).SetData(stop->name);
            svg::Text text = substrate;
            substrate.SetFillColor(maprender.GetUnderlayerColor()).SetStrokeColor(maprender.GetUnderlayerColor()).
                SetStrokeWidth(maprender.GetUnderlayerWidth()).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            text.SetFillColor("black"s);
            doc.Add(std::move(substrate));
            doc.Add(std::move(text));
        }
    }
 }
