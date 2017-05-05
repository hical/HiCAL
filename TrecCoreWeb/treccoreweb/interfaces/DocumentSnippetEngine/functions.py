def dummy_get_documents(doc_ids, query):
    """
    :param doc_ids: the ids of documents to return
    :return: lorem-based list of documents
    """

    from django.utils import lorem_ipsum
    result = []
    for i in range(len(doc_ids)):
        document = {
            'doc_id': doc_ids[i],
            'title': query + " " + lorem_ipsum.words(1, common=False),
            'content': lorem_ipsum.paragraphs(1, common=False)[0],
        }

        result.append(document)

    return result
