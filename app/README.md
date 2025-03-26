`./build.sh` for building

`./build/vecodex-app coordinator --listening-port 44400` for running coordinator server

`./build/example_client 44400` for sending request to coordinator

---

**To build dev container use:**

`docker build -f Dockerfile_dev -t app-dev:latest .`

**To run dev container use:**

`docker run -it --name docker-dev --network host -v ./app:/app -v ./index:/index app-dev:latest`

**To run Coordinator example client use:**

- `docker run -it --name docker-dev --network host -v ./app:/app -v ./index:/index app-dev:latest`
- Inside container run `make example_client && ./example_client <port>`
