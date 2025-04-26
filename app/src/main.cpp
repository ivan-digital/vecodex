#include <string>

#include "BootstrapConfig.h"
#include "Coordinator.h"
#include "Searcher.h"
#include "Writer.h"

int main(int argc, char* argv[]) {
    json config = BootstrapConfig(argc, argv).GetConfig();
    auto mode = config["mode"].template get<std::string>();

    if (mode == "searcher") {
        Searcher server = Searcher(config);
        server.Run();
    }
    if (mode == "writer") {
        Writer server = Writer(config);
        server.Run();
    }
    if (mode == "coordinator") {
        Coordinator server = Coordinator(config);
        server.Run();
    }

    throw std::invalid_argument("Invalid mode: " + mode);
    return 0;
}
