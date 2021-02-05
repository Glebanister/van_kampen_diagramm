#!/usr/bin/python3.8

from sys import argv

import matplotlib.pyplot as plt
import numpy as np
from numpy.linalg import eig, eigvals
from scipy.sparse.csgraph import laplacian


def main(args: [str]) -> None:
    '''
    Generate eigenvalue distribution chart from graph <input-file>.
    The input format of the graph, which consists of directed edges
    [(a_1, b_1), ... (a_n, b_n)], where a_i, b_i are 0-indexed integers:

    a_1 b_1
    a_2 b_2
    ...
    a_n b_n

    The output will be generated in the <input-file>-eigval.png
    '''
    input_filename = args[0]
    edges = []
    max_node = 0
    with open(input_filename, 'r') as inp:
        for line in inp.readlines():
            f, t = map(int, line.split())
            edges.append((f, t))
            max_node = max(max_node, f, t)
    nodes = max_node + 1
    mtx = np.zeros(shape=(nodes, nodes))
    for f, t in edges:
        mtx[f, t] += 1.0
        mtx[t, f] += 1.0
    laplacian_mtx = laplacian(mtx)
    print(f'Laplacian: \n{laplacian_mtx}')
    print(f'Nodes in graph: {nodes}')
    vals = eigvals(laplacian_mtx)
    print(f'Eigenvalues: {vals}')
    plt.plot(sorted(vals))
    # plt.show()
    # plt.savefig()
    plt.savefig(input_filename + '-eigval.svg')


if __name__ == '__main__':
    main(argv[1:])
