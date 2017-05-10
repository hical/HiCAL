import httplib2
import urllib.parse
import json

from config.settings.base import CAL_SERVER_IP, CAL_SERVER_PORT


def send_judgment(session, doc_id, rel, next_batch_size=5):
    h = httplib2.Http()
    url = "http://{}:{}/CAL/judge"

    body = {'session_id': str(session),
            'doc_id': doc_id,
            'rel': rel}
    body = urllib.parse.urlencode(body)
    resp, content = h.request(url.format(CAL_SERVER_IP,
                                         CAL_SERVER_PORT),
                              body=body,
                              headers={'Content-Type': 'application/json; charset=UTF-8'},
                              method="POST")

    if resp and resp['status'] == '200':
        content = json.loads(content)
        return content['docs']
    else:
        # TODO: Complete this function
        pass

    return []


def add_session(session, seed_query):
    """
    Adds session to CAL backend server
    :param session:
    :param seed_query
    :return:
    """
    h = httplib2.Http()
    url = "http://{}:{}/CAL/begin"

    body = {'session_id': str(session),
            'seed_query': seed_query}
    body = urllib.parse.urlencode(body)
    resp, content = h.request(url.format(CAL_SERVER_IP,
                                         CAL_SERVER_PORT),
                              body=body,
                              headers={'Content-Type': 'application/json; charset=UTF-8'},
                              method="POST")
    if resp and resp['status'] != '200':
        # TODO: Stop the creation of this topic
        pass


def get_documents(session, num_docs, query):
    """
    :param session: current session
    :param num_docs: number of documents to return
    :return: return JSON list of documents_ids to judge
    """
    h = httplib2.Http()
    url = "http://{}:{}/CAL/get_docs?"

    parameters = {'session_id': str(session), 'max_count': 5}
    parameters = urllib.parse.urlencode(parameters)
    resp, content = h.request(url.format(CAL_SERVER_IP,
                                         CAL_SERVER_PORT) + parameters,
                              method="GET")

    if resp and resp['status'] == '200':
        content = json.loads(content)
        return content['docs']
    else:
        # TODO: update this else condition
        print("CAL server returend something not 200")

    return []
