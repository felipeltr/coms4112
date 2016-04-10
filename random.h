#ifndef _RANDOM_H_
#define _RANDOM_H_


typedef struct {
	size_t index;
	uint32_t num[625];
} rand32_t;

rand32_t *rand32_init(uint32_t x);

uint32_t rand32_next(rand32_t *s);

int32_t *generate(size_t n, rand32_t *gen);

int32_t *generate_sorted_unique(size_t n, rand32_t *gen);

#endif