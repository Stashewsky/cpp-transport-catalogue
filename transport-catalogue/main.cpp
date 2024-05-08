#include <iostream>
#include "json_reader.h"
#include "request_handler.h"
using namespace std;
using namespace catalogue;
using namespace detail;
using namespace detail::json;
using namespace catalogue::data;
using namespace map_renderer;

int main() {
    TransportCatalogue catalogue;
    RequestHandler handler(catalogue);
    JSON_Reader reader;

    Document doc_in;
    doc_in = Load(std::cin);
    reader.ParseJSONdocument(doc_in);
    reader.ExecuteBaseRequests(catalogue);
    Document doc_out = reader.ExecuteStatRequests(handler);
    Print(doc_out, std::cout);
    return 0;
}