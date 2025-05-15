# Vecodex - Vector Search Cluster

Vecodex is a distributed vector search engine designed for high performance and scalability. It consists of several key components: an indexing library, a core application layer, and a Kubernetes operator for deployment and management.

## Project Structure

The project is organized into the following main directories:

*   **`app/`**: Contains the core application logic, including the coordinator, searcher, and writer services. See [app/README.md](app/README.md) for more details on its components and how to run them.
*   **`index/`**: Houses the vector search index library. This library can be used as a standalone component in other C++ projects. See [index/README.md](index/README.md) for usage and build instructions.
*   **`kube-operator/`**: Includes the Kubernetes operator for deploying and managing Vecodex clusters on Kubernetes. See [kube-operator/README.md](kube-operator/README.md).

## Getting Started

### Prerequisites

*   CMake
*   C++ compiler (supporting C++17)
*   Docker (for containerized build and deployment)
*   Git

### Building the Project

The project includes top-level build scripts:

*   `build.sh`: For production builds.
*   `build-dev.sh`: For development builds.

To build the entire application:
```bash
./build.sh
# or for development
./build-dev.sh
```

### Building and Running the Application (`app/`)

The `app/` directory contains the main Vecodex services.

**Build:**
Navigate to the `app/` directory if you want to build it separately (though the top-level scripts should handle this).
```bash
cd app
./build.sh
```

**Running Services:**

*   **Coordinator:**
    ```bash
    ./build/vecodex-app coordinator --listening-port 44400
    ```
*   **Example Client (to send requests to the coordinator):**
    ```bash
    ./build/example_client 44400
    ```

### Using the Index Library (`index/`)

The `index/` library provides vector indexing capabilities.

1.  **Initialize and update Git submodules:**
    ```bash
    git submodule init
    git submodule update
    ```

2.  **Build the library:**
    ```bash
    cd index
    cmake -B build .
    # To use it as a dependency in your CMake project, add:
    # add_subdirectory(path/to/vecodex/index)
    ```
    Refer to [index/README.md](index/README.md) for more details.


## Docker

Docker can be used for building and running the application components in a containerized environment.

**Build dev container:**
```bash
docker build -f Dockerfile_dev -t app-dev:latest .
```

**Run dev container:**
This mounts the local `./app` and `./index` directories into the container.
```bash
docker run -it --name docker-dev --network host -v ./app:/app -v ./index:/index app-dev:latest
```

**Run Coordinator example client inside the dev container:**
First, start the dev container as shown above. Then, inside the container:
```bash
make example_client && ./example_client <port>
```

**External Dependencies (Running with Docker):**

*   **etcd:**
    ```bash
    docker run -e ALLOW_NONE_AUTHENTICATION=yes -e ETCD_ADVERTISE_CLIENT_URLS=http://etcd:2379 --network host bitnami/etcd:latest
    ```
*   **MinIO:**
    ```bash
    docker run --network host -e MINIO_ROOT_USER=user -e MINIO_ROOT_PASSWORD=password -v ~/minio/data:/data minio/minio:latest server /data --console-address ":9001"
    ```

## Kubernetes Operator

The `kube-operator/` directory contains tools and configurations for deploying Vecodex on Kubernetes. Please refer to the [kube-operator/README.md](kube-operator/README.md) for specific instructions.

## Contribution Guidelines

We welcome contributions to Vecodex! Before contributing, please:

1.  Create an issue to discuss your proposed changes.
2.  Fork the repository and work on your feature or fix in a separate branch.
3.  Submit a pull request (PR) linked to the issue.

Please review the [Contributing Guidelines](CONTRIBUTING.md) and ensure your changes adhere to the project's code style and include necessary tests and documentation. Thank you for contributing!

## License

Vecodex is licensed under the [LICENSE](LICENSE) file in the root directory of this source tree.
