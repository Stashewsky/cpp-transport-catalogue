#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace Catalogue;

int main() {
    Data::TransportCatalogue catalogue;

    int base_request_count;
    std::cin >> base_request_count >> std::ws;

    {
        Input::InputReader reader;
        reader.ReadCommands(base_request_count, std::cin);
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    std::cin >> stat_request_count >> std::ws;
    Output::ReadAndPrintStatRequest(stat_request_count, std::cin, std::cout, catalogue);
}