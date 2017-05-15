import httplib2
import urllib.parse
import json

from config.settings.base import DOCUMENT_SNIPPET_ENGINE_SERVER_IP,\
    DOCUMENT_SNIPPET_ENGINE_SERVER_PORT


def get_documents(doc_ids, query):
    """
    :param doc_ids: the ids of documents to return
    :return: documents content
    """
    h = httplib2.Http()
    url = "http://{}:{}/fetch?"
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
                'title': doc['result']['title'] + " " + doc['doc_id'],
                'content': doc['result']['content'],
            }
            result.append(document)
    else:
        # TODO: complete this case: returned non 200 error
        print("Error getting document from DocEngine: {} {}".format(resp, content))

    return result
