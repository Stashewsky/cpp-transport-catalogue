#include "svg.h"

namespace svg {

    using namespace std::literals;
    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& slc){
        switch (slc){
            case StrokeLineCap::BUTT: out << "butt"sv; break;
            case StrokeLineCap::ROUND: out << "round"sv; break;
            case StrokeLineCap::SQUARE: out << "square"sv; break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& slj){
        switch (slj){
            case StrokeLineJoin::ROUND: out << "round"sv; break;
            case StrokeLineJoin::ARCS: out << "arcs"sv; break;
            case StrokeLineJoin::BEVEL: out << "bevel"sv; break;
            case StrokeLineJoin::MITER: out << "miter"sv; break;
            case StrokeLineJoin::MITER_CLIP: out << "miter-clip"sv; break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

// ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

//--------------PolyLine----------------

    Polyline& Polyline::AddPoint(Point p){
        points_.push_back(p);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        //<polyline points="100,100 150,25 150,75 200,0" />
        out << "<polyline points=\""sv;
        for(size_t i = 0; i < points_.size(); i++){
            out << points_[i].x << ","sv << points_[i].y;
            if(i != points_.size() - 1){
                out << " "sv;
            }
        }
        out << "\"";
        RenderAttrs(out);
        out <<"/>"sv;

    }

//---------_Text--------------

    Text& Text::SetData(std::string data) {
        text_data_ = std::move(data);
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetOffset(svg::Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetPosition(svg::Point pos) {
        position_ = pos;
        return *this;
    }

    void Text::RenderObject(const svg::RenderContext &context) const {
        //  <text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">Hello C++</text>
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if(font_family_){
            out << " font-family=\""sv << *font_family_ << "\""sv;
        }
        if(font_weight_){
            out << " font-weight=\""sv << *font_weight_ << "\""sv;
        }
        out << ">"sv;
        /*Двойная кавычка " заменяется на &quot;. Точка с запятой в представлении этого и следующих спецсимволов — обязательная часть экранирующей последовательности.
        Одинарная кавычка или апостроф ' заменяется на &apos;.
        Символы < и > заменяются на &lt; и &gt; соответственно.
                Амперсанд & заменяется на &amp;.*/
        for(auto& c : text_data_){
            switch(c) {
                case '\"': out << "&quot;"sv; break;
                case '\'': out << "&apos;"sv; break;
                case '&': out << "&amp;"sv; break;
                case '<': out << "&lt;"sv; break;
                case '>': out << "&gt;"sv; break;
                default: out << c; break;
            }
        }
        out << "</text>"sv;
    }

//------------Document-----------

    void Document::AddPtr(std::unique_ptr<Object> &&object) {
        objects_.emplace_back(std::move(object));
    }

    void Document::Render(std::ostream &out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;

        RenderContext r_cont(out, 4, 4);
        for(auto& obj : objects_){
            obj->Render(r_cont);
        }
        out << "</svg>";
    }
}  // namespace svg