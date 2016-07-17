import requests
from sys import argv
from termcolor import colored
import hmac
from hashlib import sha256
import json

# This script reimplements JSONRPC/Superstar communication spec rather than 
# using the existing Python library, to prevent changes to that library
# from becoming de facto spec changes.

##
# Helper methods
##

def create_method(method_name, path, auth_key, **opts):
    json_opts = json.dumps(opts)
    auth = hmac.new(auth_key.encode(), json_opts.encode(), sha256).hexdigest()
    params = {'path': path, 'opts': json_opts, 'auth': auth}
    return {'jsonrpc': '2.0', 'method': method_name, 'params': params, 'id': None}

def post_single_request(method):
    return requests.post(argv[1], json.dumps(method)).json()

def post_batch_request(*methods):
    return requests.post(argv[1], methods).json()

def pass_test(test):
    print(colored('Passed: ', 'green') + test)

def fail_test(test):
    print(colored('Failed: ', 'red') + test)

##
# Tests - each is a self-contained function, so that all main has to do is call them.
##

def test_set_singular():
    name = "Set value, nonbatched"
    method = create_method('set', '/test/set/singular', '', value='testvalue')
    print(post_single_request(method))
    if post_single_request(method)['result']:
        pass_test(name)
    else:
        fail_test(test)

    


if __name__ == '__main__':
    test_set_singular()
