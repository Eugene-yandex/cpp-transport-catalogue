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
    std::vector<double> MapRenderer::GetBusLabelOffset() const {
        return mp_.bus_label_offset;
    }
    std::vector<double> MapRenderer::GetStopLabelOffset() const {
        return mp_.stop_label_offset;
    }
    svg::Color MapRenderer::GetUnderlayerColor() const {
        return mp_.underlayer_color;
    }
    std::vector<svg::Color> MapRenderer::GetColorPalette() const {
        return mp_.color_palette;
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point SphereProjector::operator()(Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }
}
