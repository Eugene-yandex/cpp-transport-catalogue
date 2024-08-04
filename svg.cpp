#include "svg.h"

namespace svg {

    using namespace std::literals;

    void ColorPrinter::operator()(std::monostate) const {
        out << "none"s;
    }
    void ColorPrinter::operator()(const std::string& color) const {
        out << color;
    }
    void ColorPrinter::operator()(Rgb color) const {
        out << "rgb("s << static_cast<int>(color.red) << ',' << static_cast<int>(color.green) << ',' << static_cast<int>(color.blue) << ')';
    }
    void ColorPrinter::operator()(Rgba color) const {
        out << "rgba("s << static_cast<int>(color.red) << ',' << static_cast<int>(color.green)
            << ',' << static_cast<int>(color.blue) << ',' << color.opacity << ')';
    }

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        visit(ColorPrinter{ out }, color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap) {
        switch (stroke_line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join) {
        switch (stroke_line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
        return out;
    }
    void Point::SetPoint(double first, double second) {
        x = first;
        y = second;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------
    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool nofirst = false;
        for (const auto& point : points_) {
            if (nofirst) {
                out << ' ';
            }
            out << point.x << ',' << point.y;
            nofirst = true;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;

    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y;
        out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\""sv;
        }
        out << '>';
        for (auto& ch : data_) {
            if (ch == '\"') {
                out << "&quot"sv;
                continue;
            }
            if (ch == '\'') {
                out << "&apos;"sv;
                continue;
            }
            if (ch == '<') {
                out << "&lt;"sv;
                continue;
            }
            if (ch == '>') {
                out << "&gt;"sv;
                continue;
            }
            if (ch == '&') {
                out << "&amp;"sv;
                continue;
            }
            out << ch;
        }
        out << "</text>"sv;
    }

    //--------------------------------------------------
    void Document::RenderFirstLine(std::ostream& out) const {
        out << "<\?xml version=\"1.0\" encoding=\"UTF-8\" \?>"sv << std::endl;
    }

    void Document::RenderSecondLine(std::ostream& out) const {
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::RenderEndLine(std::ostream& out) const {
        out << "</svg>"sv;
    }


    void Document::Render(std::ostream& out) const {
        RenderFirstLine(out);
        RenderSecondLine(out);
        for (const auto& obj : objects_) {
            obj->Render({ out,2,2 });
        }
        RenderEndLine(out);
    }





}  // namespace svg