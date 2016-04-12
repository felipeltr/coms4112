#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "tree.h"
#include "random.h"

struct tree_struct {
	int32_t** tree;
	int32_t depth;
	int32_t* levelSize;
};

static int getKeys(Tree t, int depth, int position);

void die() {
	fprintf(stderr,"memory allocation failed\n");
	exit(1);
}

Tree init_tree(int32_t levels, int32_t* levelSizes) {
	Tree t;
	if( (t = (Tree)malloc(sizeof(struct tree_struct)) ) == NULL ) die();
	if( (t->levelSize = (int*)malloc(levels*sizeof(int)) ) == NULL ) die();

	t->depth = levels;
	memcpy(t->levelSize, levelSizes, levels * sizeof(int32_t));

	return t;
}

void perform_insertions(Tree t, int32_t n) {
	int32_t i, j;

	rand32_t *gen = rand32_init(time(NULL)^0xa4d8937d); // guarantess different seed
	int32_t* samples = generate_sorted_unique(n, gen);
	free(gen);

	int32_t loadPerLevel[t->depth];
	memset(loadPerLevel,0,t->depth * sizeof(int32_t));

	int32_t l = t->depth - 1;

	// calculate insertions on each level
	// by simulating insertion algorithm
	for(i=0;i<n;i++) {
		if(l==-1) { fprintf(stderr,"Too much elements\n"); exit(1); }

		/*
		// Pretty printer - uncomment to use it
		printf("-------------------------------%d\n",i); // range identifier
		for(j=0;j<l;j++)
			printf("\t");
		//printf("%d\n",samples[i]);
		*/
		

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

	if(!loadPerLevel[0]) { fprintf(stderr,"Root node empty\n"); exit(1); }
	
	// allocate space for each level
	int32_t levelArrSize[t->depth];
	if( posix_memalign((void**)&(t->tree),16,t->depth*sizeof(int32_t*)) != 0) die();
	for(i=0;i<t->depth;i++) {
		levelArrSize[i] = ( (loadPerLevel[i] / t->levelSize[i]) + (loadPerLevel[i] % t->levelSize[i] != 0) ) * t->levelSize[i];
		if( posix_memalign( (void**)&(t->tree[i]),16, levelArrSize[i]* sizeof(int)) != 0) die();
	}

	// insert into tree
	l = t->depth - 1;
	memset(loadPerLevel,0,t->depth * sizeof(int32_t));
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
	// another pretty printer
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

void perform_probes(Tree t, int n, int32_t results[][2]) {

	//array of probes to test;

	rand32_t *gen = rand32_init(time(NULL));
	int32_t* probes = generate_sorted_unique(n, gen);
	free(gen);

	//start at root and traverse or add keys when passed by
	int i;
	for(i = 0; i < n; i++){

		bool found = false;
		int probe = probes[i];
		int depth = 0;
		int position = 0;
		int identifier = 0;
		bool atLeaf = false;
		int counter = 0;
		while (found == false) {

			if(depth == (t->depth)-1){
				atLeaf = true;
			}

			//check if position is valid
			if(counter < t->levelSize[depth]){
				//if the probe is less than the current position, traverse down and left
				if (probe < t->tree[depth][position] && (!atLeaf)){

					position = ((position/t->levelSize[depth])*(t->levelSize[depth]+1) + 
						position%t->levelSize[depth]) * (t->levelSize[depth+1]);
					depth++;
					counter = 0;
				}
				//if probe is greater than position, increment the position
				else if((probe >= t->tree[depth][position]) && (!atLeaf)){
					//get keys and update identifier accordingly 
					identifier += getKeys(t, depth, position)+1;
					position++;
					counter++;
				} 
				//if probe is less than leaf, found the identifier
				else if ((probe < t->tree[depth][position]) && (atLeaf)){
					found = true;
				}
				//if probe is greater move along the leaf
				else if ((probe >= t->tree[depth][position]) && (atLeaf)){
					identifier++;
					position++;
					counter++;
					
				}
			} else{
				//if end of node and found
				if(atLeaf){
					found = true;
				}
				//if not at leaf, traverse down right most leaf
				else{
					position = ((position/t->levelSize[depth])*(t->levelSize[depth]+1) + 
						position%t->levelSize[depth]-1) * (t->levelSize[depth+1]);
					depth++;
					counter = 0;

				}
			}

		}
		
		results[i][0]=probe;
		results[i][1]=identifier;
	}
}

// get total keys when passed a pointer
static int getKeys(Tree t, int depth, int position) {
	depth++;
	position = position * t->levelSize[depth];
	bool atLeaf = false;
	int keys = 0;
	int counter = 0;

	if(depth == (t->depth)-1){
		atLeaf = true;
	}

	if(atLeaf)
		keys = t->levelSize[depth];
	else{
		while(counter <= t->levelSize[depth]){
			keys += getKeys(t,depth,position);
			position++;
			counter++;
		}
		keys+= t->levelSize[depth];
	}
	return keys;

}

void destroy_tree(Tree t) {
	int i;
	for(i=0;i<t->depth;i++)
		free(t->tree[i]);
	free(t->tree);
	free(t->levelSize);
	free(t);
}










