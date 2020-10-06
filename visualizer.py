#!/usr/bin/python3.8

import networkx as nx
import matplotlib.pyplot as plt
import time
import numpy as np
import argparse
import sys

from scipy.optimize import minimize, rosen, rosen_der
from typing import List, Tuple, Callable, Dict, Set
from random import shuffle


Point = Tuple[float, float]


def segment_len(a: Point, b: Point) -> float:
    '''
    Returns length of segment based on given points
    '''
    return np.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2)


def is_counter_clockwise(a: Point, b: Point, c: Point) -> float:
    '''
    Returns if points are sorted in counter clockwise order
    '''
    return (c[1] - a[1]) * (b[0] - a[0]) > (b[1] - a[1]) * (c[0] - a[0])


def segment_intersect(a: Point, b: Point, c: Point, d: Point) -> bool:
    '''
    Returns if segments that are based on given points are intersect
    '''
    return is_counter_clockwise(a, c, d) != is_counter_clockwise(b, c, d) and is_counter_clockwise(a, b, c) != is_counter_clockwise(a, b, d)


class GraphInformation:
    '''
    Contains information about graph
    '''

    def __init__(self,
                 planar_layout: Dict[int, Point],
                 minimal_edge: float,
                 maximal_edge: float,
                 edges: List[Tuple[int, int]],
                 graph: nx.Graph):
        '''
        Initializes GraphInformation
        planar_layout - planar graph nodes coordinates
        minimal_edge - minimal acceptable edge
        maximal_edge - maximal acceptable edge
        edges - list of graph edges
        graph - graph
        '''
        self.minimal_edge = minimal_edge
        self.maximal_edge = maximal_edge
        self.planar_layout = planar_layout
        self.max_edge = 0
        self.edges = edges
        self.graph = graph
        for f, t in edges:
            self.max_edge = max(self.max_edge, segment_len(
                planar_layout[f], planar_layout[t]))


def cell_edge_min_len_penny(cell: List[Tuple[int, int]], info: GraphInformation) -> float:
    '''
    Returns float value in range [0; 1]
    Penalizes graph cell for very short edges
    '''
    min_len = info.max_edge
    for f, t in cell:
        min_len = min(min_len, segment_len(
            info.planar_layout[f], info.planar_layout[t]))
    return (info.minimal_edge - min_len) / info.minimal_edge if min_len < info.minimal_edge else 0


def cell_edge_max_len_penny(cell: List[Tuple[int, int]], info: GraphInformation) -> float:
    '''
    Returns float value in range [0; 1]
    Penalizes graph cell for very long edges
    '''
    max_len = 0
    for f, t in cell:
        max_len = max(max_len, segment_len(
            info.planar_layout[f], info.planar_layout[t]))
    return (max_len - info.maximal_edge) / info.max_edge if max_len > info.maximal_edge else 0


def cell_edge_diff_penny(cell: List[Tuple[int, int]], info: GraphInformation) -> float:
    '''
    Returns float value in range [0; 1]
    Penalizes graph cell for large difference between longest and shortest edge
    '''
    max_len = 0
    min_len = info.max_edge
    for f, t in cell:
        min_len = min(min_len, segment_len(
            info.planar_layout[f], info.planar_layout[t]))
        max_len = max(max_len, segment_len(
            info.planar_layout[f], info.planar_layout[t]))
    return (max_len - min_len) / info.max_edge


def vec_mpl(a: Point, b: Point) -> float:
    '''
    Returns vector product of vector a and b
    '''
    return a[0] * b[1] - b[0] * a[1]


def cell_convexity_penny(cell: List[Tuple[int, int]], info: GraphInformation) -> float:
    '''
    Returns float value in range [0; 1]
    Penalizes graph cell non-convexity
    '''
    less = 0
    more = 0

    def get_ith_edge_vector(i: int) -> Tuple[int, int]:
        i %= len(cell)
        x1 = info.planar_layout[cell[i][0]][0]
        y1 = info.planar_layout[cell[i][0]][1]
        x2 = info.planar_layout[cell[i][1]][0]
        y2 = info.planar_layout[cell[i][1]][1]
        return (x2 - x1, y2 - y1)

    for i in range(len(cell)):
        u1 = get_ith_edge_vector(i)
        u2 = get_ith_edge_vector(i + 1)
        less += vec_mpl(u1, u2) < 0
        more += vec_mpl(u1, u2) >= 0
    return (min(less, more) / len(cell)) * 2.0


def cell_diameter_penny(cell: List[Tuple[int, int]], info: GraphInformation) -> float:
    '''
    Returns float value in range [0; 1]
    Penalizes graph cell for large ratio of maximum coordinates
    '''

    cell_mass_centre = np.zeros(shape=(2,))

    for _, to in cell:
        x = info.planar_layout[to][0]
        y = info.planar_layout[to][1]
        cell_mass_centre += np.array([x, y])

    cell_mass_centre /= (len(cell) - 1)

    mind: float = info.max_edge * 1000
    maxd: float = 0.0

    for _, to in cell:
        x = info.planar_layout[to][0]
        y = info.planar_layout[to][1]
        dist = segment_len((x, y), cell_mass_centre)
        mind = min(mind, dist)
        maxd = max(maxd, dist)

    return 1.0 - (mind / maxd)


def planarity_penny(info: GraphInformation) -> float:
    '''
    Returns float value in range [0; 1]
    Penalizes graph non-planarity
    '''
    intersec_count = 0
    for first_edge in info.edges:
        for second_edge in info.edges:
            first_edge = sorted(first_edge)
            second_edge = sorted(second_edge)
            if first_edge == second_edge or first_edge[0] in second_edge or first_edge[1] in second_edge:
                continue

            if segment_intersect(info.planar_layout[first_edge[0]], info.planar_layout[first_edge[1]],
                                 info.planar_layout[second_edge[0]], info.planar_layout[second_edge[1]]):
                intersec_count += 1
    return intersec_count / (len(info.edges) ** 2)


def planarity_penny_edge(edge_from: int, edge_to: int, info: GraphInformation) -> float:
    '''
    Returns float value in range [0; 1]
    Penalizes the cells of the graph adjacent to an edge for non-planarity
    '''
    intersec_count = 0
    first_edge = sorted([edge_from, edge_to])
    for second_edge in info.edges:
        first_edge = sorted(first_edge)
        second_edge = sorted(second_edge)
        if first_edge == second_edge or first_edge[0] in second_edge or first_edge[1] in second_edge:
            continue

        if segment_intersect(info.planar_layout[first_edge[0]], info.planar_layout[first_edge[1]],
                             info.planar_layout[second_edge[0]], info.planar_layout[second_edge[1]]):
            intersec_count += 1
    return intersec_count / len(info.edges)


class PennyCalculator:
    '''
    Penny calculator for given graph
    '''

    def __init__(self,
                 cell_pennies: List[Tuple[Callable[[List[Tuple[int, int]], GraphInformation], float], float]],
                 graph_pennies: List[Tuple[Callable[[GraphInformation], float], float]],
                 cells: List[List[Tuple[int, int]]],
                 info: GraphInformation):
        '''
        cell_pennies: list of [per-cell penny function, weigth of penny]
        graph_pennies: list of [graph penny function, weigth of penny]
        cells: list of graph cells
        info: graph information
        '''
        self.cell_pennies = cell_pennies
        self.graph_pennies = graph_pennies
        self.cells = cells
        self.graph_info = info

    def __call__(self, unrolled_layout: np.array) -> float:
        '''
        Calculates total penny for given layout
        '''
        total: float = 0.0

        for i in range(0, len(unrolled_layout), 2):
            self.graph_info.planar_layout[i / 2] = (
                unrolled_layout[i],
                unrolled_layout[i + 1]
            )

        for penny, coef in self.cell_pennies:
            for cell in self.cells:
                total += penny(cell, self.graph_info) * coef / len(self.cells)

        for penny, coef in self.graph_pennies:
            total += penny(self.graph_info) * coef

        return total


class PennyCalculatorVertexMutator:
    '''
    Penny calculator if only one vertex is being mutated
    '''

    def __init__(self,
                 cell_pennies: List[Tuple[Callable[[List[Tuple[int, int]], GraphInformation], float], float]],
                 edge_pennies: List[Tuple[Callable[[int, int, GraphInformation], float], float]],
                 cells: List[List[Tuple[int, int]]],
                 info: GraphInformation):
        '''
        cell_pennies: list of [per-cell penny function, weigth of penny]
        edge_pennies: list of [edge penny function, weigth of penny]
        cells: list of graph cells
        info: graph information
        '''
        self.cell_pennies = cell_pennies
        self.edge_pennies = edge_pennies
        self.cells = cells
        self.graph_info = info
        self.current_mutated = 0
        self.node_in_cells = [[] for i in range(len(info.planar_layout))]
        self.neighbors = [[] for i in range(len(info.planar_layout))]
        for i, cell in enumerate(cells):
            for fst, _ in cell:
                self.node_in_cells[fst].append(i)
        for f, t in info.edges:
            self.neighbors[f].append(t)
            self.neighbors[t].append(f)

    def set_current_mutation(self, to_mutate: int) -> None:
        '''
        Sets current mutated vertex
        '''
        self.current_mutated = to_mutate

    def __call__(self, unrolled_layout: np.array) -> float:
        '''
        Calculates total penny for given mutated node position
        '''
        total: float = 0.0

        self.graph_info.planar_layout[self.current_mutated] = (
            unrolled_layout[0],
            unrolled_layout[1])

        for penny, coef in self.cell_pennies:
            for cell_id in self.node_in_cells[self.current_mutated]:
                total += coef * penny(self.cells[cell_id],
                                      self.graph_info) / len(self.node_in_cells[self.current_mutated])

        for penny, coef in self.edge_pennies:
            for neig in self.neighbors[self.current_mutated]:
                total += coef * penny(self.current_mutated,
                                      neig,
                                      self.graph_info) / len(self.neighbors[self.current_mutated])

        return total


class PennyCalculatorCellMutator:
    '''
    Penny calculator if single cell is being mutated
    '''

    def __init__(self,
                 cell_pennies: List[Tuple[Callable[[List[Tuple[int, int]], GraphInformation], float], float]],
                 edge_pennies: List[Tuple[Callable[[int, int, GraphInformation], float], float]],
                 cells: List[List[Tuple[int, int]]],
                 info: GraphInformation):
        '''
        cell_pennies: list of [per-cell penny function, weigth of penny]
        edge_pennies: list of [edge penny function, weigth of penny]
        cells: list of graph cells
        info: graph information
        '''
        self.cell_pennies = cell_pennies
        self.edge_pennies = edge_pennies
        self.cells = cells
        self.graph_info = info
        self.current_mutated: List[int] = []
        self.adj_edges: List[Tuple[int, int]] = []

    def set_current_mutation(self, cell_to_mutate: List[int]) -> None:
        '''
        Sets current mutated cell
        '''
        self.current_mutated = cell_to_mutate
        self.adj_edges.clear()
        for node in self.current_mutated:
            for neig in self.graph_info.graph.neighbors(node):
                self.adj_edges.append(sorted((node, neig)))
        self.adj_edges = list(set(self.adj_edges))

    def __call__(self, unrolled_layout: np.array) -> float:
        '''
        Calculates total penny for given mutated cell nodes positions
        '''
        total: float = 0.0

        for i in range(len(self.current_mutated)):
            self.graph_info.planar_layout[self.current_mutated[i]] = np.array(
                [unrolled_layout[i * 2], unrolled_layout[i * 2]])

        for penny, coef in self.cell_pennies:
            for cell in self.cells:
                total += penny(cell, self.graph_info) / len(self.cells)

        for penny, coef in self.edge_pennies:
            for edge in self.adj_edges:
                total += penny(edge[0],
                               edge[1],
                               self.graph_info) / len(self.adj_edges)

        return total


def get_planar_graph_cells(graph: nx.Graph, nodes: Set[int], edges: List[Tuple[int, int]], planar_layout: Dict[int, np.array]) -> List[List[int]]:
    '''
    Returns list of planar graph cells
    graph: graph
    edges: list of graph edges
    planar_layout: node position in planar layout
    '''
    edges_ordered = {node: [] for node in nodes}
    for node in nodes:
        nodes_to = []
        for nb in graph.neighbors(node):
            nodes_to.append(nb)

        def compare_by_polar_angle(a):
            a_vec = (
                planar_layout[a][0] - planar_layout[node][0],
                planar_layout[a][1] - planar_layout[node][1]
            )
            return np.arctan2(a_vec[1], a_vec[0])

        nodes_to.sort(key=compare_by_polar_angle)

        for node_to in nodes_to:
            edges_ordered[node].append(node_to)

    cells = []
    is_in_cells = {node: set() for node in nodes}

    for n_from, n_to in edges:
        if n_to in is_in_cells[n_from]:
            continue
        cur, nxt, prev = n_from, n_to, -1
        cur_cell = []
        while True:
            cur_cell.append((cur, nxt))
            is_in_cells[cur].add(nxt)
            prev = cur
            cur = nxt

            cur_vec = (
                planar_layout[prev][0] - planar_layout[cur][0],
                planar_layout[prev][1] - planar_layout[cur][1]
            )
            best_candidate = -1
            best_candidate_angle = np.pi * 4
            for candidate in graph.neighbors(cur):
                if candidate == prev:
                    continue
                cand_vec = (
                    planar_layout[candidate][0] - planar_layout[cur][0],
                    planar_layout[candidate][1] - planar_layout[cur][1]
                )
                cand_angle = (-np.arctan2(cand_vec[1], cand_vec[0]) +
                              np.arctan2(cur_vec[1], cur_vec[0])) % (np.pi * 2)
                if cand_angle < best_candidate_angle:
                    best_candidate_angle = cand_angle
                    best_candidate = candidate
            nxt = best_candidate

            if nxt == n_to:
                break
        cells.append(cur_cell)
    return cells


def main(argv: List[str]) -> None:

    parser = argparse.ArgumentParser(
        description='Van Kampen diagram visualiser')
    parser.add_argument('path', type=str, nargs=1,
                        help='Path of .edges file (can be generated with vankamp-vis)')
    args = parser.parse_args(argv)

    edges = []
    nodes = set()

    with open(args.path[0]) as f:
        for a, b in map(lambda s: s.split(), f.readlines()):
            fr, to = int(a), int(b)
            edges.append((fr, to))
            nodes.add(fr)
            nodes.add(to)

    graph = nx.Graph()

    for v in nodes:
        graph.add_node(v)

    for fr, to in edges:
        graph.add_edge(fr, to)

    planar_layout = nx.planar_layout(graph)
    cells = get_planar_graph_cells(graph, nodes, edges, planar_layout)

    graph_info = GraphInformation(
        planar_layout=planar_layout,
        minimal_edge=0.1,
        maximal_edge=0.5,
        edges=edges
    )
    penny = PennyCalculatorVertexMutator(
        [
            (cell_edge_diff_penny, 2.0),
            (cell_edge_min_len_penny, 2.0),
            (cell_edge_max_len_penny, 3.0),
            (cell_diameter_penny, 2.0),
            (cell_convexity_penny, 1.0),
        ],
        [
            (planarity_penny_edge, 1000.0),
        ],
        cells,
        graph_info,
    )

    nodes_list = list(nodes)
    iterations = 10
    cnt = 0

    for _ in range(iterations):
        shuffle(nodes_list)
        for node in nodes_list:
            penny.set_current_mutation(node)
            plt.show()
            cnt += 1
            print("\rDone: {:.2f}".format(
                100 * cnt / (iterations * len(nodes_list))), end=' %')

            prev_node_pos = penny.graph_info.planar_layout[node][:]

            initial = np.zeros(shape=(2,))
            neigs = 0
            for neig in graph.neighbors(node):
                initial += graph_info.planar_layout[neig]
                neigs += 1
            initial /= neigs

            nodes_sum = np.zeros(shape=(2,))
            for v in nodes_list:
                nodes_sum += penny.graph_info.planar_layout[v]
            nodes_sum /= len(penny.graph_info.planar_layout)

            from_center_to_cur = (initial - nodes_sum) / 100.0
            initial += from_center_to_cur

            penny.graph_info.planar_layout[node] = initial

            result = minimize(
                penny,
                initial,
                method='Nelder-Mead',
                tol=1e-6
            )

            non_planarity: float = 0
            for neig in graph.neighbors(node):
                non_planarity += planarity_penny_edge(
                    node, neig, penny.graph_info)
            if non_planarity > 0:
                penny.graph_info.planar_layout[node] = prev_node_pos
            else:
                penny.graph_info.planar_layout[node] = result.x

    nx.draw(graph,
            pos=penny.graph_info.planar_layout,
            width=1,
            node_size=5)
    # plt.savefig('out.svg')
    plt.show()

    print()


if __name__ == '__main__':
    main(sys.argv[1:])
