#pragma once
#include "svg.h"
#include "geo.h"
#include <map>
#include "domain.h"
#include <algorithm>
namespace map_renderer{
    struct RenderSettings {
        double width_;
        double height_;

        double padding_;

        double line_width_;
        double stop_radius_;

        int bus_label_font_size_;
        std::pair<double, double> bus_label_offset_;

        int stop_label_font_size_;
        std::pair<double, double> stop_label_offset_;

        svg::Color underlayer_color_;
        double underlayer_width_;
        std::vector<svg::Color> color_palette_;
    };

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
                : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(catalogue::geoposition::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRenderer {
    public:
        MapRenderer(RenderSettings& settings) : render_settings_(settings){}
        svg::Document Render_map(const std::vector<std::pair<domain::Bus*, std::vector<domain::Stop*>>>& data);
    private:
        struct CompareStopPointers {
            bool operator()(const domain::Stop* lhs, const domain::Stop* rhs) const {
                return lhs->stop_name < rhs->stop_name; // Пример сортировки по полю id
            }
        };

        RenderSettings render_settings_;
        void Set_line_properties(svg::Polyline& line, int line_num);
        SphereProjector Get_projector(std::vector<catalogue::geoposition::Coordinates> stop_coordinates);
        svg::Color Get_color_for_this_line(int line_num);
        void Add_lines(SphereProjector& projector, const std::vector<std::pair<domain::Bus*, std::vector<domain::Stop*>>>& data, svg::Document& doc);
        void Add_route_labels(SphereProjector& projector, const std::vector<std::pair<domain::Bus*, std::vector<domain::Stop*>>>& data, svg::Document& doc);
        void Add_stop_circles(SphereProjector& projector, const std::set<domain::Stop*, CompareStopPointers>& data, svg::Document& doc);
        void Add_stop_labels(SphereProjector& projector, const std::set<domain::Stop*, CompareStopPointers>& data, svg::Document& doc);
    };
}