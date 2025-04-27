python3 -m grpc_tools.protoc -I=../../proto --python_out=generated --grpc_python_out=generated ../../proto/service.proto
locust --processes -1 -f locustfile.py MixedTrafficUser