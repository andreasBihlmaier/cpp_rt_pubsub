#!/usr/bin/env python3

import json
import argparse
import numpy as np
import matplotlib.pyplot as plt


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'results_file', help='File containing the JSON results from benchmark generator node')
    args = parser.parse_args()

    results = None
    with open(args.results_file, 'r') as results_file:
        results = json.load(results_file)
    for result in results:
        print(f'generator {result["generator_id"]} <-> reflector {result["reflector_id"]}: min={result["data"]["min"]} average={result["data"]["average"]} max={result["data"]["max"]} #values={result["data"]["values_count"]}')
        values = np.array(result["data"]["values"])
        plt.hist(values, bins='auto')
        plt.title(
            f'Round trip latency [ns] between generator {result["generator_id"]} and reflector {result["reflector_id"]}')
        plt.show()


if __name__ == '__main__':
    main()
