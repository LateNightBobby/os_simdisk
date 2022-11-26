#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<string>

#include<iostream>
#include<Windows.h>
using namespace std;

#define SHARE_SIZE 2048	//共享内存大小
#define TARGET_PROC_NAME	L"simdisk"	//进程名

struct shm_msg
{
	bool to_shell;//目标进程是否为shell
	bool req_input;//是否等待另一端输入
	bool is_read;	//
	char buffer[SHARE_SIZE];
	shm_msg()
	{
		to_shell = true;
		//is_read = true;
		memset(buffer, 0, SHARE_SIZE);
	}
	void init()
	{
		is_read = true;
		to_shell = false;
		strcpy(buffer, "");
	}
};

HANDLE handle;
shm_msg* msg = new shm_msg();

int main()
{
	handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TARGET_PROC_NAME);
	if (handle == NULL)
	{
		printf("共享内存打开失败\n");
		exit(0);
	}
	//msg->init();
	msg = (shm_msg*)MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	printf("connected\n");

	//执行操作
	while (true)
	{
		if (msg->to_shell && !msg->is_read)
		{
			if (!strcmp(msg->buffer, "$wrong exit$"))
			{
				printf("出错退出\n");
				exit(0);
			}
			if (!strcmp(msg->buffer, "$user exit$"))
			{
				printf("正常退出\n");
				exit(0);
			}
			cout << msg->buffer;
			msg->is_read = true;
			strcpy(msg->buffer, "");
			if (msg->req_input) 
			{
				cin.getline(msg->buffer, SHARE_SIZE);
			}
			msg->to_shell = false;
		}
		Sleep(5);
	}

	UnmapViewOfFile(msg);
	CloseHandle(handle);

	return 0;
}