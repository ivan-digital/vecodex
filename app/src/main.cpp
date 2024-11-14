
#include <argparse/argparse.hpp>
#include <stdio.h>
#include <string>
#include <unordered_map>


namespace {
    enum VecodexAppMode {
        Searcher,
        Writer,
        Coordinator
    };
    const std::unordered_map<std::string, VecodexAppMode> str2mode = {
        {"searcher", VecodexAppMode::Searcher},
        {"writer", VecodexAppMode::Writer},
        {"coordinator", VecodexAppMode::Coordinator}
    };
    const std::unordered_map<VecodexAppMode, std::string> mode2str = {
        {VecodexAppMode::Searcher, "searcher"},
        {VecodexAppMode::Writer, "writer"},
        {VecodexAppMode::Coordinator, "coordinator"}
    };
}


int main(int argc, char* argv[]) {
    
    argparse::ArgumentParser program("vecodex-app");
    program.add_argument("mode")
        .help("...")
        .choices(
            mode2str.find(VecodexAppMode::Searcher)->second, 
            mode2str.find(VecodexAppMode::Writer)->second, 
            mode2str.find(VecodexAppMode::Coordinator)->second
        )
        .action([](const std::string& value) {
            return str2mode.find(value)->second;
        });
    program.add_argument("-d", "--daemon")
        .help("...")
        .flag()
        .default_value(false);
    program.add_argument("--listening-port")
        .help("...")
        .scan<'u', unsigned int>();
    program.add_argument("--output-port")
        .help("...")
        .scan<'u', unsigned int>();
    program.add_argument("--zookeeper-host")
        .help("...");
    program.add_argument("--s3-host")
        .help("...");
    program.add_argument("--zookeeper-host")
        .help("...");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    const VecodexAppMode program_mode = program.get<VecodexAppMode>("mode");
    switch (program_mode)
    {
    case VecodexAppMode::Searcher:
        // TODO
        break;
    case VecodexAppMode::Writer:
        // TODO
        break;
    case VecodexAppMode::Coordinator:
        // TODO 
        break;
    default:
        break;
    }

    return 0;
}
