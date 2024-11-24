# Vector Search Cluster Vecodex Kubernetes Go Operator

The **Vecodex Kube-Operator** is a Kubernetes controller that manages the lifecycle of a custom resource called `VecodexSet`. It orchestrates the creation, update, and deletion of deployments, services, and ingress resources for Vecodex components: `Coordinator`, `Writer`, and `Searcher`.

---

## Table of Contents
- [Resource Structure](#resource-structure)
- [Controller Functionality](#controller-functionality)
- [Building the Operator](#building-the-operator)
- [Using the Operator](#using-the-operator)
- [Testing the Operator](#testing-the-operator)

---

## Resource Structure

The custom resource `VecodexSet` defines the desired state for managing Vecodex components. Below is the structure of the `VecodexSet` resource:

### Example `VecodexSet` Resource

```yaml
apiVersion: apps.vecodex.link/v1alpha1
kind: VecodexSet
metadata:
  name: search-cluster
  namespace: default
spec:
  coordinator:
    image: "vecodex-coordinator-image:latest"
    replicas: 1
  writer:
    image: "vecodex-writer-image:latest"
    replicas: 1
  searcher:
    image: "vecodex-searcher-image:latest"
    replicas: 3
  ingress:
    host: "search.vecodex.link"
```

### `VecodexSet` Specification

| Field          | Description                                                                 |
|-----------------|-----------------------------------------------------------------------------|
| `coordinator`   | Configuration for the Coordinator component, including image and replicas. |
| `writer`        | Configuration for the Writer component, including image and replicas.      |
| `searcher`      | Configuration for the Searcher component, including image and replicas.    |
| `ingress.host`  | The hostname for the ingress resource.                                     |

The operator uses this specification to create the following Kubernetes resources:
- **Deployments**: For `coordinator`, `writer`, and `searcher`.
- **Ingress**: Routes traffic to the `coordinator` service.
- **Services**: Exposes each deployment internally within the cluster.

---
## Controller Functionality

The **Vecodex Kube-Operator** implements the following core functionalities:

1. **Resource Reconciliation**:
   - Watches for changes to [`VecodexSet`](api/v1alpha1/vecodexset_types.go) resources.
   - The [`Reconcile`](internal/controller/vecodexset_controller.go#L44) function ensures the associated deployments, services, and ingress are in the desired state.

2. **Deployment Management**:
   - Creates or updates deployments for `coordinator`, `writer`, and `searcher` components.
   - Handled by the [`reconcileDeployment`](internal/controller/vecodexset_controller.go#L96) helper function.

3. **Ingress Management**:
   - Creates an ingress resource pointing to the `coordinator` service.
   - Handled by the [`reconcileIngress`](internal/controller/vecodexset_controller.go#L135) helper function.

4. **Status Updates**:
   - Updates the status of the [`VecodexSet`](api/v1alpha1/vecodexset_types.go) resource to reflect the current state of the underlying Kubernetes resources.
   - Status updates are triggered within the [`Reconcile`](hinternal/controller/vecodexset_controller.go#L44) function.
If you change something you need to run:
```bash 
make generate
make manifests
```
[Kubernetes Operator SDK architecture](https://sdk.operatorframework.io/docs/building-operators/golang/tutorial/)

## Building the Operator

To build the operator, ensure you have the required tools installed:
- [Go](https://golang.org/doc/install) (v1.19+)
- [Docker](https://www.docker.com/products/docker-desktop)
- [Kubebuilder](https://book.kubebuilder.io/quick-start.html)

### Steps to Build

1. Clone the repository:
   ```bash
   git clone https://github.com/ivan-digital/vecodex.git
   cd kube-operator
   ```

2. Build the Docker image using the [`Makefile`](https://github.com/ivan-digital/vecodex/kube-operator/blob/main/Makefile):
  ```bash
  make docker-build IMG=<docker hub username>/vecodex-operator:v0.0.1
  ```
3. Push the Docker image to your registry:
  ```bash
  make docker-push IMG=<docker hub username>/vecodex-operator:v0.0.1
  ```
4. (Optional) Verify the image in your registry to ensure it was pushed successfully.

## Using the Operator

### Install CRDs
Before deploying the operator, install the necessary CRDs into your Kubernetes cluster. This step ensures the cluster understands the custom VecodexSet resource.

```bash
make install
```
This installs CRDs defined in config/crd/bases.

### Deploy the Operator
Deploy the operator to your cluster using the built image:

```bash
make deploy IMG=<docker hub username>/vecodex-operator:v0.0.1
```
### Apply a VecodexSet Resource
Create an example VecodexSet resource to verify the operator's functionality:

```bash
kubectl apply -f config/samples/apps_v1alpha1_vecodexset.yaml
```
You can modify the sample YAML file at [apps_v1alpha1_vecodexset.yaml](config/samples/apps_v1alpha1_vecodexset.yaml) to suit your requirements.

## Testing the Operator

### Unit Tests
Run unit tests to validate the controller logic:

```bash
make test
```
Unit tests are located in the controllers directory and cover reconciliation behavior.

### End-to-End (E2E) Tests
Set up a local cluster using Kind:
```bash
kind create cluster
```
Possibly you need to install `brew install kind`

### Run the E2E test suite:
```bash
go test ./test/e2e/... -ginkgo.v
```
Possibly you need to install `brew install kubebuilder`. Also on mac requires binaries etcd to download `curl -LO https://storage.googleapis.com/kubebuilder-tools/kubebuilder-tools-1.28.0-darwin-amd64.tar.gz` and put in `kubebuilder/bin/` directory.
The E2E tests validate the complete lifecycle of the VecodexSet resource, ensuring the operator behaves as expected in a live cluster environment.