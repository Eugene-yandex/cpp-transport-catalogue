#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    catalog::TransportCatalogue catalogue;
    input::CreateDatabase(cin, catalogue);
    out::GetInformation(cin, catalogue);
}
