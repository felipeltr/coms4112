#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

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
	int32_t results[p];

	for(i=0;i<total;i++)
		levelSizes[i] = atoi(argv[i+3])-1;

	Tree t = init_tree(total, levelSizes);

	perform_insertions(t,k);

	rand32_t *gen = rand32_init(time(NULL));
	int32_t* probes = generate_sorted_unique(p, gen);
	free(gen);

	for (i=0;i<p;i++)
		probes[i] = i*10 + 5;

	clock_t s1 = clock();
	perform_probes(t, probes, p, results);
	clock_t e1 = clock();

	for(i=0;i<p;i++)
		printf("Probe %d -> rangeid %d\n",probes[i],results[i]);


	printf("------\n");
	clock_t s2 = clock();
	perform_probes_simd(t, probes, p, results);
	clock_t e2 = clock();

	for(i=0;i<p;i++)
		printf("Probe %d -> rangeid %d\n",probes[i],results[i]);

	printf("Total time for regular operation: %f s\n",((float)(e1-s1))/CLOCKS_PER_SEC);
	printf("Total time for SIMD operation: %f s\n",((float)(e2-s2))/CLOCKS_PER_SEC);

	free(probes);
	destroy_tree(t);

	return 0;
}