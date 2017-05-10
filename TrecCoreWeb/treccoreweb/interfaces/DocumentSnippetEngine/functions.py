import httplib2
import urllib.parse
import json

from config.settings.base import DOCUMENT_SNIPPET_ENGINE_SERVER_IP,\
    DOCUMENT_SNIPPET_ENGINE_SERVER_PORT


def get_documents(doc_ids, query):
    """
    :param doc_ids: the ids of documents to return
    :return: lorem-based list of documents
    """
    h = httplib2.Http()
    url = "http://{}:{}/fetch?"
    print(doc_ids)
    parameters = {'docid': " ".join(doc_ids)}
    parameters = urllib.parse.urlencode(parameters)
    resp, content = h.request(url.format(DOCUMENT_SNIPPET_ENGINE_SERVER_IP,
                                         DOCUMENT_SNIPPET_ENGINE_SERVER_PORT) + parameters,
                              method="GET")
    result = []
    if resp and resp['status'] == '200':
        content = json.loads(content)
        for doc in content:
            isFound = doc.get("isFound", False)
            if not isFound:
                continue

            document = {
                'doc_id': doc['doc_id'],
                'title': doc['result']['title'],
                'content': doc['result']['content'],
            }
            result.append(document)
    else:
        # TODO: complete this case: returned non 200 error
        print(resp, content)

    return result

    #
    # from django.utils import lorem_ipsum
    #
    # for i in range(len(doc_ids)):
    #     document = {
    #         'doc_id': doc_ids[i],
    #         'title': query + " " + doc_ids[i],
    #         'content': lorem_ipsum.paragraphs(1, common=False)[0],
    #     }
    #
    #     result.append(document)
    #
    # return result
