#include "json_reader.h"
#include "request_handler.h"


#include <algorithm>
#include <cassert>
#include <iterator>
#include <unordered_map>
#include <string>
#include <sstream>
#include "json_builder.h"

namespace jreader {
    using namespace std::literals;

    JSONReader::JSONReader(std::istream& input) {
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

    void JSONReader::CreateDatabase(catalog::TransportCatalogue& catalogue) {
        if (!base_requests_.empty()) {
            ApplyCommandsStop(catalogue);
            ApplyCommandsBus(catalogue);
        }
        
    }

    void JSONReader::ApplyCommandsStop([[maybe_unused]] catalog::TransportCatalogue& catalogue) {
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

    DescriptionCommandStop JSONReader::AddDescriptionCommandStop(const json::Dict& map_info) {
        DescriptionCommandStop result;
        result.stop = map_info.at("name").AsString();
        result.coordinates = { map_info.at("latitude").AsDouble(),map_info.at("longitude").AsDouble() };
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

    //понравилась идея перенести реализацию функции AddRoute в ApplyCommandsBus
    void JSONReader::ApplyCommandsBus([[maybe_unused]] catalog::TransportCatalogue& catalogue) {
        // Реализуйте метод самостоятельно

        for (const auto& data : base_requests_) {
            if (data.AsMap().at("type"s) == "Bus"s) {
                std::vector<std::string_view> result;
                std::optional<std::string_view> last_stop;
                const auto& stops = data.AsMap().at("stops"s).AsArray();
                if (!stops.empty()) {
                    for (const auto& stop : stops) {
                        result.push_back(stop.AsString());
                    }
                    if (!data.AsMap().at("is_roundtrip"s).AsBool()) {
                        if (*result.rbegin() != *result.begin()) {
                            last_stop = *result.rbegin();
                        }
                        result.insert(result.end(), std::next(result.rbegin()), result.rend());
                    }
                }
                catalogue.AddBus(data.AsMap().at("name").AsString(), result, last_stop);
            }
        }   
    }

    json::Node JSONReader::PrintBus(const catalog::TransportCatalogue& tansport_catalogue, int id, std::string_view name) {
        json::Builder bild{};
        bild.StartDict().Key("request_id"s).Value(id);
        auto bus_info = tansport_catalogue.GetBusInfo(name);
        if (!bus_info) {
            bild.Key("error_message"s).Value("not found"s);
        }
        else {
            bild.Key("curvature"s).Value(bus_info.value().route_length_real / bus_info.value().route_length_geographical_coordinates);
            bild.Key("route_length"s).Value(bus_info.value().route_length_real);
            bild.Key("stop_count"s).Value(bus_info.value().count_stops);
            bild.Key("unique_stop_count"s).Value(bus_info.value().unique_stops);
        }
        return bild.EndDict().Build();
    }

    json::Node JSONReader::PrintStop(const catalog::TransportCatalogue& tansport_catalogue,
        int id, std::string_view name) {
        json::Builder bild{};
        bild.StartDict().Key("request_id"s).Value(id);
        auto stops = tansport_catalogue.GetStopInfo(name);
        json::Array result_stop;
        if (stops.size() == 0) {
            bild.Key("error_message"s).Value("not found"s);
        }
        else if (stops.size() == 1 && stops[0] == "no buses"sv) {
            bild.Key("buses"s).Value(result_stop);

        }
        else {
            for (const auto& stop : stops) {
                result_stop.emplace_back(json::Node(std::string(stop)));
            }
            bild.Key("buses"s).Value(result_stop);

        }
        return bild.EndDict().Build();
    }

    renderer::MapRenderer JSONReader::MakeMapRenderer() const {
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

    svg::Color JSONReader::GetColorRenderSettings(const json::Node& node) const {
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

    void JSONReader::PrintStat(const catalog::TransportCatalogue& tansport_catalogue, std::ostream& output) {

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

    json::Node JSONReader::PrintRenderMap(const catalog::TransportCatalogue& tansport_catalogue, int id) const{
        json::Builder bild{};
        bild.StartDict().Key("request_id"s).Value(id);
        std::ostringstream out;
        svg::Document doc = RenderMap(tansport_catalogue);
        doc.Render(out);
        bild.Key("map"s).Value(out.str());
        return bild.EndDict().Build();
    }

    svg::Document JSONReader::RenderMap(const catalog::TransportCatalogue& tansport_catalogue) const {
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
        maprender.RenderLine(buses, proj, doc);
        maprender.RenderBusLabels(buses, proj, doc);
        std::sort(stops.begin(), stops.end(), [](domain::Stop* lhs, domain::Stop* rhs) {
            return lhs->name < rhs->name;
            });
        maprender.RenderStopPoints(stops, proj, doc);
        maprender.RenderStopLabels(stops, proj, doc);
        return doc;
    }

   
 }
