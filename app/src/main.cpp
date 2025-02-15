#include <stdio.h>
#include <string>
#include <unordered_map>

#include "Coordinator.h"
#include "Searcher.h"
#include "BootstrapConfig.h"

int main(int argc, char* argv[]) {
    json config = BootstrapConfig(argc, argv).GetConfig();
    auto mode = config["mode"].template get<std::string>();

    if (mode == "searcher") {
        Searcher server = Searcher(config);
        server.Run();
        // TODO
    }
    if (mode == "writer") {
        // TODO
    }
    if (mode == "coordinator") {
        Coordinator server = Coordinator(config);
        server.Run();
        // TODO
    }

    throw std::invalid_argument("Invalid mode: " + mode);
    return 0;
}
