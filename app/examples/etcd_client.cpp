#include <iostream>
#include <etcd/Client.hpp>

int main(int argc, char* argv[]) {
    // Create an etcd client
    etcd::Client etcd("http://[::]:2379");

    // Set a key-value pair
    etcd.set("my_key", "my_value");

    // Get the value of a key
    auto task = etcd.get("my_key");
    if (task.wait()) {
        auto response = task.get();
        if (response.is_ok()) {
            std::cout << "Value of my_key: " << response.value().as_string() << std::endl;
        } else {
            std::cerr << "Failed to get value of my_key: " << response.error_message() << std::endl;
        }
    }

    return 0;
}