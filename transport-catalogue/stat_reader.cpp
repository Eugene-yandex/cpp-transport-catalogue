#include "stat_reader.h"

#include <vector>
#include <iomanip>
#include <iostream>

namespace out {
    void GetInformation(std::istream& in, std::ostream& out, const catalog::TransportCatalogue& tansport_catalogue) {
        int stat_request_count;
        in >> stat_request_count >> std::ws;
        for (int i = 0; i < stat_request_count; ++i) {
            std::string line;
            std::getline(in, line);
            PrintStat(tansport_catalogue, line, out);
        }
    }

    CommandDescription ParseStatCommand(std::string_view line) {
        auto start_first_word = line.find_first_not_of(' ');
        auto end_first_word = line.find_first_of(' ', start_first_word);
        auto start_second_word = line.find_first_not_of(' ', end_first_word);
        return { line.substr(start_first_word,end_first_word - start_first_word),line.substr(start_second_word) };
    }

    void PrintBus(const catalog::TransportCatalogue& tansport_catalogue, std::string_view id,
        std::ostream& output) {
        using namespace std::literals;
        auto result = tansport_catalogue.GetBusInfo(id);
        output << "Bus "s << std::string(id) << ": "s;
        if (result) {
            output << "not found"s << std::endl;
        }
        else {
            output << result.count_stops << " stops on route, "s << result.unique_stops <<
                " unique stops, "s << std::setprecision(6) << result.route_length_real << " route length, "s << std::defaultfloat <<
                result.route_length_real / result.route_length_geographical_coordinates << " curvature"s << std::endl;
        }
    }

    void PrintStop(const catalog::TransportCatalogue& tansport_catalogue, std::string_view id,
        std::ostream& output) {
        using namespace std::literals;
        std::vector<std::string_view> result = tansport_catalogue.GetStopInfo(id);
        output << "Stop "s << std::string(id) << ": "s;
        if (result.empty()) {
            output << "not found"s << std::endl;
        }
        else if (result.size() == 1 && result.back() == "no buses"sv) {
            output << result.back() << std::endl;
        }
        else {
            output << "buses"s;
            for (auto bus : result) {
                output << ' ' << std::string(bus);
            }
            output << std::endl;
        }
    }

    void PrintStat(const catalog::TransportCatalogue& tansport_catalogue, std::string_view request,
        std::ostream& output) {
        using namespace std::literals;
        // Реализуйте самостоятельно
        CommandDescription words_request = ParseStatCommand(request);

        if (words_request.command == "Bus"s) {
            PrintBus(tansport_catalogue, words_request.id, output);
        }
        else {
            PrintStop(tansport_catalogue, words_request.id, output);
        }
    }
}
