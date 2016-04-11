#include <stdio.h>
#include <stdlib.h>

#include "tree.h"

int main (int argc, char** argv) {
	int k, p;

	if(argc < 5) {
		perror("Wrong call");
		return 1;
	}

	k = atoi(argv[1]);
	p = atoi(argv[2]);

	int i, total = argc-3;
	int levelSizes[total];

	for(i=0;i<total;i++)
		levelSizes[i] = atoi(argv[i+3])-1;

	Tree t = init_tree(total, levelSizes);

	perform_insertions(t,k);

	//printf(t->depth);
	perform_probes(t, p);

	return 0;
}