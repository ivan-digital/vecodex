#include <argparse/argparse.hpp>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

class BootstrapConfig {
public:
    BootstrapConfig(int argc, char* argv[]) {
        program_.add_argument("--config-path")
            .default_value("");
        program_.add_argument("--mode")
            .help("...")
            .choices("searcher", "writer", "coordinator", "")
            .default_value("");
        program_.add_argument("--hostname")
            .help("...")
            .default_value("localhost");
        program_.add_argument("--listening-port")
            .help("...")
            .default_value("");
        program_.add_argument("--output-port")
            .help("...")
            .default_value("");
        program_.add_argument("--s3-host")
            .help("...")
            .default_value("");
        program_.add_argument("--etcd-address")
            .help("...")
            .default_value("");
        program_.add_argument("--indexes")
            .help("indexes and shards info")
            .default_value("");

        Parse_(argc, argv);
    }

    json GetConfig() {
        return result_; 
    }

private:
    void Parse_(int argc, char* argv[]) {
        try {
            program_.parse_args(argc, argv);
        } catch (const std::exception& err) {
            std::cerr << err.what() << std::endl;
            std::cerr << program_;
            std::exit(1);
        }
        const std::string mode = program_.get("mode");
        const std::string config_path = program_.get("--config-path");
        const std::string output_port = program_.get("--output-port");
        const std::string listening_port = program_.get("--listening-port");
        const std::string s3_host = program_.get("--s3-host");
        const std::string etcd_addr = program_.get("--etcd-address");
        const std::string hostname = program_.get("--hostname");
        const std::string indexes = program_.get("--indexes");

        result_ = ReadJsonConfig_(config_path);

        result_["mode"] = mode == "" ? result_["mode"].template get<std::string>() : mode;
        result_["hostname"] = hostname == "" ? result_["hostname"].template get<std::string>() : hostname;
        result_["port"] = listening_port == "" ? result_["port"].template get<std::string>() : listening_port;
        result_["s3-host"] = s3_host == "" ? result_["s3-host"].template get<std::string>() : s3_host;
        result_["etcd_address"] = etcd_addr == "" ? result_["etcd_address"].template get<std::string>() : etcd_addr;
        result_["indexes"] = indexes == "" ? result_["indexes"] : json::parse(indexes);

        std::cout << "CONFIG:\n" << result_ << "\n";
    }

    json ReadJsonConfig_(const std::string& config_path) {
        json data;
        if (config_path == "") {
            return data;
        }
        try {
            std::ifstream file(config_path);
            data = json::parse(file);
            file.close();
        } catch (const json::type_error& e) {
            std::cout << "Error while handling json: " << e.what() << std::endl;
            throw e;
        }
        return data;
    }

    argparse::ArgumentParser program_{"vecodex-app"};
    json result_;
};