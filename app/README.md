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

**Start etcd in separate container:**

- `docker run -e ALLOW_NONE_AUTHENTICATION=yes -e ETCD_ADVERTISE_CLIENT_URLS=http://etcd:2379 --network host bitnami/etcd:latest`

**Start minio in separate container:**

- `docker run --network host -e MINIO_ROOT_USER=user -e MINIO_ROOT_PASSWORD=password -v ~/minio/data:/data minio/minio:latest server /data --console-address ":9001"`

