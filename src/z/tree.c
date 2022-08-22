#include "z.h"

#include "util.h"

static void
	ztree_find_counts(unsigned int *counts, unsigned char *lens)
{
	unsigned int	i;

	i = 0;
	while (i < 17)
	{
		counts[i] = 0;
		i += 1;
	}
	i = 0;
	while (i < 288)
	{
		counts[lens[i]] += 1;
		i += 1;
	}
	counts[0] = 0;
}

static void
	ztree_find_codes(unsigned int *codes, unsigned int *counts, unsigned char *lens)
{
	unsigned int	indices[17];
	unsigned int	i;

	indices[0] = 0;
	i = 1;
	while (i < 17)
	{
		indices[i] = indices[i - 1] + counts[i - 1];
		i += 1;
	}
	i = 0;
	while (i < 288)
	{
		if (lens[i] != 0)
			codes[indices[lens[i]]++] = i;
		i += 1;
	}
}

void
	ztree_init(t_ztree *tree, unsigned char *lens)
{
	ztree_find_counts(tree->counts, lens);
	ztree_find_codes(tree->codes, tree->counts, lens);
}

unsigned int
	ztree_get(t_ztree *tree, t_zbuf *zb)
{
	unsigned int	bits;
	unsigned int	code;
	unsigned int	offset;
	unsigned int	value;

	bits = 1;
	code = 0;
	offset = 0;
	value = zbuf_read(zb, 1);
	while (value - code >= tree->counts[bits])
	{
		rt_assert(bits < 16, "ztree_get: too many bits in huffman code");
		offset = offset + tree->counts[bits];
		code = (code + tree->counts[bits]) << 1;
		bits += 1;
		value = (value << 1) | zbuf_read(zb, 1);
	}
	return (tree->codes[offset + value - code]);
}