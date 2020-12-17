#include <unistd.h>
#include <stdio.h>
#include<stdlib.h>

#include<iostream>
#include <cstring>
#include <fstream>
#include <sys/time.h>
#include <pthread.h>


#define SAT_NUM 48
using namespace std;

pthread_t * threadArray;
int thread_num;
struct pthread_param
{
	int id;
};

void *thread_clear_file(void*arg)
{
	pthread_param *pm = (pthread_param*)arg;
	int satelliteId = pm->id;
	string file = "sat"+to_string(satelliteId)+".txt";
	ifstream in(file,ios::in);
	ofstream fout;
	fout.open(file,ios::trunc);
	fout.close();
	in.close();
	pthread_exit(NULL);
}

void *thread_read_file(void *arg)
{
	pthread_param *pm = (pthread_param*)arg;
	int satelliteId = pm->id;
	string file = "sat"+to_string(satelliteId)+".txt";
	ifstream in(file,ios::in);
	string line;
	while(getline(in,line))
	{
		system(line.c_str());   
	}

	ofstream fout;
	fout.open(file,ios::trunc);
	fout.close();
	in.close();
	pthread_exit(NULL);

} 
void thread_create()
{
	thread_num = SAT_NUM;
	pthread_param pms[thread_num];
	threadArray = new pthread_t[thread_num];
	for(int i = 0; i < thread_num; i++){
		pms[i].id = i+1;
		pthread_create(&threadArray[i],NULL,thread_read_file,&pms[i]);
		
	}
	for (int i = 0; i < thread_num; ++i)
	{
		pthread_join(threadArray[i],NULL);
	}

}

void thread_exit()
{
	thread_num = SAT_NUM;
	pthread_param pms[thread_num];	
	threadArray = new pthread_t[thread_num];
	for(int i = 0; i < thread_num; i++){
		pms[i].id = i+1;
		pthread_create(&threadArray[i],NULL,thread_clear_file,&pms[i]);
	}
	for (int i = 0; i < thread_num; ++i)
	{
		pthread_join(threadArray[i],NULL);
	}

}

int main(int argc,char *argv[])
{
		int routeMode = atoi(argv[1]);;
		if(routeMode==1)
		{
			cout<<"clear"<<endl;
			thread_exit();
		}
		else if(routeMode==2)
		{

			thread_create();
			
		}
	return 0;
}

