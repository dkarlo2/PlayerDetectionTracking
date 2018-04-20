#pragma once

#include <iostream>
#include <map>
#include <set>
#include <vector>

#define non_exist (std::numeric_limits<double>::max())

class Node {
	int id;
	bool type;
	int typeIndex;
public:
	Node() : id(-1) {}
	Node(bool T, int TI);
	int getId() const {
		return id;
	}
	bool isType() const {
		return type;
	}
	int getTypeIndex() const {
		return typeIndex;
	}
	bool operator<(const Node &other) const {
		return id < other.id;
	}
	bool operator==(const Node &other) const {
		return id == other.id;
	}
};

class Graph {
	std::vector<Node> nodes;
	double** edges;
	std::map<Node, int> nodesPos;
	std::map<Node, int> neisCount;
	int size;
	bool initialized = false;
public:
	Graph(int S) : size(S) {
		edges = new double*[size];
		for (int i = 0; i < size; i++) {
			edges[i] = new double[size];
		}
	}
	~Graph() {
		for (int i = 0; i < size; i++) {
			delete[] edges[i];
		}
		delete[] edges;
	}
	std::vector<Node> getNodes() {
		return nodes;
	}
	double** getEdges() {
		return edges;
	}
	void addNode(Node n) {
		if (initialized) {
			std::cerr << "Already initialized!" << std::endl;
			throw "";
		}
		nodesPos[n] = nodes.size();
		nodes.push_back(n);
	}
	void init() {
		if (initialized) {
			std::cerr << "Already initialized!" << std::endl;
			throw "";
		}
		for (int i = 0; i < nodes.size(); i++) {
			for (int j = 0; j < nodes.size(); j++) {
				edges[i][j] = non_exist;
			}
		}
		initialized = true;
	}
	void addEdge(Node n1, Node n2, double d) {
		if (!initialized) {
			std::cerr << "Not initialized!" << std::endl;
			throw "";
		}
		edges[nodesPos[n1]][nodesPos[n2]] = d;
		edges[nodesPos[n2]][nodesPos[n1]] = d;
	}
	int getNeisCount(Node n) {
		return neisCount[n];
	}
	bool isEmpty(Node n) {
		return neisCount[n] == 0;
	}
	void updateNeisCount();
};


std::map<Node, std::set<Node>> findMinimalEdgeCover(Graph &g);
