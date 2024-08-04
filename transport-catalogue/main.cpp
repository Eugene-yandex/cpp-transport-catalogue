#include <iostream>
#include <string>

#include "json_reader.h"

using namespace std;

int main() {
    catalog::TransportCatalogue catalogue;
    jreader::JSONReader ji(cin);
    ji.CreateDatabase(catalogue);
    ji.PrintStat(catalogue, cout);
}