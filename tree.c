#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#include "tree.h"

struct tree_struct {
	int32_t** tree;
	int32_t depth;
	int32_t* levelSize;
};

void die() {
	fprintf(stderr,"memory allocation failed\n");
	exit(1);
}

Tree init_tree(int32_t levels, int32_t* levelSizes) {
	Tree t;
	if( (t = (Tree)malloc(sizeof(struct tree_struct)) ) == NULL ) die();

	t->depth = levels;
	memcpy(t->levelSize, levelSizes, levels * sizeof(int32_t));

	return t;
}

void perform_insertions(Tree t, int32_t n) {
	int32_t i, j, samples[n];

	for(i=0;i<n;i++)
		samples[i] = (i+1)*10; // TODO: change to random generator

	// TODO: sort samples

	int32_t loadPerLevel[t->depth];
	memset(loadPerLevel,0,t->depth * sizeof(int32_t));

	int32_t l = t->depth - 1;

	// calculate insertions on each level
	// by simulating insertion algorithm
	for(i=0;i<n;i++) {
		if(l==-1) { fprintf(stderr,"Too much elements\n"); exit(1); }
		for(j=0;j<l;j++)
			printf("\t");
		printf("%d\n",samples[i]);
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
	
	// allocate space for each level
	int32_t levelArrSize[t->depth];
	if( posix_memalign((void**)&(t->tree),16,t->depth*sizeof(int32_t*)) != 0) die();
	for(i=0;i<t->depth;i++) {
		if (loadPerLevel[i]) // check if there is at least one element
			levelArrSize[i] = ( (loadPerLevel[i] / t->levelSize[i]) + (loadPerLevel[i] % t->levelSize[i] != 0) ) * t->levelSize[i];
		else // if not, create one element only
			levelArrSize[i] = t->levelSize[i];
		if( posix_memalign( (void**)&(t->tree[i]),16, levelArrSize[i]* sizeof(int32_t)) != 0) die();
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

	//printf("%d" ,t->levelSize[0]);

}

void perform_probes(Tree t, int32_t n){
	//testing purpose probe
	int32_t testprobe = 95;
	//array of probes to test;
	int32_t	probes[n];

	for(int i = 0; i < n; i++){
		//TODO: Set equal to random selection
		probes[i] = testprobe;
	}
	//start at root and traverse or add keys when passed by
	for(int i = 0; i < n; i++){

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
				if (probe < t->tree[depth][position] && (!atLeaf)){
					depth++;
					position = position * t->levelSize[depth];
					counter = 0;
				}
				else if((probe >= t->tree[depth][position]) && (!atLeaf)){
					identifier += getKeys(t, depth, position) + 1;
					position++;
					counter++;
				} 

				else if ((probe < t->tree[depth][position]) && (atLeaf)){
					found = true;
				}

				else if ((probe >= t->tree[depth][position]) && (atLeaf)){
					identifier++;
					position++;
					counter++;
				}
			} else{
				if(atLeaf){
					found = true;
				}
				else{
					depth++;
					position = position * t->levelSize[depth];
					counter = 0;
				}
			}

			

		}
		printf("For probe %d : %d \n", probe, identifier);
	}
}

//supposed to get total keys when passed a pointer
int getKeys(Tree t, int depth, int position){
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
	}

	return keys;




}








