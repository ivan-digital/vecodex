from locust import User, task, between, events
import random
import grpc
import grpc.experimental.gevent as grpc_gevent
import time
from generated.service_pb2 import Document, SearchRequest, WriteRequest
from generated.service_pb2_grpc import BaseServiceStub

grpc_gevent.init_gevent()

class GrpcClient:
    def __init__(self, writer_host, searcher_host):
        self.writer_channel = grpc.insecure_channel(writer_host)
        self.searcher_channel = grpc.insecure_channel(searcher_host)
        self.writer_stub = BaseServiceStub(self.writer_channel)
        self.searcher_stub = BaseServiceStub(self.searcher_channel)

    def write_document(self):
        index_id = str(random.randint(0, 1))
        doc = Document(
            vector_data=[random.random() for _ in range(2)],
            id=f"doc_{random.randint(1, 100000)}",
            attributes={"index-id": index_id}
        )
        return self.writer_stub.ProcessWriteRequest(WriteRequest(data=doc))

    def search_documents(self):
        index_id = str(random.randint(0, 1))
        request = SearchRequest(
            vector_data=[random.random() for _ in range(2)],
            k=5,
            index_id=index_id
        )
        return self.searcher_stub.ProcessSearchRequest(request)


class MixedTrafficUser(User):
    wait_time = between(0.1, 1)

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.client = GrpcClient(
            writer_host="localhost:8004",
            searcher_host="localhost:8000"
        )

    def track_request(self, request_type, start_time, exception=None):
        events.request.fire(
            request_type=request_type,
            name=request_type,
            response_time=(time.time() - start_time) * 1000,
            response_length=0,
            exception=exception
        )

    @task(1)
    def write_task(self):
        start_time = time.time()
        try:
            self.client.write_document()
            self.track_request("write", start_time)
        except Exception as e:
            self.track_request("write", start_time, e)

    @task(3)
    def search_task(self):
        start_time = time.time()
        try:
            self.client.search_documents()
            self.track_request("search", start_time)
        except Exception as e:
            self.track_request("search", start_time, e)
