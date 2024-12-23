
#include <argparse/argparse.hpp>
#include <stdio.h>
#include <string>
#include <unordered_map>

#include "coordinator.h"
#include "Writer.h"


int main(int argc, char* argv[]) {
    
    argparse::ArgumentParser program("vecodex-app");
    program.add_argument("mode")
        .help("...")
        .choices("searcher", "writer", "coordinator");
    program.add_argument("-d", "--daemon")
        .help("...")
        .flag()
        .default_value(false);
    program.add_argument("--listening-port")
        .help("...")
        .default_value("44400");
    program.add_argument("--output-port")
        .help("...")
        .default_value("44401");
    program.add_argument("--zookeeper-host")
        .help("...");
    program.add_argument("--s3-host")
        .help("...");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    const std::string program_mode = program.get("mode");
    const std::string output_port = program.get("--output-port");
    const std::string listening_port = program.get("--listening-port");

    if (program_mode == "searcher") {
        // TODO
    }
    if (program_mode == "writer") {
        const std::string s3_host = program.get("--s3-host");
        Writer server = AppWriter("localhost", output_port, s3_host);
        server.Run();
        // TODO
    }
    if (program_mode == "coordinator") {
        Coordinator server = Coordinator("localhost", listening_port);
        server.Run();
        // TODO
    }

    throw std::invalid_argument("Invalid mode: " + program_mode);
    return 0;
}
