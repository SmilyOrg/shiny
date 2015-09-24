#ifdef _WIN32
#include <Windows.h>
void shSleep(int seconds) {
	Sleep(seconds*1000);
}
#else
#include <unistd.h>
void shSleep(int seconds) {
	sleep(seconds);
}
#endif