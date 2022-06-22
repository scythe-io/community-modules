import uuid
import sys
import marshal
import pickle
from io import StringIO

g_client = None

CATEGORY_WORKER = 4
PYTHON_MODULE_ID = uuid.UUID('bf5e4010-f260-11ec-9bb2-290282c90ef4')

def init(client, **kwargs):
    """

    :param client:
    :param kwargs:
    :return:
    """
    global g_client
    g_client = client
    return True


def run(message,  **kwargs):
    """

    :param bytes message:
    :param kwargs:
    :return bytes or None: None if post will happen asynchronously
    """
    #unpickle
    code = pickle.loads(message)
    #unmarshal
    exec_code = marshal.loads(code)
    message = None
    my_stdout = StringIO()
    my_stderr = StringIO()
    try:
        sys.stdout = my_stdout
        sys.stderr = my_stderr
        exec(exec_code)
    except Exception as e:
        message = f"Error: {e}".encode('utf-8')
        return message.encode('utf-8')
    message = f"STDOUT:\n {my_stdout.getvalue()}\nSTDERR:\n {my_stderr.getvalue()}"
    return message.encode('utf-8')


def getinfo():
    """

    :return:
    """
    return { "type": CATEGORY_WORKER, "version" : {"major": 1, "minor": 0}, "id" : PYTHON_MODULE_ID}


def deinit(**kwargs):
    """

    :param kwargs:
    :return:
    """
    return True
