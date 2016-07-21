#!/usr/bin/env python3
from os import urandom
from main import Test
from sys import argv


class StressTest(Test):
    def random_string(self, bytes):
        return str(urandom(bytes))


class SetLargeValue(StressTest):
    def task(self):
        method = self.create_method(
            'set',
            self.random_string(10),
            '',
            value=self.random_string(1000000)
        )
        self.post_single_request(method)


class GetEverything(StressTest):
    # Superstar normally handles this fine, but if the other methods
    # don't kill it this is usually an effective finishing move.
    def task(self):
        self.get('')


class RecursiveExponentialSet(StressTest):
    def recursive_set(self, parent, depth, width, size):
        for _ in range(width):
            self.post_single_request(self.create_method(
                'set',
                parent,
                '',
                value=self.random_string(1000 * size)
            ))
            self.recursive_set(
                parent + '/' + self.random_string(10),
                depth-1,
                width,
                size
            )

    def run(self, depth=3, width=2, size=1000):
        self.recursive_set('', depth, width, size)


if __name__ == '__main__':
    if len(argv) > 1:
        url = argv[1]
        tests = [
            'Create a 1MB child of root',
            'Create an exponentially large tree',
            'GET /'
        ]
        for i in range(len(tests)):
            print(str(i + 1) + ') ' + tests[i])
        test = int(input('Run which test? '))
        times = int(input('How many times should the test be run? '))
        for i in range(times):
            print('Run #' + str(i + 1) + '...')
            # Why doesn't Python have a switch statement? Sigh.
            if test == 1:
                SetLargeValue(url).run()
            elif test == 2:
                depth = int(input('Depth: '))
                width = int(input('Width: '))
                size = int(input('Value size (kbytes): '))
                RecursiveExponentialSet(url).run(depth, width, size)
            elif test == 3:
                GetEverything(url).run()
            else:
                print('Was it really that hard to pick a number between 1 '
                      'and ' + str(len(tests)) + '?')
    else:
        print('No URL argument provided.')
