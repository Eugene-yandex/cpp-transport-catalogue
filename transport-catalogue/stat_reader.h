#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"
std::pair<std::string_view, std::string_view> ParseStatReader(std::string_view line);

void PrintStat(const catalog::TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output);
