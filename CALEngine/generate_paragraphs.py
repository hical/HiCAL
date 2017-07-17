"""
Extract paragraphs from xml files
"""
import os
import sys
import shutil
import traceback
from xpaths import DATA_FIELDS_DICT
from tqdm import tqdm
from lxml import etree
from multiprocessing import Queue, Process
import time

def exec_xpath(tree, xpath):
    vals = tree.xpath(xpath)
    val_str = [etree.tostring(val, encoding="unicode", method="text") for val in vals]
    return val_str

SOURCE = sys.argv[1]
DEST = sys.argv[2]
if not os.path.exists(DEST):
    os.makedirs(DEST)

def writer(q):
    while True:
        p, v = q.get()
        if p is None:
            break
        with open(p, 'w') as f:
            f.write(v)

queue = Queue(10000)
pp = Process(target=writer, args=(queue,))
pp.start()
pbar = tqdm()
for root, dirs, files in os.walk(SOURCE):
    for name in files:
        pbar.update(1)
        path = os.path.join(root, name)
        dest_path = os.path.join(DEST, name[:4])
        doc_id = name.split('.')[0]
        tree = etree.parse(path)

        title = exec_xpath(tree, '/nitf/body[1]/body.head/hedline/hl1')
        if len(title) == 0:
            title = ''
        else:
            title = title[0]
        paragraphs = exec_xpath(tree, '//p')

        if not os.path.exists(dest_path):
            os.makedirs(dest_path)
        queue.put((os.path.join(dest_path, doc_id + '.' + str(0)), title))
        for idx, paragraph in enumerate(paragraphs):
            queue.put((os.path.join(dest_path, doc_id + '.' + str(idx+1)), paragraph))

queue.put((None, None))
pp.join()
pbar.close()
