#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include "tree.h"
#include "random.h"

#define int int32_t

struct tree_struct {
	int** tree;
	int depth;
	int* levelSize;
};

void die() {
	fprintf(stderr,"memory allocation failed\n");
	exit(1);
}

Tree init_tree(int levels, int* levelSizes) {
	Tree t;
	if( (t = (Tree)malloc(sizeof(struct tree_struct)) ) == NULL ) die();
	if( (t->levelSize = (int*)malloc(levels*sizeof(int)) ) == NULL ) die();

	t->depth = levels;
	memcpy(t->levelSize, levelSizes, levels * sizeof(int));

	return t;
}

void perform_insertions(Tree t, int n) {
	int i, j;

	rand32_t *gen = rand32_init(time(NULL));
	int* samples = generate_sorted_unique(n, gen);
	free(gen);

	int loadPerLevel[t->depth];
	memset(loadPerLevel,0,t->depth * sizeof(int));

	int l = t->depth - 1;

	// calculate insertions on each level
	// by simulating insertion algorithm
	for(i=0;i<n;i++) {
		if(l==-1) { fprintf(stderr,"Too much elements\n"); exit(1); }
		//for(j=0;j<l;j++)
		//	printf("\t");
		//printf("%d\n",samples[i]);
		loadPerLevel[l]++;
		if(loadPerLevel[l] % t->levelSize[l] == 0 &&
			( l == (t->depth - 1) || loadPerLevel[l+1] % ((t->levelSize[l]+1)*t->levelSize[l+1]) == 0 ) ) {
				l--;
				while( l >= 0 && loadPerLevel[l+1] % ((t->levelSize[l]+1)*t->levelSize[l+1]) == 0 )
					l--;
		} else
			while(l != (t->depth - 1))
				l++;
	}

	if(!loadPerLevel[i]) { fprintf(stderr,"Root node empty\n"); exit(1); }
	
	// allocate space for each level
	int levelArrSize[t->depth];
	if( posix_memalign((void**)&(t->tree),16,t->depth*sizeof(int*)) != 0) die();
	for(i=0;i<t->depth;i++) {
		//if (loadPerLevel[i]) // check if there is at least one element
		levelArrSize[i] = ( (loadPerLevel[i] / t->levelSize[i]) + (loadPerLevel[i] % t->levelSize[i] != 0) ) * t->levelSize[i];
		//else // if not, output error
			//levelArrSize[i] = t->levelSize[i];
		if( posix_memalign( (void**)&(t->tree[i]),16, levelArrSize[i]* sizeof(int)) != 0) die();
	}

	// insert into tree
	l = t->depth - 1;
	memset(loadPerLevel,0,t->depth * sizeof(int));
	for(i=0;i<n;i++) {
		t->tree[l][ loadPerLevel[l]++ ] = samples[i];
		if(loadPerLevel[l] % t->levelSize[l] == 0 &&
			( l == (t->depth - 1) || loadPerLevel[l+1] % ((t->levelSize[l]+1)*t->levelSize[l+1]) == 0 ) ) {
				l--;
				while( l >= 0 && loadPerLevel[l+1] % ((t->levelSize[l]+1)*t->levelSize[l+1]) == 0 )
					l--;
		} else
			while(l != (t->depth - 1))
				l++;
	}

	// pad with MAXINT
	for(i=0;i<t->depth;i++)
		for(j=loadPerLevel[i];j<levelArrSize[i];j++)
			t->tree[i][j] = INT_MAX;

	/*
	// pretty printer
	for(i=0;i<t->depth;i++) {
		printf("level %d: [ ",i);
		for(j=0;j<levelArrSize[i];j++) {
			printf("%d ",t->tree[i][j]);
			if( (j+1) % t->levelSize[i] == 0 && j != levelArrSize[i]-1)
				printf("] [ ");
		}
		printf("]\n");
	}
	*/

	free(samples);
}








