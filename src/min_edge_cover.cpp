#include "min_edge_cover.h"
#include "time_measurement.h"

#include <queue>

#undef min
#undef max

// mostly copied from
//		https://www.topcoder.com/community/data-science/data-science-tutorials/assignment-problem-and-hungarian-algorithm/
// ###############################################################################################################################
#define INF 100000000 //just infinity

int max_match; //n workers and n jobs
double *lx, *ly; //labels of X and Y parts
int *xy; //xy[x] - vertex that is matched with x,
int *yx; //yx[y] - vertex that is matched with y
bool *S, *T; //sets S and T in algorithm
double *slack; //as in the algorithm description
int *slackx; //slackx[y] such a vertex, that
			   // l(slackx[y]) + l(y) - w(slackx[y],y) = slack[y]
int *prev; //array for memorizing alternating paths

void init_labels(int n, double **cost)
{
	memset(lx, 0, n * sizeof(*lx));
	memset(ly, 0, n * sizeof(*ly));
	for (int x = 0; x < n; x++)
		for (int y = 0; y < n; y++)
			lx[x] = std::max(lx[x], cost[x][y]);
}

void update_labels(int n)
{
	int x, y;
	double delta = INF; //init delta as infinity
	for (y = 0; y < n; y++) //calculate delta using slack
		if (!T[y])
			delta = std::min(delta, slack[y]);
	for (x = 0; x < n; x++) //update X labels
		if (S[x]) lx[x] -= delta;
	for (y = 0; y < n; y++) //update Y labels
		if (T[y]) ly[y] += delta;
	for (y = 0; y < n; y++) //update slack array
		if (!T[y])
			slack[y] -= delta;
}

void add_to_tree(int x, int prevx, int n, double **cost)
//x - current vertex,prevx - vertex from X before x in the alternating path,
//so we add edges (prevx, xy[x]), (xy[x], x)
{
	S[x] = true; //add x to S
	prev[x] = prevx; //we need this when augmenting
	for (int y = 0; y < n; y++) //update slacks, because we add new vertex to S
		if (lx[x] + ly[y] - cost[x][y] < slack[y])
		{
			slack[y] = lx[x] + ly[y] - cost[x][y];
			slackx[y] = x;
		}
}

void augment(int n, double **cost) //main function of the algorithm
{
	if (max_match == n) return; //check wether matching is already perfect
	int x, y, root; //just counters and root vertex
	int *q = new int[n];
	int wr = 0, rd = 0; //q - queue for bfs, wr,rd - write and read
							  //pos in queue
	memset(S, false, n * sizeof(*S)); //init set S
	memset(T, false, n * sizeof(*T)); //init set T
	memset(prev, -1, n * sizeof(*prev)); //init set prev - for the alternating tree
	for (x = 0; x < n; x++) //finding root of the tree
		if (xy[x] == -1)
		{
			q[wr++] = root = x;
			prev[x] = -2;
			S[x] = true;
			break;
		}

	for (y = 0; y < n; y++) //initializing slack array
	{
		slack[y] = lx[root] + ly[y] - cost[root][y];
		slackx[y] = root;
	}

	//second part of augment() function
	while (true) //main cycle
	{
		while (rd < wr) //building tree with bfs cycle
		{
			x = q[rd++]; //current vertex from X part
			for (y = 0; y < n; y++) //iterate through all edges in equality graph
				if (cost[x][y] == lx[x] + ly[y] && !T[y])
				{
					if (yx[y] == -1) break; //an exposed vertex in Y found, so
											//augmenting path exists!
					T[y] = true; //else just add y to T,
					q[wr++] = yx[y]; //add vertex yx[y], which is matched
									 //with y, to the queue
					add_to_tree(yx[y], x, n, cost); //add edges (x,y) and (y,yx[y]) to the tree
				}
			if (y < n) break; //augmenting path found!
		}
		if (y < n) break; //augmenting path found!

		update_labels(n); //augmenting path not found, so improve labeling
		wr = rd = 0;
		for (y = 0; y < n; y++)
			//in this cycle we add edges that were added to the equality graph as a
			//result of improving the labeling, we add edge (slackx[y], y) to the tree if
			//and only if !T[y] && slack[y] == 0, also with this edge we add another one
			//(y, yx[y]) or augment the matching, if y was exposed
			if (!T[y] && slack[y] == 0)
			{
				if (yx[y] == -1) //exposed vertex in Y found - augmenting path exists!
				{
					x = slackx[y];
					break;
				}
				else
				{
					T[y] = true; //else just add y to T,
					if (!S[yx[y]])
					{
						q[wr++] = yx[y]; //add vertex yx[y], which is matched with
										 //y, to the queue
						add_to_tree(yx[y], slackx[y], n, cost); //and add edges (x,y) and (y,
																//yx[y]) to the tree
					}
				}
			}
		if (y < n) break; //augmenting path found!
	}

	if (y < n) //we found augmenting path!
	{
		max_match++; //increment matching
					 //in this cycle we inverse edges along augmenting path
		for (int cx = x, cy = y, ty; cx != -2; cx = prev[cx], cy = ty)
		{
			ty = xy[cx];
			yx[cy] = cx;
			xy[cx] = cy;
		}
		augment(n, cost); //recall function, go to step 1 of the algorithm
	}

	delete q;
}//end of augment() function

int* hungarian(int n, double **cost)
{
	lx = new double[n];
	ly = new double[n];
	xy = new int[n];
	yx = new int[n];
	S = new bool[n];
	T = new bool[n];
	slack = new double[n];
	slackx = new int[n];
	prev = new int[n];

	int a[6];

	max_match = 0; //number of vertices in current matching
	memset(xy, -1, n * sizeof(*xy));
	memset(yx, -1, n * sizeof(*yx));
	init_labels(n, cost); //step 0
	augment(n, cost); //steps 1-3

	delete[] prev;
	delete[] slackx;
	delete[] slack;
	delete[] T;
	delete[] S;
	delete[] yx;
	delete[] ly;
	delete[] lx;

	return xy;
}
// ###############################################################################################################################


static int nodeIdGenerator = 0;

Node::Node(bool T, int TI) : id(nodeIdGenerator++), type(T), typeIndex(TI) {}

void Graph::updateNeisCount() {
	neisCount.clear();
	for (int i = 0; i < nodes.size(); i++) {
		neisCount[nodes[i]] = 0;
		for (int j = 0; j < nodes.size(); j++) {
			if (edges[i][j] != non_exist) {
				neisCount[nodes[i]]++;
			}
		}
	}
}

static void decreaseTableElement(double &element, double d) {
	if (element != non_exist) {
		element -= d;
		if (element < 1e-5) {
			element = 0;
		}
	}
}

static void increaseTableElement(double &element, double d) {
	if (element != non_exist) {
		element += d;
	}
}

static void dumpTable(double **table, int n) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (table[i][j] == non_exist) {
				std::cout << "- ";
			}
			else {
				std::cout << table[i][j] << ' ';
			}
		}
		std::cout << std::endl;
	}
	system("pause");
}

std::map<Node, std::set<Node>> findMinimalEdgeCover(Graph &g) {
	std::vector<Node> nodes = g.getNodes();
	double** edges = g.getEdges();
	int n = nodes.size();

	std::vector<int> nodeMapping;
	for (int i = 0; i < nodes.size(); i++) {
		if (g.isEmpty(nodes[i])) {
			n--;
		} else {
			nodeMapping.push_back(i);
		}
	}

	std::cerr << "Number of non empty nodes: " << n << std::endl;

	if (n == 0) {
		return std::map<Node, std::set<Node>>();
	}

	std::vector<int> minWeightNodes;

	MyTime mt;
	mt.start();

	// create a table representing the expanded graph
	double **table = new double*[n];
	for (int i = 0; i < n; i++) {
		table[i] = new double[n];
	}

	for (int i = 0; i < n; i++) {
		double d = std::numeric_limits<double>::max();
		int minDIndex = -1;
		for (int j = 0; j < n; j++) {
			table[i][j] = edges[nodeMapping[i]][nodeMapping[j]];
			if (table[i][j] != non_exist && table[i][j] < d) {
				d = table[i][j];
				minDIndex = j;
			}
		}
		if (minDIndex != -1) {
			table[i][i] = 2 * d;
			minWeightNodes.push_back(minDIndex);
		} else {
			std::cerr << "Was" << std::endl;
			throw "";
		}
	}

	// check
	for (int i = 0; i < n; i++) {
		Node ni = nodes[nodeMapping[i]];
		for (int j = 0; j < n; j++) {
			Node nj = nodes[nodeMapping[j]];
			if (table[i][j] != non_exist && i != j && (ni.isType() && nj.isType() || !ni.isType() && !nj.isType())) {
				std::cerr << "should not happen" << std::endl;
				throw "";
			}
		}
	}

	// dumpTable(table, n);

	// convert from minimal to maximal weight perfect matching problem
	double d = 0;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (table[i][j] != non_exist) {
				d = std::max(d, table[i][j]);
			}
		}
	}
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (table[i][j] != non_exist) {
				table[i][j] = d - table[i][j];
			} else {
				table[i][j] = -std::numeric_limits<double>::max();
			}
		}
	}

	// dumpTable(table, n);

	std::cerr << "Init table: " << mt.time() << std::endl;

	mt.start();

	int *assignments = hungarian(n, table);

	std::cerr << "Hungarian: " << mt.time() << std::endl;

	std::map<Node, std::set<Node>> cover;
	for (int i = 0; i < n; i++) {
		Node ni = nodes[nodeMapping[i]];
		cover[ni] = std::set<Node>();
	}

	for (int i = 0; i < n; i++) {
		if (assignments[i] == -1) {
			continue;
		}
		Node ni = nodes[nodeMapping[i]];
		int j = assignments[i];
		if (i == j) {
			Node n = nodes[nodeMapping[minWeightNodes[i]]];
			cover[ni].insert(n);
			cover[n].insert(ni);
			if (n.isType() && ni.isType() || !n.isType() && !ni.isType()) {
				std::cerr << ":o" << std::endl;
				system("pause");
			}
		} else {
			Node n = nodes[nodeMapping[j]];
			cover[ni].insert(n);
			cover[n].insert(ni);
			if (n.isType() && ni.isType() || !n.isType() && !ni.isType()) {
				std::cerr << ":o2" << std::endl;
				system("pause");
			}
		}
	}

	for (int i = 0; i < n; i++) {
		delete[] table[i];
	}
	delete[] table;
	delete assignments;

	return cover;
}

void testFindMinimalEdgeCover() {
	Graph g(5);
	Node a(true, 0);
	Node b(true, 1);
	Node c(true, 2);
	Node x(false, 0);
	Node y(false, 1);
	g.addNode(a);
	g.addNode(b);
	g.addNode(c);
	g.addNode(x);
	g.addNode(y);
	g.init();
	g.addEdge(a, x, 10);
	g.addEdge(b, x, 1);
	g.addEdge(b, y, 3);
	g.addEdge(c, y, 5);

	g.updateNeisCount();

	std::map<Node, std::set<Node>> cover = findMinimalEdgeCover(g);
	for (std::map<Node, std::set<Node>>::iterator it = cover.begin(); it != cover.end(); it++) {
		if (it->first.isType()) {
			for (std::set<Node>::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
				if (jt->isType()) {
					std::cerr << "should not happen" << std::endl;
					throw "";
				}
				int measurementIndex = it->first.getTypeIndex();
				int trackIndex = jt->getTypeIndex();
				std::cout << "Match: " << measurementIndex << ' ' << trackIndex << std::endl;
			}
		}
	}

	system("pause");
}
