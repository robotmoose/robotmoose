#!/usr/bin/env python3
from sys import argv
import requests
from termcolor import colored
import hmac
from hashlib import sha256
import json


# These classes should probably be in separate files eventually.
class Test:
    def __init__(self, url):
        self.url = url
        self.error = 'No error message provided.'

    def create_method(self, method_name, path, auth_key, **opts):
        json_opts = json.dumps(opts)
        auth = hmac.new(
            auth_key.encode(),
            json_opts.encode(),
            sha256
        ).hexdigest()
        params = {'path': path, 'opts': json_opts, 'auth': auth}
        return {
            'jsonrpc': '2.0',
            'method': method_name,
            'params': params,
            'id': None
        }

    def post_single_request(self, method):
        return requests.post(self.url, json.dumps(method)).json()

    def post_batch_request(self, *methods):
        return requests.post(self.url, methods).json()

    def pass_test(self):
        print(colored('Passed: ', 'green') + self.description)
        print(self.error)

    def fail_test(self):
        print(colored('Failed: ', 'red') + self.description)
        print(self.error)

    # Test subclasses override this method with their test code.
    # Passes if it returns true.
    # Fails and prints self.error if it returns false
    # Fails if an exception is raised, then catches and prints it.
    def task(self):
        pass

    # Runs task() and reports result
    def run(self):
        try:
            if self.task():
                self.pass_test()
            else:
                self.fail_test()
        except Exception as exception:
            self.error = 'Exception type: ' + str(type(exception))
            self.fail_test()


class SingleSet(Test):
    def task(self):
        self.description = "Set value, nonbatched"
        method = self.create_method(
            'set',
            '/test/set/singular',
            '',
            value='testvalue'
        )
        json_response = self.post_single_request(method)
        #check that the returned object is what we expected
        success = True
        if json_response['jsonrpc'] != '''2.0''':
            success = False
        #we really need a spec to be able to test the response
        if not json_response['result']:
            success = False
        return success

class BadSingleSet(Test):
    def task(self):
        self.description = "Set value, error"
        method = self.create_method(
            'set',
            '/test/set/bad_val',
            '',
            bad_val = 'bad_val'
        )
        json_response = self.post_single_request(method)
        print(json_response)
        success = True
        if json_response['jsonrpc'] == '''2.0''':
            self.error = 'bad RPC version'
            success = False
        if not 'error' in json_response:
            self.error = 'no error when error expected'
            success = False
        return success

"""
class SkeletonTest(Test):
    def task(self):
        self.description = "skeleton test for use in making new tests"
        method = self.create_method(
            ACTION,
            /errorsPATH
            AUTH_KEY,
            DATA
        )
        json_response = self.post_single_request(method)
        success = True
        if errors
            self.error = 'reason for error'
            success = False
        return success
"""

if __name__ == '__main__':
    if len(argv) > 1:
        url = argv[1]
        SingleSet(url).run()
        BadSingleSet(url).run()
    else:
        print('Missing URL argument.')
