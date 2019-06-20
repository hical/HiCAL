from django.db.models import Q
from hicalweb.judgment.models import Judgment


def join_judgments(document_values, document_ids, user, task):
    """
    Adds the relevance judgment of the document to the document_values dict.
    If document has not been judged yet, `isJudged` will be False.
    :param user:
    :param task:
    :param document_values:
    :param document_ids:
    :return: document_values with extra information about the document
    """

    judged_docs = Judgment.objects.filter(Q(user=user,
                                             task=task,
                                             doc_id__in=document_ids) &
                                           (
                                               Q(rel=0) |
                                               Q(rel=1) |
                                               Q(rel=2)
                                           )
                                           )
    judged_docs = {j.doc_id: j for j in judged_docs}
    for id in document_ids:
        isJudged = True if id in judged_docs else False
        if isJudged:
            rel = judged_docs.get(id)
            document_values[id]['rel'] = rel
        document_values[id]['isJudged'] = isJudged
    return document_values


def padder(doc_ids):
    """
    Pads zeros to ids to be length of 7
    :param doc_ids: list of document ids
    :return: doc ids padded with 0 to be length of 7.
    """

    for idx, doc_id in enumerate(doc_ids):
        doc_id = doc_id.zfill(7)
        doc_ids[idx] = doc_id

    return doc_ids
