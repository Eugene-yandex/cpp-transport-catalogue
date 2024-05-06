#include "stat_reader.h"

#include <vector>
#include <iomanip>

std::pair<std::string_view, std::string_view> ParseStatReader(std::string_view line) {
    auto start_first_word = line.find_first_not_of(' ');
    auto end_first_word = line.find_first_of(' ', start_first_word);
    auto start_second_word = line.find_first_not_of(' ', end_first_word);
    return { line.substr(start_first_word,end_first_word - start_first_word),line.substr(start_second_word) };
}

void PrintStat(const catalog::TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output) {
    using namespace std::literals;
    // Реализуйте самостоятельно
    std::pair<std::string_view, std::string_view> words_request = ParseStatReader(request);

    if (words_request.first == "Bus"s) {
        std::vector<double> result = tansport_catalogue.BusInfo(words_request.second);
        output << "Bus "s << std::string(words_request.second) << ": "s;
        if (result.empty()) {
            output << "not found"s << std::endl;
        }
        else {
            output << static_cast<int>(result[0]) << " stops on route, "s << static_cast<int>(result[1]) <<
                " unique stops, "s << std::setprecision(6) << result[2] << " route length"s << std::endl;
        }
    }
    else {
        std::vector<std::string> result = tansport_catalogue.StopInfo(words_request.second);
        output << "Stop "s << std::string(words_request.second) << ": "s;
        if (result.empty()) {
            output << "not found"s << std::endl;
        }
        else if (result.size() == 1 && result.back() == "no buses"s) {
            output << "no buses"s << std::endl;
        }
        else {
            output << "buses"s;
            for (auto bus : result) {
                output << ' ' << std::string(bus);
            }
            output << std::endl;
        }
    }
}
