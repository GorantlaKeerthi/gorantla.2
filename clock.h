struct clock{
	unsigned int s;	 //seconds
	unsigned int ns; //nanoseconds
};

void clock_add_ns(struct clock * c, const unsigned int ns);
int clock_alarm(struct clock * c, struct clock * end);
