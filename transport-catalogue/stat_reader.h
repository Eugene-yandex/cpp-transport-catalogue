#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"
namespace out {
    struct CommandDescription {
        std::string_view command;      // Название команды
        std::string_view id;           // id маршрута или остановки
    };
    CommandDescription ParseStatCommand(std::string_view line);

    void PrintStat(const catalog::TransportCatalogue& tansport_catalogue, std::string_view request,
        std::ostream& output);

    void PrintBus(const catalog::TransportCatalogue& tansport_catalogue, std::string_view id,
        std::ostream& output);

    void PrintStop(const catalog::TransportCatalogue& tansport_catalogue, std::string_view id,
        std::ostream& output);

    void GetInformation(std::istream& in, const catalog::TransportCatalogue& tansport_catalogue);
}
