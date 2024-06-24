#pragma once
#include "transport_catalogue.h"

#include <optional>
#include <unordered_set>

 class RequestHandler {
 public:
     // MapRenderer понадобится в следующей части итогового проекта
     RequestHandler(const catalog::TransportCatalogue& db);

     std::optional<domain::BusInformation> GetBusInfo(std::string_view bus) const;
     const std::vector<std::string_view> GetStopInfo(std::string_view stop) const;

 private:
     // RequestHandler использует агрегацию объектов "Транспортный Справочник"
     const catalog::TransportCatalogue& db_;

 };
 