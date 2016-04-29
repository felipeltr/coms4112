#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>

#include "tree.h"
#include "random.h"

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
	int32_t *results;
	int simd = 1;
	int hardcode = 0;

	if( (results = (int32_t*)malloc(p*sizeof(int32_t)) ) == NULL ){
		perror("malloc failed\n");
		return 1;
	}

#ifndef _CLIC_ENV
	clock_t s3;
	clock_t e3;
#endif

	for(i=0;i<total;i++) {
		levelSizes[i] = atoi(argv[i+3])-1;
		simd = simd && (levelSizes[i]==4 || levelSizes[i]==8 || levelSizes[i]==16);
	}

	Tree t = init_tree(total, levelSizes);

	perform_insertions(t,k);

	rand32_t *gen = rand32_init(time(NULL));
	int32_t* probes = generate_sorted_unique(p, gen);
	free(gen);

#ifndef _CLIC_ENV
	clock_t s1 = clock();
#endif
	perform_probes(t, probes, p, results);
#ifndef _CLIC_ENV
	clock_t e1 = clock();
#endif
	for(i=0;i<p;i++)
		printf("Probe %d -> rangeid %d\n",probes[i],results[i]);

#ifndef _CLIC_ENV
	clock_t s2, e2;
#endif
	if(simd){

		printf("------\n");
#ifndef _CLIC_ENV
		s2 = clock();
#endif
		perform_probes_simd(t, probes, p, results);
#ifndef _CLIC_ENV
		e2 = clock();
#endif
		for(i=0;i<p;i++)
			printf("Probe %d -> rangeid %d\n",probes[i],results[i]);

		if(total == 3 && levelSizes[0] == 8 && levelSizes[1] == 4 && levelSizes[2] == 8){	
			hardcode = 1;
			
			// drops last probes if probes number is not multiple of 4
			int newp = p & 0xfffffffc;
#ifndef _CLIC_ENV			
			s3 = clock();
#endif
			perform_probes_hardcode(t,probes,newp,results);
#ifndef _CLIC_ENV
			e3 = clock();
#endif
			printf("------\n");
			for(i=0;i<4;i++)
				printf("Probe %d -> rangeid %d\n",probes[i],results[i]);
			
		}
	}

#ifndef _CLIC_ENV
	printf("Total time for regular operation: %f s\n",((float)(e1-s1))/CLOCKS_PER_SEC);
	if(simd) {
		printf("Total time for SIMD operation: %f s\n",((float)(e2-s2))/CLOCKS_PER_SEC);
		if(hardcode)
			printf("Total time for hardcode operation: %f s\n",((float)(e3-s3))/CLOCKS_PER_SEC);
	}
#endif

	free(probes);
	free(results);
	destroy_tree(t);

	return 0;
}