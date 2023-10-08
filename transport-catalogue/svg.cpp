#include "svg.h"

namespace svg
{

    using namespace std::literals;

    Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r), green(g), blue(b)
    {
    }
    Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
        : Rgb(r, g, b), opacity(a)
    {
    }
    std::string ExtractColor::operator()(std::monostate mst)
    {
        (void)mst;
        return "none"s;
    }
    std::string ExtractColor::operator()(std::string str)
    {
        return str;
    }
    std::string ExtractColor::operator()(Rgb color)
    {
        return "rgb("s + std::to_string(color.red) + ","s + std::to_string(color.green) + ","s + std::to_string(color.blue) + ")"s;
    }
    std::string ExtractColor::operator()(Rgba color)
    {
        std::string alpha = color.opacity == 0 ? "0"s : std::to_string(color.opacity).substr(0, 3);
        return "rgba("s + std::to_string(color.red) + ","s + std::to_string(color.green) + ","s + std::to_string(color.blue) + ","s + alpha +")"s;
    }

    std::ostream &operator<<(std::ostream &out, const Color &color)
    {
        out << std::visit(ExtractColor{}, color);
        return out;
    }

    void Object::Render(const RenderContext &context) const
    {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    //-----------output operator overload--------

    std::ostream &operator<<(std::ostream &out, const StrokeLineCap &line_cap)
    {
        using namespace std::literals;
        switch (line_cap)
        {
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

    std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &line_join)
    {
        using namespace std::literals;
        switch (line_join)
        {
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

    // ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<circle "sv;
        RenderAttrs(context);
        out << "cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\"/>"sv;
    }

    //-------------Polyline-------------

    Polyline &Polyline::AddPoint(Point point)
    {
        vertexes_.push_back(point);
        return *this;
    }
    void Polyline::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<polyline "sv;
        RenderAttrs(context);
        out << "points=\""sv;
        bool begin = true;
        for (const Point &point : vertexes_)
        {
            if (begin)
            {
                begin = false;
            }
            else
            {
                out << " "sv;
            }
            out << point.x << ","sv << point.y;
        }
        out << "\" />"sv;
    }

    //-------------------Text--------------

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text &Text::SetPosition(Point pos)
    {
        attr_.position = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text &Text::SetOffset(Point offset)
    {
        attr_.offcet = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text &Text::SetFontSize(uint32_t size)
    {
        attr_.size = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text &Text::SetFontFamily(std::string font_family)
    {
        attr_.font_family = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text &Text::SetFontWeight(std::string font_weight)
    {
        attr_.font_weight = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text &Text::SetData(std::string data)
    {
        std::string shielded = "";
        for (char ch : data)
        {
            switch (ch)
            {
            case '&':
                shielded += "&amp";
                continue;
            case '"':
                shielded += "&quot";
                continue;
            case '<':
                shielded += "&lt";
                continue;
            case '>':
                shielded += "&gt";
                continue;
            case '\'':
                shielded += "&apos";
                continue;
            default:
                shielded += ch;
            }
        }

        text_ = std::move(shielded);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<text ";
        RenderAttrs(context);
        out << "x=\""sv << attr_.position.x << "\" "sv;
        out << "y=\"" << attr_.position.y << "\" "sv;
        out << "dx=\"" << attr_.offcet.x << "\" "sv;
        out << "dy=\"" << attr_.offcet.y << "\" "sv;
        out << "font-size=\""sv << attr_.size << "\""sv;
        if (attr_.font_family != "")
        {
            out << " font-family=\""sv << attr_.font_family << "\""sv;
        }
        if (attr_.font_weight != "")
        {
            out << " font-weight=\""sv << attr_.font_weight << "\""sv;
        }
        out << ">"sv << text_ << "</text> "sv;
    }

    //--------------Document class------------

    // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object> &&obj)
    {
        objects_.push_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream &out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        int object_count = objects_.size();
        for (int idx = 0; idx < object_count; idx++)
        {
            objects_[idx].get()->Render({out});
        }
        out << "</svg>"sv;
    }

} // namespace svg