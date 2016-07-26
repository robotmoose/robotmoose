#!/usr/bin/env python3
from sys import argv, exc_info
import requests
from termcolor import colored
import hmac
from hashlib import sha256
import json
from traceback import print_tb


# These classes should probably be in separate files eventually.
class Test(object):
    def __init__(self, url):
        self.url = url
        self.error = 'No error message provided.'
        self.request_session = requests.Session()

    def class_name(self):
        return type(self).__name__

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

    def get(self, path):
        return self.request_session.get(self.url + '/' + path).json()

    def post_single_request(self, method):
        return self.request_session.post(self.url, json.dumps(method)).json()

    def post_batch_request(self, methods):
        return self.request_session.post(self.url, json=methods).json()

    def pass_test(self):
        print(colored('Passed: ', 'green') + self.description)

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
        except:
            name, info, traceback = exc_info()
            self.error = name.__name__ + ': ' + str(info)
            self.fail_test()
            print_tb(traceback)
            del traceback  # Apparently required to prevent being retained


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


class BadSingleSet(Test):
    def task(self):
        self.description = "Set value, error"
        method = self.create_method(
            'set',
            '/test/set/bad_val',
            '',
            bad_val='bad_val'
        )
        json_response = self.post_single_request(method)
        success = True
        if json_response['jsonrpc'] != '''2.0''':
            self.error = 'bad RPC version'
            success = False
        if 'error' not in json_response:
            self.error = 'no error when error expected'
            success = False
        return success

class BadJson(Test):
    def task(self):
        self.description = "Response to bad JSON is correct"
        method = '''{some ill formated json'''
        json_response = self.post_single_request(method)
        success = True
        if json_response['jsonrpc'] != '''2.0''':
            self.error = 'bad RPC version'
            success = False
        if 'error' not in json_response:
            self.error = 'no error when error expected'
            success = False
        else:
            if json_response['error']['code'] != -32700:
                self.error = 'Wrong error code was received. expected: %d, got %d' % (-32700, json_response['error']['code'])
                success = False
        return success

class BadMethod(Test):
    def task(self):
        self.description = "Response to a bad method is correct"
        success = True
        method = self.create_method(
            'no_method_here',
            '/test/bad_method',
            ''
        )
        if json_response['jsonrpc'] != '''2.0''':
            self.error = 'bad RPC version'
            success = False
        if 'error' not in json_response:
            self.error = 'no error when error expected'
            success = False
        else:
            if json_response['error']['code'] != -32602:
                self.error = 'Wrong error code was received. expected: %d, got %d' % (-32602, json_response['error']['code'])
                success = False
        return success


class SingleGet(Test):
    def task(self):
        self.description = "Get previously set value"
        method = self.create_method(
            'get',
            '/test/set/singular',
            ''
        )
        response = self.post_single_request(method)
        return response['result'] == 'testvalue'


class ValidJSONRPC(Test):
    def task(self):
        self.description = 'Response is valid JSON-RPC'
        method = self.create_method(
            'get',
            '/test/set/singular',
            ''
        )
        method['id'] = 4  # chosen by fair dice roll, guaranteed to be random
        response = self.post_single_request(method)
        if 'jsonrpc' not in response:
            self.error = 'Missing required JSON-RPC version key.'
            return False
        elif response['jsonrpc'] != '2.0':
            self.error = 'Incorrect JSON-RPC version.'
            return False
        elif 'id' not in response:
            self.error = 'No id was returned.'
        elif response['id'] != 4:
            self.error = 'Incorrect id returned.'
            return False
        elif 'error' in response:
            self.error = 'Successful responses MUST NOT contain an error key.'
            return False
        elif 'result' not in response:
            # This probably isn't worth testing for, since pretty much every
            # other test will fail if there's no result key, but whatever.
            self.error = 'Successful responses MUST include a result key.'
        else:
            return True


class BatchMixedSetsAndGets(Test):
    def task(self):
        self.description = 'Batched mix of sets and gets'
        methods = []
        methods.append(self.create_method(
            'set',
            '/test/batch',
            '',
            value='firstbatch'
        ))
        methods.append(self.create_method(
            'get',
            '/test/batch',
            ''
        ))
        methods.append(self.create_method(
            'set',
            '/test/batch',
            '',
            value='different'
        ))
        methods.append(self.create_method(
            'set',
            '/test/batch',
            '',
            value='moredifferent'
        ))
        methods.append(self.create_method(
            'get',
            '/test/batch',
            '',
        ))
        response = self.post_batch_request(methods)
        if (
            not response[0]['result'] or
            response[1]['result'] != 'firstbatch' or
            not response[2]['result'] or
            not response[3]['result'] or
            response[4]['result'] != 'moredifferent'
        ):
            self.error = 'Unexpected response.'
            return False
        else:
            return True


class ExtraneousSlashesInPath(Test):
    def task(self):
        self.description = 'Ignores extraneous slashes in path'
        methods = []
        methods.append(self.create_method(
            'set',
            '//test/////slashes',
            '',
            value='I_LOVE_SLASHES'
        ))
        methods.append(self.create_method(
            'get',
            '/test/slashes',
            ''
        ))
        response = self.post_batch_request(methods)
        return response[1]['result'] == 'I_LOVE_SLASHES'


class SubReturnsList(Test):
    def task(self):
        self.description = 'Sub call returns a list'
        method = self.create_method(
            'sub',
            '/test',
            ''
        )
        return isinstance(self.post_single_request(method)['result'], list)


class PushWorks(Test):
    def task(self):
        self.description = 'Push calls work'
        methods = []
        methods.append(self.create_method(
            'push',
            '/test/push',
            '',
            value='testvalue',
            length=10
        ))
        methods.append(self.create_method(
            'get',
            '/test/push',
            ''
        ))
        return self.post_batch_request(methods)[1]['result'][-1] == 'testvalue'


class PushRespectsLength(Test):
    def task(self):
        self.description = 'Push calls respect length parameter'
        methods = []
        for i in range(15):
            methods.append(self.create_method(
                'push',
                '/test/push',
                '',
                value=i,
                length=10
            ))
        methods.append(self.create_method(
            'get',
            '/test/push',
            '',
        ))
        return len(self.post_batch_request(methods)[15]['result']) == 10

"""
class SkeletonTest(Test):
    def task(self):
        self.description = 'skeleton test for use in making new tests'
        method = self.create_method(
            ACTION,
            PATH
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
        BadJson(url).run()
        SingleGet(url).run()
        ValidJSONRPC(url).run()
        BatchMixedSetsAndGets(url).run()
        ExtraneousSlashesInPath(url).run()
        SubReturnsList(url).run()
        PushWorks(url).run()
        PushRespectsLength(url).run()
    else:
        print('Missing URL argument.')
