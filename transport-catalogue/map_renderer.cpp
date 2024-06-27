#include "map_renderer.h"
namespace renderer {
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    MapRenderer::MapRenderer(MapParameters mp) :
        mp_(std::move(mp)) {
    }

    MapParameters MapRenderer::GetMapParameters() {
        return mp_;
    }

    double MapRenderer::GetWidth() const {
        return mp_.width;
    }
    double MapRenderer::GetHeight() const {
        return mp_.height;
    }
    double MapRenderer::GetPadding() const {
        return mp_.padding;
    }
    double MapRenderer::GetLine_width() const {
        return mp_.line_width;
    }
    double MapRenderer::GetStop_radius() const {
        return mp_.stop_radius;
    }
    int MapRenderer::GetBusLabelFontSize() const {
        return mp_.bus_label_font_size;
    }
    int MapRenderer::GetStopLabelFontSize() const {
        return mp_.stop_label_font_size;
    }
    double MapRenderer::GetUnderlayerWidth() const {
        return mp_.underlayer_width;
    }
    const std::vector<double>& MapRenderer::GetBusLabelOffset() const {
        return mp_.bus_label_offset;
    }
    const std::vector<double>& MapRenderer::GetStopLabelOffset() const {
        return mp_.stop_label_offset;
    }
    svg::Color MapRenderer::GetUnderlayerColor() const {
        return mp_.underlayer_color;
    }
    const std::vector<svg::Color>& MapRenderer::GetColorPalette() const {
        return mp_.color_palette;
    }
    void MapRenderer::RenderLine(const std::deque<domain::Bus*>& buses, const SphereProjector& proj, svg::Document& doc) const {
        int i = 0;
        int max_color_palette = GetColorPalette().size();
        for (const auto& bus : buses) {
            svg::Polyline result;
            if (i >= max_color_palette) {
                i = 0;
            }
            for (const auto& stop : bus->stops) {
                result.AddPoint(proj(stop->stop_coordinates));
            }
            result.SetStrokeColor(GetColorPalette()[i]).SetStrokeWidth(GetLine_width()).
                SetFillColor(svg::NoneColor).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            doc.Add(std::move(result));
            ++i;
        }

    }

    void MapRenderer::RenderBusLabels(const std::deque<domain::Bus*>& buses, const SphereProjector& proj, svg::Document& doc) const {
        using namespace std::literals;
        int i = 0;
        int max_color_palette = GetColorPalette().size();
        for (const auto& bus : buses) {
            svg::Text substrate;
            substrate.SetPosition(proj(bus->stops[0]->stop_coordinates)).SetOffset({ GetBusLabelOffset()[0],GetBusLabelOffset()[1] }).
                SetFontSize(GetBusLabelFontSize()).SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetData(bus->name);
            svg::Text text = substrate;
            substrate.SetFillColor(GetUnderlayerColor()).SetStrokeColor(GetUnderlayerColor()).
                SetStrokeWidth(GetUnderlayerWidth()).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            text.SetFillColor(GetColorPalette()[i % max_color_palette]);
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
    void MapRenderer::RenderStopPoints(const std::vector<domain::Stop*>& stops, const SphereProjector& proj, svg::Document& doc) const {
        using namespace std::literals;
        for (const auto& stop : stops) {
            svg::Circle circle;
            circle.SetCenter(proj(stop->stop_coordinates)).SetRadius(GetStop_radius()).SetFillColor("white"s);
            doc.Add(std::move(circle));
        }
    }

    void MapRenderer::RenderStopLabels(const std::vector<domain::Stop*>& stops, const SphereProjector& proj, svg::Document& doc) const {
        using namespace std::literals;
        for (const auto& stop : stops) {
            svg::Text substrate;
            substrate.SetPosition(proj(stop->stop_coordinates)).SetOffset({ GetStopLabelOffset()[0],GetStopLabelOffset()[1] }).
                SetFontSize(GetStopLabelFontSize()).SetFontFamily("Verdana"s).SetData(stop->name);
            svg::Text text = substrate;
            substrate.SetFillColor(GetUnderlayerColor()).SetStrokeColor(GetUnderlayerColor()).
                SetStrokeWidth(GetUnderlayerWidth()).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            text.SetFillColor("black"s);
            doc.Add(std::move(substrate));
            doc.Add(std::move(text));
        }
    }
    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point SphereProjector::operator()(Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }
}
