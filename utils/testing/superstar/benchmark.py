#!/usr/bin/env python3
from main import Test
from time import time
import argparse
import gc
import random
import string

from multiprocessing.pool import ThreadPool


class Benchmark(Test):
    """Benchmark 'interface'

Not intended to be created, only inherited from classes that inherit from
this class need only to implement the function req_gen req_gen should
return a JSON rpc request object, usually via the create_method method.
When using this class, the with syntax should be used, so that garbage
collection is only done at times when the benchmarks aren't running
    """
    def __enter__(self):
        return self
    def __exit__(self, *err):
        gc.collect()
    def run(self, runs):
        tests = []
        for _ in range(runs):
            tests.append(self.task())
        start_time = time()
        with ThreadPool(processes=2) as tp:
            tp.map(lambda t : t(), tests)
        elapsed_time = time() - start_time
        print('Performed ' + str(runs) + 'x' + self.class_name() + ' in ' +
              str(elapsed_time) + 'sec.')
        print('Average time/run: ' + str(elapsed_time / runs * 1000) +
              ' msec.\n')
    def task(self):
        req = self.req_gen()
        return lambda : self.post_single_request(req)


class RandomSet(Benchmark):
    """Produces a bunch of random keys to set to random paths, then sets them"""
    def req_gen(self):
        def rs(size):
            return ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(size))
        path = [rs(random.randint(5, 10)) for _ in range(random.randint(1,10))]
        value = rs(random.randint(5,20))
        return self.create_method(
                    'set',
                    path,
                    '',
                    value = value
               )

class SingleSet(Benchmark):
    def req_gen(self):
        return self.create_method(
            'get',
            '/benchmark/test',
            '',
            value='foobar'
        )


class SingleGet(Benchmark):
    def req_gen(self):
        return self.create_method(
            'get',
            '/',
            ''
        )

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description = 'Benchmark utility for superstar v2')
    parser.add_argument('--url', metavar='URL', type=str, default='http://localhost:8081/superstar', help='url to run the benchmark on')
    parser.add_argument('--runs', metavar='N', type=int, default=1000, help='number of iterations per test')
    args =parser.parse_args()
    url = args.url
    runs = args.runs

    with SingleGet(url) as r:
        r.run(runs)
    with SingleSet(url) as r:
        r.run(runs)
    with RandomSet(url) as r:
        r.run(runs)
