#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace Catalogue {
    namespace Input {
        namespace Detail {
            Geoposition::Coordinates ParseCoordinates(std::string_view str) {
                static const double nan = std::nan("");

                auto not_space = str.find_first_not_of(' ');
                auto comma = str.find(',');

                if (comma == str.npos) {
                    return {nan, nan};
                }

                auto not_space2 = str.find_first_not_of(' ', comma + 1);

                double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
                double lng = std::stod(std::string(str.substr(not_space2)));

                return {lat, lng};
            }

            std::string_view Trim(std::string_view string) {
                const auto start = string.find_first_not_of(' ');
                if (start == string.npos) {
                    return {};
                }
                return string.substr(start, string.find_last_not_of(' ') + 1 - start);
            }

            std::vector<std::string_view> Split(std::string_view string, char delim) {
                std::vector<std::string_view> result;

                size_t pos = 0;
                while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                    auto delim_pos = string.find(delim, pos);
                    if (delim_pos == string.npos) {
                        delim_pos = string.size();
                    }
                    if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                        result.push_back(substr);
                    }
                    pos = delim_pos + 1;
                }

                return result;
            }

            std::vector<std::string_view> ParseRoute(std::string_view route) {
                if (route.find('>') != route.npos) {
                    return Split(route, '>');
                }

                auto stops = Split(route, '-');
                std::vector<std::string_view> results(stops.begin(), stops.end());
                results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

                return results;
            }

            CommandDescription ParseCommandDescription(std::string_view line) {
                auto colon_pos = line.find(':');
                if (colon_pos == line.npos) {
                    return {};
                }

                auto space_pos = line.find(' ');
                if (space_pos >= colon_pos) {
                    return {};
                }

                auto not_space = line.find_first_not_of(' ', space_pos);
                if (not_space >= colon_pos) {
                    return {};
                }

                return {std::string(line.substr(0, space_pos)),
                        std::string(line.substr(not_space, colon_pos - not_space)),
                        std::string(line.substr(colon_pos + 1))};
            }

            std::vector <std::pair <std::string_view, int>> ParseRouteDistance(std::string_view line){
                std::vector<std::pair<std::string_view, int>> result;
                auto first_comma = line.find(',');
                auto second_comma = line.find(',', first_comma + 1);

                if((first_comma == std::string_view::npos) || (second_comma == std::string_view::npos)){
                    return result;
                }

                auto start = line.find_first_not_of(' ', second_comma + 1);

                while (start < line.size()) {
                    // Ищем "m to"
                    size_t pos_m_to = line.find("m to", start);
                    size_t next_comma = line.find(',', pos_m_to);
                    if (pos_m_to != std::string_view ::npos) {
                        // Извлекаем расстояние и местоположение
                        std::string_view distance = line.substr(start, pos_m_to - start);
                        auto name_start_pos = pos_m_to + 5;
                        std::string_view stop_name = line.substr(name_start_pos, next_comma - name_start_pos);
                        int dist = std::stoi(std::string (distance));
                        result.push_back(std::make_pair(stop_name, dist));
                    }
                    if(next_comma == std::string_view::npos){
                        break;
                    }
                    start = next_comma + 1;
                }

                return result;
            }
        }

        void InputReader::ReadCommands(size_t base_request_count, std::istream& input) {
            for (int i = 0; i < base_request_count; ++i) {
                std::string line;
                getline(input, line);
                ParseLine(line);
            }
        }
        void InputReader::ParseLine(std::string_view line) {
            auto command_description = Detail::ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }

        void InputReader::ApplyCommands([[maybe_unused]] Data::TransportCatalogue &catalogue) const {
            // Реализуйте метод самостоятельно
            using namespace std::literals;
            for (auto &command: commands_) {
                if (command.command == "Stop"s) {
                    Data::Stop new_stop;
                    new_stop.stop_name = command.id;
                    new_stop.pos = Detail::ParseCoordinates(command.description);
                    catalogue.Add_stop(std::move(new_stop));
                }
            }

            for (auto &command: commands_) {
                if (command.command == "Stop"s) {
                    auto stop_from = command.id;
                    for(auto& stop_distance : Detail::ParseRouteDistance(command.description)){
                        catalogue.Add_distance_between_stops(stop_from, stop_distance.first, stop_distance.second);
                    }
                }
            }

            for (auto &command: commands_) {
                if (command.command == "Bus"s) {
                    Data::Bus new_bus;
                    new_bus.bus_name = command.id;
                    for (auto &stop: Detail::ParseRoute(command.description)) {
                        new_bus.stops_on_route.push_back(catalogue.Find_stop(std::string_view(stop)));
                    }
                    catalogue.Add_bus(std::move(new_bus));
                }
            }
        }
    }
}