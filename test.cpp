#include<thread>
#include <unistd.h>


using namespace std;

void fun(){
	int i=1;

	while(i==1){

	}
}

int main(){
	
	for(int i=0; i<8; i++){
		thread(fun).detach();
	}

	sleep(100000);
	return 0;
}