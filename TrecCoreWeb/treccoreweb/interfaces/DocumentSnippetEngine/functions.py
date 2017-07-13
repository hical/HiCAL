import os
import traceback
from datetime import date
from lxml import etree

try:
    # For c speedups
    from simplejson import loads, dumps
except ImportError:
    from json import loads, dumps

from config.settings.base import DOCUMENTS_PATH, PARA_PATH

def exec_xpath(tree, xpath):
    try:
        val = tree.xpath(xpath)[0]
        val_str = etree.tostring(val, encoding="unicode")
        if len(val_str) == 0:
            return "N/A"
        return val_str
    except:
        traceback.print_exc()
        return "N/A"

def get_date(tree, xpath):
    try:
        d = tree.xpath(xpath)[0]
        year = int(d[:4])
        month = int(d[4:6])
        day = int(d[6:8])
        d = date(year,month,day)
        return d.strftime("%A %d. %B %Y")
    except:
        traceback.print_exc()
        return "N/A"

def get_documents(doc_ids, query):
    """
    :param query:
    :param doc_ids: the ids of documents to return
    :return: documents content
    """
    result = []
    for doc_id in doc_ids:
        tree = etree.parse(os.path.join(DOCUMENTS_PATH, doc_id[:4], doc_id + '.xml'))
        title = exec_xpath(tree, '/nitf/body[1]/body.head/hedline/hl1')
        content = exec_xpath(tree, '/nitf/body/body.content/block[@class="full_text"]')
        date = get_date(tree, '/nitf/head/pubdata/@date.publication')
        document = {
            'doc_id': doc_id,
            'title': title,
            'content': content,
            'date': date
        }
        result.append(document)

    return result


def get_documents_with_snippet(doc_ids, query, top_terms):
    result = get_documents([doc['doc_id'] for doc in doc_ids], query)
    for doc_para_id, doc in zip(doc_ids, result):
        if 'para_id' not in doc_para_id:
            doc['snippet'] = 'N/A'
            continue
        try:
            para_id = doc_para_id['doc_id'] + '.' + doc_para_id['para_id']
            with open(os.path.join(PARA_PATH, para_id[:4], para_id)) as f:
                doc['snippet'] = f.read().strip()
        except:
            traceback.print_exc()
            doc['snippet'] = 'N/A'
    return result
