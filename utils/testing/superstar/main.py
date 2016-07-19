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
            self.error = exception
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
        return self.post_single_request(method)['result']


if __name__ == '__main__':
    if len(argv) > 1:
        url = argv[1]
        SingleSet(url).run()
    else:
        print('Missing URL argument.')
