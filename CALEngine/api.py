""" Python bindings for bmi_fcgi
"""

import requests
from json import JSONDecodeError

class SessionExistsException(Exception):
    pass

class SessionNotFoundException(Exception):
    pass

class DocNotFoundException(Exception):
    pass

class InvalidJudgmentException(Exception):
    pass

URL = 'http://scspc538.cs.uwaterloo.ca:9002/CAL'


def set_url(url):
    """ Set API endpoint
    """
    global URL
    while url[-1] == '/':
        url = url[:-1]
    URL = url


def begin_session(session_id, seed_query, async=False, mode="doc", seed_documents=[], judgments_per_iteration=1):
    """ Creates a bmi session

    Args:
        session_id (str): unique session id
        seed_query (str): seed query string
        async (bool): If set to True, the server retrains in background whenever possible
        mode (str): For example, "para" or "doc"
        seed_documents ([(str, int), ]): List of tuples containing document_id (str) and its relevance (int)
        judgments_per_iteration (int): Batch size; -1 for default bmi

    Return:
        None

    Throws:
        SessionExistsException
    """

    data = {
        'session_id': str(session_id),
        'seed_query': seed_query,
        'async': str(async).lower(),
        'mode': mode,
        'judgments_per_iteration': str(judgments_per_iteration)
    }
    if len(seed_documents) > 0:
        data['seed_judgments'] = ','.join(['%s:%d' % (doc_id, rel) for doc_id, rel in seed_documents])

    data = '&'.join(['%s=%s' % (k,v) for k,v in data.items()])
    r = requests.post(URL+'/begin', data=data)
    resp = r.json()
    if resp.get('error', '') == 'session already exists':
        raise SessionExistsException("Session %s already exists" % session_id)


def get_docs(session_id, max_count=1):
    """ Get documents to judge

    Args:
        session_id (str): unique session id
        max_count (int): maximum number of doc_ids to fetch

    Returns:
        document ids ([str,]): A list of string document ids

    Throws:
        SessionNotFoundException
    """
    data = '&'.join([
        'session_id=%s' % str(session_id),
        'max_count=%d' % max_count
    ])
    resp = requests.get(URL+'/get_docs?'+data).json()

    if resp.get('error', '') == 'session not found':
        raise SessionNotFoundException('Session %s not found' % session_id)

    return resp['docs']


def judge(session_id, doc_id, rel):
    """ Judge a document
    Args:
        session_id (str): unique session id
        doc_id (str): document id
        rel (int): Relevance judgment 1 or -1

    Returns:
        None

    Throws:
        SessionNotFoundException, DocNotFoundException, InvalidJudgmentException
    """
    if rel > 0:
        rel = 1
    else:
        rel = -1

    data = '&'.join([
        'session_id=%s' % str(session_id),
        'doc_id=%s' % doc_id,
        'rel=%d' % rel
    ])
    resp = requests.post(URL + '/judge', data=data).json()

    if resp.get('error', '') == 'session not found':
        raise SessionNotFoundException('Session %s not found' % session_id)
    elif resp.get('error', '') == 'session not found':
        raise DocNotFoundException('Document %s not found' % doc_id)
    elif resp.get('error', '') == 'session not found':
        raise InvalidJudgmentException('Invalid judgment %d for doc %s' % (rel, doc_id))


def get_ranklist(session_id):
    """ Get the current ranklist

    Args:
        session_id (str): unique session id

    Returns:
        ranklist ([(str, float), ]): Ranked list of document ids

    Throws:
        SessionNotFoundException
    """
    data = '&'.join(['session_id=%s' % str(session_id)])
    resp = requests.get(URL+'/get_ranklist?'+data)

    try:
        if resp.json().get('error', '') == 'session not found':
            raise SessionNotFoundException('Session %s not found' % session_id)
    except JSONDecodeError:
        pass

    ret = []
    for line in resp.text.split('\n'):
        if len(line) == 0:
            continue
        doc_id, score = line.split(' ')
        ret.append((doc_id, float(score)))
    return ret


def delete_session(session_id):
    """ Delete a session

    Args:
        session_id (str): unique session id

    Returns:
        None

    Throws:
        SessionNotFoundException
    """

    data = '&'.join(['session_id=%s' % str(session_id)])
    resp = requests.delete(URL+'/delete_session', data=data).json()

    if resp.get('error', '') == 'session not found':
        print(SessionNotFoundException('Session %s not found' % session_id))
