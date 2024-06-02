#include "input_reader.h"


#include <algorithm>
#include <cassert>
#include <iterator>
#include <unordered_map>
#include <string>

namespace input {

     void CreateDatabase(std::istream& in, catalog::TransportCatalogue& catalogue) {

        int base_request_count = 0;
        in >> base_request_count >> std::ws;
        Reader reader;
        for (int i = 0; i < base_request_count; ++i) {
            std::string line;
            std::getline(in, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    /**
     * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
     */
    Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return { nan, nan };
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));

        return { lat, lng };
    }

    std::unordered_map<std::string, int> ParseDistance(std::string line) {
        std::unordered_map<std::string, int> result;

        auto not_space = line.find_first_not_of(' ');
        auto comma = line.find(',', not_space + 1);
        auto space = line.find_first_of(' ', not_space);
        for (int i = 0; not_space != line.npos; ++i) {
            if (i > 0) {
                not_space = line.find_first_not_of(' ', comma + 1);
                comma = line.find(',', not_space + 1);
                space = line.find_first_of(' ', not_space);
            }
            int distance = std::stoi(line.substr(not_space, space - 1));

            not_space = line.find_first_not_of(' ', space);
            space = line.find_first_of(' ', not_space);
            not_space = line.find_first_not_of(' ', space);
            result.insert({ line.substr(not_space, comma - not_space),distance });
            not_space = comma;
        }
        return result;
    }

    DescriptionCommandStop ParseCommandStop(std::string line) {
        DescriptionCommandStop result;
        auto comma = line.find(',');
        if (comma == line.npos) {
            return result;
        }

        auto comma2 = line.find(',', comma + 1);
        result.coordinates = ParseCoordinates(line.substr(0, comma2));
        if (comma2 == line.npos) {
            return result;
        }
        result.distance = ParseDistance(std::move(line.substr(comma2 + 1)));

        return result;
    }


    /**
     * Удаляет пробелы в начале и конце строки
     */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    /**
     * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
     */
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

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
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

        return { std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1)) };
    }

    void Reader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void Reader::ApplyCommands([[maybe_unused]] catalog::TransportCatalogue& catalogue) const {
        // Реализуйте метод самостоятельно
        using namespace std::literals;
        std::vector<DescriptionCommandStop> description_stops;
        for (auto& com : commands_) {
            if (com.command == "Bus"s) {
                continue;
            }
            DescriptionCommandStop description_stop = ParseCommandStop(std::move(com.description));
            catalogue.AddStop(com.id, description_stop.coordinates);
            description_stop.stop = com.id;
            description_stops.push_back(std::move(description_stop));
        }

        for (const auto& description_stop : description_stops) {
            catalogue.AddDistance(description_stop.stop, description_stop.distance);
        }

        for (const auto& com : commands_) {
            if (com.command == "Stop"s) {
                continue;
            }
            catalogue.AddBus(com.id, ParseRoute(com.description));
        }
    }
}
