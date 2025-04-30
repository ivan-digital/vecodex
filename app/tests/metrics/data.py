from datasets import load_dataset
import torch
import os

MAX_DOCS_STORE = int(os.environ.get("MAX_DOCS_STORE")) if os.environ.get("MAX_DOCS_STORE") else 1000
MAX_QUERIES_EVAL = int(os.environ.get("MAX_QUERIES_EVAL")) if os.environ.get("MAX_QUERIES_EVAL") else 10
METRICS_FILENAME = os.environ.get("METRICS_FILENAME") if os.environ.get("METRICS_FILENAME") else "metrics.pkl"
CHARTS_FILENAME = os.environ.get("CHARTS_FILENAME") if os.environ.get("CHARTS_FILENAME") else "metrics.png"
PARAMETER_NAME = "k"
DATASET_NAME = "Cohere/wikipedia-22-12-en-embeddings"
RETRIEVED_CNT_LIST = [2 * x for x in range(2, 20)]


class Dataset:
    def __init__(self, k):
        self.k = k
        self.doc_ids = []
        self.doc_embs = []
        self.docs = []
        self.eval_doc_ids = []
        self.eval_embs = []
        self.eval_docs = []
        self.target_doc_ids = []
        self.predicted_doc_ids = []
        self.index_id = "0"
        self.init_()

    def find_relevant_ids_(self, query_emb) -> list[str]:
        query_emb = torch.tensor(query_emb)

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
                self.docs.append(doc['text'] + " " + doc['title'])
            elif i < MAX_DOCS_STORE + MAX_QUERIES_EVAL:
                self.eval_doc_ids.append(docid)
                self.eval_embs.append(emb)
                self.eval_docs.append(doc['text'] + " " + doc['title'])
            else:
                break

        self.doc_embs = torch.tensor(self.doc_embs)
        self.eval_embs = torch.tensor(self.eval_embs)

        for docid, emb in zip(self.eval_doc_ids, self.eval_embs):
            self.target_doc_ids.append(self.find_relevant_ids_(emb))
    
    def precision_k(self, relevant, retrieved):
        ans = 0.0
        for rel, ret in zip(relevant, retrieved):
            rels = set(rel)
            rets = set(ret)
            ans += len(rets.intersection(rels)) / len(rels) 
        return ans / len(relevant)

    def recall_k(self, relevant, retrieved):
        ### Not implemented
        return 0

    def _average_precision(self, relevant, retrieved):
        relevant = set(relevant)
        retrieved = torch.as_tensor(retrieved)
        relevant_tensor = torch.tensor(list(relevant))
        
        is_relevant = (retrieved.unsqueeze(1) == relevant_tensor.unsqueeze(0)).any(dim=1)
        
        relevant_at_k = torch.cumsum(is_relevant.float(), dim=0)
        
        precision_at_k = relevant_at_k / (torch.arange(len(retrieved), dtype=torch.float32) + 1)
        
        ap = torch.sum(precision_at_k * is_relevant.float()) / len(relevant)
        
        return ap.item()

    def _mean_average_precision(self, relevant_docs, retrieved_docs):
        aps = []
        for rel, ret in zip(relevant_docs, retrieved_docs):
            # Skip queries with no relevant documents
            if len(rel) == 0:
                continue
            ap = self._average_precision(rel, ret)
            aps.append(ap)
        
        return torch.mean(torch.tensor(aps)) if aps else 0.0
    
    def _dcg(self, relevance_scores, k=None):
        discounts = torch.log2(torch.arange(2, len(relevance_scores) + 2))
        gains = (2 ** relevance_scores.float() - 1)
        return torch.sum(gains / discounts)

    def mean_ndcg(self, relevant_docs, retrieved_docs, k=None):
        ans = 0.
        for rel, ret in zip(relevant_docs, retrieved_docs):
            relevance_scores = torch.tensor([x in ret for x in rel], dtype=torch.bool)
            ideal_scores = torch.full((len(ret),), 1)
        
            dcg_score = self._dcg(relevance_scores, k)
            idcg_score = self._dcg(ideal_scores, k)
            
            ans += dcg_score / idcg_score

        return ans / len(relevance_scores)

    def iterate_write_document(self):
        for docid, emb in zip(self.doc_ids, self.doc_embs):
            yield docid, emb
    
    def iterate_search_requests(self):
        for emb in self.eval_embs:
            yield emb

    def save_predicted_ids(self, predicted_ids: list[str]):
        self.predicted_doc_ids.append(predicted_ids)

    def count_metrics(self):
        print("Target docids:\n", self.target_doc_ids)

        metrics = {
            "mAP": self._mean_average_precision(self.target_doc_ids, self.predicted_doc_ids),
            "precision_k": self.precision_k(self.target_doc_ids, self.predicted_doc_ids),
            "ndcg": self.mean_ndcg(self.target_doc_ids, self.predicted_doc_ids),
        }
        
        print(f"metrics: {metrics}")
        return metrics
