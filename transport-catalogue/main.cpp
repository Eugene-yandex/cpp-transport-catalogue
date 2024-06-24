#include <iostream>
#include <fstream>
#include <string>

#include "json_reader.h"

using namespace std;

int main() {
    catalog::TransportCatalogue catalogue;
    jreader::JsonInformation ji(cin);
    ji.CreateDatabase(catalogue);
    ji.PrintStat(catalogue, cout);
}