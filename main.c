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

	perform_probes(t, probes, p, results);

	for(i=0;i<p;i++)
		printf("Probe %d -> rangeid %d\n",probes[i],results[i]);


	printf("------\n");
	perform_probes_simd(t, probes, p, results);

	for(i=0;i<p;i++)
		printf("Probe %d -> rangeid %d\n",probes[i],results[i]);

	free(probes);
	destroy_tree(t);

	return 0;
}