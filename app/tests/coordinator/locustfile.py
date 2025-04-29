from locust import User, task, between, events, SequentialTaskSet
import grpc
import time
import pickle
from generated.service_pb2 import Document, SearchRequest, WriteRequest
from generated.service_pb2_grpc import BaseServiceStub
from data import Dataset, MAX_DOCS_STORE, MAX_QUERIES_EVAL, K, RETRIEVED_CNT_LIST, METRICS_FILENAME

METRICS_STORAGE = []


class GrpcClient:
    def __init__(self, writer_host, searcher_host):
        self.writer_channel = grpc.insecure_channel(writer_host)
        self.searcher_channel = grpc.insecure_channel(searcher_host)
        self.writer_stub = BaseServiceStub(self.writer_channel)
        self.searcher_stub = BaseServiceStub(self.searcher_channel)

    def write_document(self, index_id, docid, vector_data):
        doc = Document(
            vector_data=[float(x) for x in vector_data.tolist()],
            id=str(docid),
            attributes={"index-id": index_id}
        )
        return self.writer_stub.ProcessWriteRequest(WriteRequest(data=doc))

    def search_documents(self, index_id, vector_data):
        request = SearchRequest(
            vector_data=[float(x) for x in vector_data.tolist()],
            k=K,
            index_id=index_id,
        )
        return self.searcher_stub.ProcessSearchRequest(request)


class CustomSequence(SequentialTaskSet):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.first_task_repeat = MAX_DOCS_STORE
        self.second_task_repeat = MAX_QUERIES_EVAL
        self.dataset = Dataset(k=K)

    @task
    def initial_task(self):
        for docid, emb in self.dataset.iterate_write_document():
            start_time = time.time()
            try:
                self.client.write_document(self.dataset.index_id, docid, emb)
                self.track_request("write", start_time)
            except Exception as e:
                self.track_request("write", start_time, e)  

    @task
    def main_task(self):
        for emb in self.dataset.iterate_search_requests():
            start_time = time.time()
            try:
                out = self.client.search_documents(self.dataset.index_id, emb)
                self.track_request("search", start_time)
                print(f"out = {out}")
                self.dataset.save_predicted_ids([int(x) for x in out.ids])
            except Exception as e:
                self.track_request("search", start_time, e)

    @task
    def count_metrics(self):
        metrics = self.dataset.count_metrics()
        METRICS_STORAGE.append(metrics)
        with open(METRICS_FILENAME, "wb") as f:
            pickle.dump(METRICS_STORAGE, f)
        self.interrupt()
    
    def track_request(self, request_type, start_time, exception=None):
        events.request.fire(
            request_type=request_type,
            name=request_type,
            response_time=(time.time() - start_time) * 1000,
            response_length=0,
            exception=exception
        )


class CustomUser(User):
    wait_time = between(0.1, 1)
    tasks = [CustomSequence]
    load_counter = 0

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.client = GrpcClient(
            writer_host="localhost:8004",
            searcher_host="localhost:8000"
        )

    def on_start(self):
        # Assign different data per user
        if RETRIEVED_CNT_LIST is None:
            self.tasks[0].dataset = Dataset(k=K)
        else:
            assert(CustomUser.load_counter < len(RETRIEVED_CNT_LIST), f"Created more Users than experiments set up: {CustomUser.load_counter}")
            self.tasks[0].dataset = Dataset(k=RETRIEVED_CNT_LIST[CustomUser.load_counter])
        CustomUser.load_counter += 1