import grpc
import pickle
import tqdm
from generated.service_pb2 import Document, SearchRequest, WriteRequest
from generated.service_pb2_grpc import BaseServiceStub
from data import Dataset, RETRIEVED_CNT_LIST, METRICS_FILENAME, PARAMETER_NAME, CHARTS_FILENAME
import matplotlib.pyplot as plt


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

    def search_documents(self, index_id, vector_data, k):
        request = SearchRequest(
            vector_data=[float(x) for x in vector_data.tolist()],
            k=k,
            index_id=index_id,
        )
        return self.searcher_stub.ProcessSearchRequest(request)


class VecodexTest:
    def __init__(self, parameters_list):
        self.client = GrpcClient(
            writer_host="localhost:8004",
            searcher_host="localhost:8000"
        )
        self.parameters_list = parameters_list
        self.metrics_storage = []


    def write_batch(self):
        for docid, emb in tqdm.tqdm(self.dataset.iterate_write_document(), desc="Write"):
            try:
                self.client.write_document(self.dataset.index_id, docid, emb)
            except Exception as e:
                print(f"Exception occured while write request: {e}")


    def search_batch(self):
        for emb in tqdm.tqdm(self.dataset.iterate_search_requests(), desc="Search"):
            try:
                out = self.client.search_documents(self.dataset.index_id, emb, self.dataset.k)
                self.dataset.save_predicted_ids([int(x) for x in out.ids])
                print(f"search response: {out}")
            except Exception as e:
                print(f"Exception occured while search request: {e}")

    
    def save_metrics(self):
        metrics = self.dataset.count_metrics()
        self.metrics_storage.append(metrics)
        with open(METRICS_FILENAME, "wb") as f:
            pickle.dump(self.metrics_storage, f)

    
    def plot(self):
        metrics_storage = {}
        for metrics in self.metrics_storage:
            for k, v in metrics.items():
                if k in metrics_storage: metrics_storage[k].append(v)
                else: metrics_storage[k] = [v]             
        
        ncharts = len(metrics_storage)
        figure = plt.figure(figsize=(ncharts * 3, 10))
        idx = 0
        for metric_name, values in metrics_storage.items():
            idx += 1
            ax = figure.add_subplot(ncharts, 1, idx)        
            ax.plot(RETRIEVED_CNT_LIST, values, label=metric_name)
            ax.set_xlabel(PARAMETER_NAME)
            ax.set_ylabel(metric_name)
            ax.legend()
        figure.savefig(CHARTS_FILENAME)


    def run(self):
        for k in self.parameters_list:
            self.dataset = Dataset(k=k)
            self.write_batch()
            self.search_batch()
            self.save_metrics()     
        self.plot()


if __name__ == "__main__":
    suite = VecodexTest(RETRIEVED_CNT_LIST)
    suite.run()
