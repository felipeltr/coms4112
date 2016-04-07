#ifndef _TREE_H_
#define _TREE_H_

/* Tree reference type */
typedef struct tree_struct * Tree;

/**
	Creates tree.
	The tree contains 'levels' levels,
		where the k'th level has fanout levelSizes[k].
	Returns tree reference.
*/
Tree create_tree(int levels, int* levelSizes);

/**
	Inserts n random keys into tree t.
*/
void perform_insertions(Tree t, int n);

/**
	Perform n random probes into tree t.
	Results are stored in 'results', which
		must be an two-dimensional array of size 2xn.
		- results[0] has the n probes realized.
		- results[1] has the key for each probe, respectively.
*/
void perform_probes(Tree t, int n, int** results);

/*
	Destroys tree.
*/
void destroy_tree(Tree t);

#endif