#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>


using namespace std::literals;
namespace svg
{
    class Rgb{
        public:
        Rgb()=default;
        Rgb(uint8_t r, uint8_t g, uint8_t b);
        uint8_t red=0;
        uint8_t green=0;
        uint8_t blue=0;
    };

    class Rgba:public Rgb
    {
        public:
            Rgba() = default;
            Rgba(uint8_t r, uint8_t g, uint8_t b, double a);
            double opacity=1;
    };

    inline const std::string NoneColor{"none"};
    using Color = std::variant<std::monostate,std::string,Rgb,Rgba>;
    
    struct ExtractColor{
        std::string operator()(std::monostate mst);
        std::string operator()(std::string str);
        std::string operator()(Rgb color);
        std::string operator()(Rgba color);
    };

    std::ostream &operator<<(std::ostream &out, const Color &color);

    enum class StrokeLineCap
    {
        BUTT,
        ROUND,
        SQUARE,
    };
    
    std::ostream &operator<<(std::ostream &out, const StrokeLineCap &line_cap);

    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &line_join);

    struct Point
    {
        Point() = default;
        Point(double x, double y)
            : x(x), y(y)
        {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext
    {
        RenderContext(std::ostream &out);
        RenderContext(std::ostream &out, int indent_step, int indent = 0);
        RenderContext Indented() const;
        void RenderIndent() const;
        std::ostream &out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object
    {
    public:
        void Render(const RenderContext &context) const;

        virtual ~Object() = default;

    protected:
        Object() = default;

    private:
        virtual void RenderObject(const RenderContext &context) const = 0;
    };

    // интeрфейс ObjectContainer
    class ObjectContainer
    {
    public:
        template <typename Obj>
        void Add(Obj object)
        {
            AddPtr(std::move(std::make_unique<Obj>(object)));
        }

        virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;
    };

    // интерфейс Drawable

    class Drawable
    {
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer &container) const = 0;
    };

    template <typename Obj>
    class PathProps
    {
    public:
        PathProps() = default;

        Obj &SetFillColor(Color color)
        {
            fill_color_ = std::move(color);
            return GetReference();
        }

        Obj &SetStrokeColor(Color color)
        {
            stroke_color_ = std::move(color);
            return GetReference();
        }

        Obj &SetStrokeWidth(double width)
        {
            stroke_width_ = width;
            return GetReference();
        }

        Obj &SetStrokeLineCap(StrokeLineCap line_cap)
        {
            line_cap_ = std::move(line_cap);
            return GetReference();
        }

        Obj &SetStrokeLineJoin(StrokeLineJoin line_join)
        {
            line_join_ = std::move(line_join);
            return GetReference();
        }

        void RenderAttrs(const RenderContext &context) const
        {
            using namespace std::literals;
            auto &out = context.out;
            if (fill_color_.has_value())
            {
                out << "fill=\""sv << fill_color_.value() << "\" "sv;
            }
            if (stroke_color_.has_value())
            {
                out << "stroke=\"" << stroke_color_.value() << "\" "sv;
            }
            if (stroke_width_.has_value())
            {
                out << "stroke-width=\"" << stroke_width_.value() << "\" "sv;
            }
            if (line_cap_.has_value())
            {
                out << "stroke-linecap=\"" << line_cap_.value() << "\" "sv;
            }
            if (line_join_.has_value())
            {
                out << "stroke-linejoin=\"" << line_join_.value() << "\""sv;
            }
        }

    protected:
        ~PathProps() = default;

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;

    private:
        Obj &GetReference()
        {
            return static_cast<Obj &>(*this);
        }
    };

    class Circle final : public Object, public PathProps<Circle>
    {
    public:
        Circle() = default;
        Circle &SetCenter(Point center);
        Circle &SetRadius(double radius);

    private:
        void RenderObject(const RenderContext &context) const override;
        Point center_ = {0, 0};
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline>
    {
    public:
        Polyline() = default;
        Polyline &AddPoint(Point point);

    private:
        void RenderObject(const RenderContext &context) const override;
        std::vector<Point> vertexes_ = {};
    };


    struct TextAttributes
    {
        Point position;
        Point offcet;
        uint32_t size;
        std::string font_family;
        std::string font_weight;
    };

    class Text final : public Object, public PathProps<Text>
    {
    public:
        Text() = default;
        Text &SetPosition(Point pos);
        Text &SetOffset(Point offset);
        Text &SetFontSize(uint32_t size);
        Text &SetFontFamily(std::string font_family);
        Text &SetFontWeight(std::string font_weight);
        Text &SetData(std::string data);

    private:
        void RenderObject(const RenderContext &context) const override;
        TextAttributes attr_ = {{0, 0},
                                {0, 0},
                                1,
                                "",
                                ""};
        std::string text_;

    };

    class Document : public ObjectContainer
    {
    public:
        Document() = default;

        void AddPtr(std::unique_ptr<Object> &&obj) override;

        void Render(std::ostream &out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;

    };

} // namespace svg