#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>


#include "tree.h"
#include "random.h"

struct tree_struct {
	int32_t** tree;
	int32_t depth;
	int32_t* levelSize;
};

//static int getKeys(Tree t, int depth, int position);

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
	//rand32_t *gen = rand32_init(5678);
	int32_t* samples = generate_sorted_unique(n, gen);
	free(gen);

	int32_t loadPerLevel[t->depth];
	memset(loadPerLevel,0,t->depth * sizeof(int32_t));

	int32_t l = t->depth - 1, lastl;

	// calculate insertions on each level
	// by simulating insertion algorithm
	for(i=0;i<n;i++) {
		//samples[i] = (i+1)*10;
		if(l==-1) { fprintf(stderr,"Too much elements\n"); exit(1); }

		/*
		// Pretty printer - uncomment to use it
		printf("-------------------------------%d\n",i); // range identifier
		for(j=0;j<l;j++)
			printf("\t");
		printf("%d\n",samples[i]);
		*/
		

		loadPerLevel[l]++;
		lastl = l;
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

	if(lastl < l)
		l = lastl+1;
		while(l < t->depth) {
			loadPerLevel[l]++;
			l++;
		}
	
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

void perform_probes(Tree t, int32_t probes[], int n, int32_t results[]) {

	//array of probes to test;

	

	//start at root and traverse or add keys when passed by
	int i;
	for(i = 0; i < n; i++){

		int rangeId = 0;
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
			if(counter <= t->levelSize[depth]){
				//if the probe is less than the current position, traverse down and left
				if (probe <= t->tree[depth][position] && (!atLeaf)){
					rangeId += position;
					position = ((position/t->levelSize[depth])*(t->levelSize[depth]+1) + 
						position%t->levelSize[depth]) * (t->levelSize[depth+1]);
					depth++;
					counter = 0;
				}
				//if probe is greater than position, increment the position
				else if((probe > t->tree[depth][position]) && (!atLeaf)){
					//get keys and update identifier accordingly 
					//identifier += getKeys(t, depth, position)+1;
					position++;
					counter++;
				} 
				//if probe is less than leaf, found the identifier
				else if ((probe <= t->tree[depth][position]) && (atLeaf)){
					rangeId += position;
					found = true;
				}
				//if probe is greater move along the leaf
				else if ((probe > t->tree[depth][position]) && (atLeaf)){
					identifier++;
					position++;
					counter++;
					
				}
			} else{
				//if end of node and found
				if(atLeaf){
					rangeId += position;
					found = true;
				}
				//if not at leaf, traverse down right most leaf
				else{
					rangeId += position;
					position = ((position/t->levelSize[depth])*(t->levelSize[depth]+1) + 
						position%t->levelSize[depth]-1) * (t->levelSize[depth+1]);
					depth++;
					counter = 0;

				}
			}

		}
		
		//results[i][0]=probe;
		//results[i][1]=identifier;
		results[i]=rangeId;
	}
}


static void dump (__m128i v) {
  int n = 4;
  unsigned int *p1 = (unsigned int *)&v;
  while (n--) {
    printf("|%08x", *p1);
    p1++;
  }
  printf("\t");
  p1 = (unsigned int *)&v;
  n=4;
  while (n--) {
    printf("|%d", *p1);
    p1++;
  }
  printf("\n");
}


void perform_probes_simd(Tree t, int32_t probes[], int n, int32_t results[]) {

	int i, lv, rangeId;
	unsigned int pos = 0, r = 0;
	register __m128i probe; // SWITCH TO REGISTER LATER
	__m128i lvla, lvlb, lvlc, lvld, cmpa, cmpb, cmpc, cmpd;
	for(i = 0; i < n; i++){
		probe = _mm_set1_epi32(probes[i]);

		lv = 0;
		rangeId = 0;
		pos = 0;
		r = 0;
		//printf("-> probing %d\n",probes[i]);
		//dump(probe);
		while(lv < t->depth) {
			switch(t->levelSize[lv]) {
				case 4: // fanout 5
					pos <<= 2;
					//printf("lv %d - fanout 5 - pos %d\n",lv,pos);

					lvla = _mm_load_si128((__m128i *)&(t->tree[lv][pos]));
					//dump(lvla);
					cmpa = _mm_cmpgt_epi32(probe,lvla);
					//dump(cmpa);
					r = 0x10000 | ~_mm_movemask_epi8(cmpa);
					// printf("hex: %08x\n",r);
					r = __builtin_ctz(r) >> 2; // offset inside node
					// printf("offset: %d\n",r);
					rangeId += pos + r;
					pos += (pos >> 2) + r;
					lv++;
				break;

				case 8: // fanout 9
					pos <<= 3;
					// printf("lv %d - fanout 9\n",lv);
					lvla = _mm_load_si128((__m128i *)&(t->tree[lv][pos]));
					lvlb = _mm_load_si128((__m128i *)&(t->tree[lv][pos+4]));
					cmpa = _mm_cmpgt_epi32(probe,lvla);
					cmpb = _mm_cmpgt_epi32(probe,lvlb);
					//dump(cmpa);
					//dump(cmpb);
					cmpa = _mm_packs_epi32(cmpa,cmpb);
					//dump(cmpa);
					r = 0x10000 | ~_mm_movemask_epi8(cmpa);
					// printf("hex: %08lx\n",r);
					r = __builtin_ctz(r) >> 1; // offset inside node
					// printf("offset: %ld\n",r);

					rangeId += pos + r;
					pos += (pos >> 3) + r;
					lv++;


				break;

				case 16: // fanout 17
					pos <<= 4;
					// printf("lv %d - fanout 17 - pos %d\n",lv,pos);
					// printf("%d\n",t->tree[lv][pos]);
					lvla = _mm_load_si128((__m128i *)&(t->tree[lv][pos]));
					lvlb = _mm_load_si128((__m128i *)&(t->tree[lv][pos+4]));
					lvlc = _mm_load_si128((__m128i *)&(t->tree[lv][pos+8]));
					lvld = _mm_load_si128((__m128i *)&(t->tree[lv][pos+12]));
					// dump(lvla);
					// dump(lvlb);
					// dump(lvlc);
					// dump(lvld);
					cmpa = _mm_cmpgt_epi32(probe,lvla);
					cmpb = _mm_cmpgt_epi32(probe,lvlb);
					cmpc = _mm_cmpgt_epi32(probe,lvlc);
					cmpd = _mm_cmpgt_epi32(probe,lvld);
					// dump(cmpa);
					// dump(cmpb);
					// dump(cmpc);
					// dump(cmpd);
					cmpa = _mm_packs_epi32(cmpa,cmpb);
					cmpc = _mm_packs_epi32(cmpc,cmpd);
					// dump(cmpa);
					// dump(cmpc);
					cmpa = _mm_packs_epi16(cmpa,cmpc);
					// dump(cmpa);
					r = 0x10000 | ~_mm_movemask_epi8(cmpa);
					//printf("hex: %08lx\n",r);
					r = __builtin_ctz(r);
					//printf("offset: %ld\n",r);

					rangeId += pos + r;
					pos += (pos >> 4) + r;
					lv++;

				break;
			}
			//break;
		}

		results[i] = rangeId;
	}

}





/*
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
*/

void perform_probes_hardcode(Tree t, int32_t probes[], int n, int32_t results[]){
	int i, rangeId, rangeId2, rangeId3, rangeId4;
	unsigned int pos = 0, r = 0, pos2 = 0, pos3 = 0, pos4 = 0;
	__m128i lvla, lvlb, cmpa, cmpb;
	//go through probes 4 at a time
	//root node loaded
	__m128i lvlaroot = _mm_load_si128((__m128i *)&(t->tree[0][0]));
	__m128i lvlbroot = _mm_load_si128((__m128i *)&(t->tree[0][4]));

	for(i = 0; i < n; i = i+4){
		rangeId = 0;
		rangeId2 = 0;
		rangeId3 = 0;
		rangeId4 = 0;
		pos = 0;
		pos2 = 0;
		pos3 = 0;
		pos4 = 0;
		r = 0;

		
		__m128i k = _mm_load_si128((__m128i*)&(probes[i]));
		register __m128i k1 = _mm_shuffle_epi32(k, _MM_SHUFFLE(0,0,0,0));
		register __m128i k2 = _mm_shuffle_epi32(k, _MM_SHUFFLE(1,1,1,1));
		register __m128i k3 = _mm_shuffle_epi32(k, _MM_SHUFFLE(2,2,2,2));
		register __m128i k4 = _mm_shuffle_epi32(k, _MM_SHUFFLE(3,3,3,3));
		
		//__m128i k  = _mm_set1_epi32(probes[i]);


		//for root
		cmpa = _mm_cmpgt_epi32(k1,lvlaroot);
		cmpb = _mm_cmpgt_epi32(k1,lvlbroot);
		cmpa = _mm_packs_epi32(cmpa,cmpb);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 1; // offset inside node
		rangeId += pos + r;
		pos += (pos >> 3) + r;

		cmpa = _mm_cmpgt_epi32(k2,lvlaroot);
		cmpb = _mm_cmpgt_epi32(k2,lvlbroot);
		cmpa = _mm_packs_epi32(cmpa,cmpb);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 1; // offset inside node
		rangeId2 += pos + r;
		pos2 += (pos >> 3) + r;

		cmpa = _mm_cmpgt_epi32(k3,lvlaroot);
		cmpb = _mm_cmpgt_epi32(k3,lvlbroot);
		cmpa = _mm_packs_epi32(cmpa,cmpb);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 1; // offset inside node
		rangeId3 += pos + r;
		pos3 += (pos >> 3) + r;

		cmpa = _mm_cmpgt_epi32(k4,lvlaroot);
		cmpb = _mm_cmpgt_epi32(k4,lvlbroot);
		cmpa = _mm_packs_epi32(cmpa,cmpb);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 1; // offset inside node
		rangeId4 += pos + r;
		pos4 += (pos >> 3) + r;



		//for middle 
		pos <<= 2;
		lvla = _mm_load_si128((__m128i *)&(t->tree[1][pos]));
		cmpa = _mm_cmpgt_epi32(k1,lvla);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 2; // offset inside node
		rangeId += pos + r;
		pos += (pos >> 2) + r;

		pos2 <<= 2;
		lvla = _mm_load_si128((__m128i *)&(t->tree[1][pos2]));
		cmpa = _mm_cmpgt_epi32(k2,lvla);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 2; // offset inside node
		rangeId2 += pos2 + r;
		pos2 += (pos2 >> 2) + r;

		pos3 <<= 2;
		lvla = _mm_load_si128((__m128i *)&(t->tree[1][pos3]));
		cmpa = _mm_cmpgt_epi32(k3,lvla);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 2; // offset inside node
		rangeId3 += pos3 + r;
		pos3 += (pos3 >> 2) + r;

		pos4 <<= 2;
		lvla = _mm_load_si128((__m128i *)&(t->tree[1][pos4]));
		cmpa = _mm_cmpgt_epi32(k4,lvla);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 2; // offset inside node
		rangeId4 += pos4 + r;
		pos4 += (pos4 >> 2) + r;



		//for leaf
		pos <<= 3;
		lvla = _mm_load_si128((__m128i *)&(t->tree[2][pos]));
		lvlb = _mm_load_si128((__m128i *)&(t->tree[2][pos+4]));
		cmpa = _mm_cmpgt_epi32(k1,lvla);
		cmpb = _mm_cmpgt_epi32(k1,lvlb);
		cmpa = _mm_packs_epi32(cmpa,cmpb);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 1;
		rangeId += pos + r;
		results[i] = rangeId;

		pos2 <<= 3;
		lvla = _mm_load_si128((__m128i *)&(t->tree[2][pos2]));
		lvlb = _mm_load_si128((__m128i *)&(t->tree[2][pos2+4]));
		cmpa = _mm_cmpgt_epi32(k2,lvla);
		cmpb = _mm_cmpgt_epi32(k2,lvlb);
		cmpa = _mm_packs_epi32(cmpa,cmpb);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 1;
		rangeId2 += pos2 + r;
		results[i+1] = rangeId2;

		pos3 <<= 3;
		lvla = _mm_load_si128((__m128i *)&(t->tree[2][pos3]));
		lvlb = _mm_load_si128((__m128i *)&(t->tree[2][pos3+4]));
		cmpa = _mm_cmpgt_epi32(k3,lvla);
		cmpb = _mm_cmpgt_epi32(k3,lvlb);
		cmpa = _mm_packs_epi32(cmpa,cmpb);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 1;
		rangeId3 += pos3 + r;
		results[i+2] = rangeId3;

		pos4 <<= 3;
		lvla = _mm_load_si128((__m128i *)&(t->tree[2][pos4]));
		lvlb = _mm_load_si128((__m128i *)&(t->tree[2][pos4+4]));
		cmpa = _mm_cmpgt_epi32(k4,lvla);
		cmpb = _mm_cmpgt_epi32(k4,lvlb);
		cmpa = _mm_packs_epi32(cmpa,cmpb);
		r = 0x10000 | ~_mm_movemask_epi8(cmpa);
		r = __builtin_ctz(r) >> 1;
		rangeId4 += pos4 + r;
		results[i+3] = rangeId4;
	}

	
}


void destroy_tree(Tree t) {
	int i;
	for(i=0;i<t->depth;i++)
		free(t->tree[i]);
	free(t->tree);
	free(t->levelSize);
	free(t);
}









