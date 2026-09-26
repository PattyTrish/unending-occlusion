static unsigned long next = 1;

int rand(void)
{
	next = 0x41C64E6D * next + 12345;
	return (next >> 16) & 0x7FFF;
}

void srand(unsigned int seed) { next = seed; }
