#include <unistd.h>
#include "clock.h"

//nanoseconds in one second
#define MAX_NANOS 1000000000

//Check if alarm is past the current clock time
int clock_alarm(struct clock * c, struct clock * alarm){
	int rv = 0;

	/* Check if we have run more than endtime */
	if(	(c->s > alarm->s) ||
			((c->s == alarm->s) && (c->ns >= alarm->ns))	){
		rv = 1;
	}
	return rv;
}

//Add nano seconds to the clock
void clock_add_ns(struct clock * c, const unsigned int ns){
  c->ns = c->ns + ns;
  if(c->ns > MAX_NANOS){
    c->s = c->s + 1;
    c->ns = c->ns % MAX_NANOS;
  }
}
