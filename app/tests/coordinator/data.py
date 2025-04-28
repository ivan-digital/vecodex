from datasets import load_dataset
import torch

MAX_DOCS_STORE = 10000
MAX_QUERIES_EVAL = 20
K = 10
DATASET_NAME = "Cohere/wikipedia-22-12-en-embeddings"


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
        ans = 0.0
        ### Not implemented
        return ans

    def _average_precision(self, relevant, retrieved):
        """
        Calculate Average Precision (AP) for a single query using PyTorch.
        """
        relevant = set(relevant)
        retrieved = torch.as_tensor(retrieved)
        relevant_tensor = torch.tensor(list(relevant))
        
        # PyTorch equivalent of np.in1d
        is_relevant = (retrieved.unsqueeze(1) == relevant_tensor.unsqueeze(0)).any(dim=1)
        
        # Cumulative sum of relevant items at each rank
        relevant_at_k = torch.cumsum(is_relevant.float(), dim=0)
        
        # Precision at each rank
        precision_at_k = relevant_at_k / (torch.arange(len(retrieved), dtype=torch.float32) + 1)
        
        # Average precision only over relevant ranks
        ap = torch.sum(precision_at_k * is_relevant.float()) / len(relevant)
        
        return ap.item()


    def _mean_average_precision(self, relevant_docs, retrieved_docs):
        """
        Calculate Mean Average Precision (mAP) across multiple queries.
        
        Args:
            relevant_docs: List of lists, where each sublist contains indices of relevant docs for a query
            retrieved_docs: List of lists, where each sublist contains indices of retrieved docs for a query
            
        Returns:
            Mean Average Precision score
        """
        aps = []
        for rel, ret in zip(relevant_docs, retrieved_docs):
            # Skip queries with no relevant documents
            if len(rel) == 0:
                continue
            ap = self._average_precision(rel, ret)
            aps.append(ap)
        
        return torch.mean(torch.tensor(aps)) if aps else 0.0


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
        # print(self.predicted_doc_ids)
        print(self.target_doc_ids)

        metrics = {
            "mAP": self._mean_average_precision(self.target_doc_ids, self.predicted_doc_ids),
            "precision_k": self.precision_k(self.target_doc_ids, self.predicted_doc_ids),
            "recall_k": self.recall_k(self.target_doc_ids, self.predicted_doc_ids),
        }
        
        print(f"metrics: {metrics}")


if __name__ == "__main__":
    d = Dataset(k=10)
    print("searched: ", d.eval_docs[19], end='\n')
    print("9993: ", d.docs[9993], end='\n')
    print("3542: ", d.docs[3542], end='\n')
    print(torch.sum(d.doc_embs[9993] * d.eval_embs[19]), end='\n')
    print(torch.sum(d.doc_embs[3542] * d.eval_embs[19]), end='\n')