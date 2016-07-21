#!/usr/bin/env python3
from main import Test
from time import time
from sys import argv


class Benchmark(Test):
    def run(self, runs):
        start_time = time()
        for _ in range(runs):
            self.task()
        elapsed_time = time() - start_time
        print('Performed ' + str(runs) + 'x' + self.class_name() + ' in ' +
              str(elapsed_time) + 'sec.')
        print('Average time/run: ' + str(elapsed_time / runs * 1000) +
              ' msec.\n')


class SingleSet(Benchmark):
    def task(self):
        response = self.post_single_request(self.create_method(
            'set',
            '/benchmark/set',
            '',
            value='test_value'
        ))
        return response['result']


class SingleGet(Benchmark):
    def task(self):
        response = self.post_single_request(self.create_method(
            'get',
            '/',
            ''
        ))
        return response['result']


if __name__ == '__main__':
    if len(argv) > 2:
        url = argv[1]
        runs = int(argv[2])
        SingleSet(url).run(runs)
        SingleGet(url).run(runs)
    else:
        print('Arguments missing.')
        print('Format: ./benchmark.py address:port/superstar/ '
              + 'numberOfIterations')
        print('Ex: ./benchmark.py http://localhost:8081/superstar/ 100')
