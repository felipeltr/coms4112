#ifndef _TREE_H_
#define _TREE_H_

#include <stdint.h>

/* Tree reference type */
typedef struct tree_struct * Tree;

/**
	Initializes tree.
	The tree contains 'levels' levels,
		where the k'th level has fanout levelSizes[k].
	Returns tree reference.
*/
Tree init_tree(int levels, int* levelSizes);

/**
	Inserts n random keys into tree t.
*/
void perform_insertions(Tree t, int n);

/**
	Perform n random probes into tree t.
	Results are stored in 'results', which
		must be an two-dimensional array of size nx2.
		- results[i][0] has the i'th probe realized.
		- results[i][1] has the range id for the i'th probe.
*/
void perform_probes(Tree t, int n, int32_t results[][2]);

/*
	Destroys tree.
*/
void destroy_tree(Tree t);

#endif