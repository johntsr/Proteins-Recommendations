#include "Timing.h"

#include <iostream>

using namespace std;

volatile sig_atomic_t print_flag = 0;

void handle_alarm(int sig);

void setup(int millisecs){
	struct itimerval it_val;
	signal(SIGALRM, (void (*)(int)) handle_alarm);
	it_val.it_value.tv_sec =  millisecs/1000;
	it_val.it_value.tv_usec = (millisecs * 1000) % 1000000;
	it_val.it_interval = it_val.it_value;
	setitimer(ITIMER_REAL, &it_val, NULL);
}

void handle_alarm(int sig) {
    print_flag = 1;
}

PrintReset& PrintReset::operator << (stringstream &s) {
	print_flag = 0;
	std::cout << s.str() << endl;
	s.str("");
	return *this;
}

PrintCondReset& PrintCondReset::operator << (stringstream &s) {
	if( print_flag == 0 ){
		s.str("");
		return *this;
	}

	print_flag = 0;
	std::cout << s.str() << endl;
	s.str("");
	return *this;
}
