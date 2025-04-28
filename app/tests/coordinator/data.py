from datasets import load_dataset
import torch

MAX_DOCS_STORE = 1000
MAX_QUERIES_EVAL = 10
K = 20
DATASET_NAME = "Cohere/wikipedia-22-12-en-embeddings"


class Dataset:
    def __init__(self, k):
        self.k = k
        self.doc_ids = []
        self.doc_embs = []
        self.eval_doc_ids = []
        self.eval_embs = []
        self.target_doc_ids = []
        self.predicted_doc_ids = []
        self.index_id = "0"
        self.init_()

    def find_relevant_ids_(self, query_emb) -> list[str]:
        query_emb = torch.tensor(query_emb)

        # Compute L2 score between query embedding and document embeddings
        dot_scores = query_emb @ self.doc_embs.transpose(1, 0)
        top_k = torch.topk(dot_scores, self.k)
        
        real_docs_ids = []
        
        for doc_id in top_k.indices.tolist():
            real_docs_ids.append(self.doc_ids[doc_id])
        
        return real_docs_ids

    def init_(self):
        docs_stream = load_dataset(DATASET_NAME, split="train", streaming=True)
        for i, doc in enumerate(docs_stream):
            docid = doc['id']
            emb = doc['emb']
            if i < MAX_DOCS_STORE:
                self.doc_ids.append(docid)
                self.doc_embs.append(emb)
            elif i < MAX_DOCS_STORE + MAX_QUERIES_EVAL:
                self.eval_doc_ids.append(docid)
                self.eval_embs.append(emb)
            else:
                break

        self.doc_embs = torch.tensor(self.doc_embs)    
        print(f"embs shape = {self.doc_embs.shape}")
        
        self.eval_embs = torch.tensor(self.eval_embs)    

        for docid, emb in zip(self.eval_doc_ids, self.eval_embs):
            self.target_doc_ids.append(self.find_relevant_ids_(emb))
    

    def mAP(self, found: list[str], real: list[str]):
        pass


    def iterate_write_document(self):
        for docid, emb in zip(self.doc_ids, self.doc_embs):
            print(f"write doc id: {docid}")
            yield docid, emb
    

    def iterate_search_requests(self):
        for emb in self.eval_embs:
            print(f"search")
            yield emb


    def save_predicted_ids(self, predicted_ids: list[str]):
        self.predicted_doc_ids.append(predicted_ids)


    def count_metrics(self):
        print(self.predicted_doc_ids)
        print(self.target_doc_ids)


if __name__ == "__main__":
    dataset = Dataset(k=10)
    a = dataset.iterate_write_document()
    print(a)