#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <iostream>


#include "geo.h"
#include "transport_catalogue.h"

namespace input {
    struct CommandDescription {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      // Название команды
        std::string id;           // id маршрута или остановки
        std::string description;  // Параметры команды
    };

    struct DescriptionCommandStop {
        std::string_view stop;
        Coordinates coordinates;
        std::unordered_map<std::string,int> distance;  
    };

    std::unordered_map<std::string, int> ParseDistance(std::string line);


    DescriptionCommandStop ParseCommandStop(std::string_view line);

    class Reader {
    public:

        /**
         * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
         */
        void ParseLine(std::string_view line);


        /**
         * Наполняет данными транспортный справочник, используя команды из commands_
         */
        void ApplyCommands(catalog::TransportCatalogue& catalogue) const;

    private:
        std::vector<CommandDescription> commands_;
    };

    void CreateDatabase(std::istream& in, catalog::TransportCatalogue& catalogue);
}
