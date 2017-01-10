#ifndef __TIMING__
#define __TIMING__

#include <sstream>

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

void setup(int millisecs);

class PrintReset{
	public:
	    PrintReset& operator << (std::stringstream &s);
};

class PrintCondReset{
	public:
	    PrintCondReset& operator << (std::stringstream &s);
};





#endif
