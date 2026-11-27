"""
Evaluation: Recall@k.

Given a dataset of questions with their expected source document(s), the
evaluator measures how many of the expected documents appear within the top-k
retrieved results for each question, and reports Recall@1, Recall@3,
Recall@5 and Recall@k.

A retrieved result matches an expected document if its ``file_path`` matches
(and, when the expectation includes character offsets, those also match).
"""
from collections import defaultdict


def _expected_matches(result_chunk, expected):
    """Whether a retrieved chunk matches one expected-document entry."""
    # expected may be a string (file_path) or a dict with file_path +
    # optional character range.
    if isinstance(expected, str):
        return result_chunk.file_path == expected
    if isinstance(expected, dict):
        if result_chunk.file_path != expected.get("file_path"):
            return False
        if "first_character_index" in expected:
            if (result_chunk.first_character_index
                    < expected["first_character_index"]):
                return False
        if "last_character_index" in expected:
            if (result_chunk.last_character_index
                    > expected["last_character_index"]):
                return False
        return True
    return False


def _question_resolved(results, expected, k):
    """Whether at least one expected document appears in the top-k."""
    top = results[:k]
    for exp in expected:
        for r in top:
            if _expected_matches(r.chunk, exp):
                return True
    return False


def evaluate(results_by_query, dataset, ks=(1, 3, 5)):
    """Compute Recall@k for the dataset.

    ``results_by_query`` maps a question string to its list of ``SearchResult``.
    ``dataset`` is a list of {"question": str, "expected": [...]}.

    Returns a dict: {recall_at_1, recall_at_3, recall_at_5, ..., total}.
    """
    total = len(dataset)
    if total == 0:
        return {"recall_at_1": 0.0, "recall_at_3": 0.0, "recall_at_5": 0.0,
                "total": 0}

    hits_by_k = defaultdict(int)
    for item in dataset:
        question = item["question"]
        expected = item.get("expected", [])
        results = results_by_query.get(question, [])
        for k in ks:
            if _question_resolved(results, expected, k):
                hits_by_k[k] += 1

    report = {"total": total}
    for k in ks:
        report[f"recall_at_{k}"] = hits_by_k[k] / total
    return report
