#include "stat_reader.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
namespace Catalogue {
    namespace Output {
        void ParseAndPrintStat(const Data::TransportCatalogue &transport_catalogue, std::string_view request,
                               std::ostream &output) {
            // Реализуйте самостоятельно
            using namespace std::literals;

            auto space_pos = request.find_first_of(" ");
            if (space_pos == request.npos) {
                return;
            }

            std::string_view command = Detail::TrimSpaces(request.substr(0, space_pos));
            std::string_view description = Detail::TrimSpaces(request.substr(space_pos + 1));
            if (command == "Bus") {
                auto bus = transport_catalogue.Find_bus(description);
                output << command << " "s << description << ": "s;

                if (!bus) {
                    output << "not found"s << std::endl;
                    return;
                }

                output << std::setprecision(2) << std::fixed <<
                       bus->stops_on_route.size() << " stops on route, "s <<
                       Detail::CountUniqueStops(bus->stops_on_route) << " unique stops, "s <<
                       Detail::ComputeRouteLength(bus->stops_on_route) << " route length"s << std::endl;

            }

            if (command == "Stop") {
                output << command << " "s << description << ": "s;

                if (!transport_catalogue.Find_stop(description)) {
                    output << "not found"s << std::endl;
                } else if (transport_catalogue.Find_buses_for_stop(description).empty()) {
                    output << "no buses"s << std::endl;
                } else {
                    output << "buses"s;
                    for (auto bus: transport_catalogue.Find_buses_for_stop(description)) {
                        output << " "s << bus;
                    }
                    output << std::endl;
                }

            }

        }

        namespace Detail {
            std::string_view TrimSpaces(std::string_view string) {
                const auto start = string.find_first_not_of(' ');
                if (start == string.npos) {
                    return {};
                }
                return string.substr(start, string.find_last_not_of(' ') + 1 - start);
            }

            //не могу придумать более оптимальную реализацию
            //или считать уникальные остановки исходя из того, кольцевой ли маршрут?
            //добавить поле в структуру автобуса
            //т.е. если кольцевой - количество остановок -1
            //туда-обратно маршрут - (количество остановок/2) +1
            size_t CountUniqueStops(const std::vector<Data::Stop *> &vector) {
                auto vec = vector;
                std::sort(vec.begin(), vec.end());
                auto last = std::unique(vec.begin(), vec.end());
                vec.erase(last, vec.end());
                return vec.size();
            }

            double ComputeRouteLength(const std::vector<Data::Stop *> &route) {
                double res = 0;
                for (int i = 0; i < route.size() - 1; i++) {
                    res += ComputeDistance(route[i]->pos, route[i + 1]->pos);
                }
                return res;
            }
        }
    }
}