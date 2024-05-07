#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <vector>
#include <optional>
#include <variant>
namespace svg {
    using namespace std::literals;
    struct Rgb{
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b){
            red  = r;
            green = g;
            blue = b;
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba{
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double o){
            red = r;
            green = g;
            blue = b;
            if (o < 0.0 || o > 1) {
                throw std::invalid_argument("Opacity must be in the range [0.0, 1.0]");
            }
            opacity = o;
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{};

    struct GetColor{
        std::ostream& out;

        void operator()(const std::monostate&)const{
            out << "none"sv;
        }
        void operator()(const std::string& color)const{
            out << color;
        }

        void operator()(const Rgb& color)const{
            out << "rgb("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ")"sv;
        }
        void operator()(const Rgba& color)const{
            out << "rgba("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ","sv << color.opacity <<")"sv;
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Color& color) {
        std::visit(GetColor{os}, color);
        return os;
    }

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };
    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& slc);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& slj);

    struct Point {
        Point() = default;
        Point(double x, double y)
                : x(x)
                , y(y) {
        }
        Point(std::pair<double, double> p) :
        x(p.first),
        y(p.second){
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
                : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
                : out(out)
                , indent_step(indent_step)
                , indent(indent) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    template <typename Owner>
    class PathProps{
    public:
        Owner& SetFillColor(Color color){
            fill_color_ = color;
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color){
            stroke_color_ = color;
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width){
            stroke_width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap slc){
            stroke_line_cap_ = slc;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin slj){
            stroke_line_join_ = slj;
            return AsOwner();
        }
    protected:
        ~PathProps() = default;
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (!std::holds_alternative<std::monostate>(fill_color_)) {
                out << " fill=\""sv << fill_color_ << "\""sv;
            }
            if (!std::holds_alternative<std::monostate>(stroke_color_)) {
                out << " stroke=\""sv << stroke_color_ << "\""sv;
            }
            if(stroke_width_){
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if(stroke_line_cap_){
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if(stroke_line_join_){
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }
    private:
        Owner& AsOwner(){
            return static_cast<Owner&>(*this);
        }

        Color fill_color_;
        Color stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class Circle final : public Object, public PathProps<Circle>{
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline : public Object, public PathProps<Polyline>{
    public:
        Polyline& AddPoint(Point point);
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    class Text : public Object, public PathProps<Text>{
    public:
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);
    private:
        Point position_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::optional<std::string> font_family_;
        std::optional<std::string> font_weight_;
        std::string text_data_;

        void RenderObject(const RenderContext& context) const override;
    };

    class ObjectContainer {
    public:
        template<typename T>
        void Add(T obj) {
            AddPtr(std::make_unique<T>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object> &&object) = 0;
        virtual ~ObjectContainer() = default;
    };

    class Drawable{
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };


    class Document : public ObjectContainer{
    public:
        void AddPtr(std::unique_ptr<Object> &&object) override;
        void Render(std::ostream& out) const;
    private:
        std::deque<std::unique_ptr<Object>> objects_;
    };
}