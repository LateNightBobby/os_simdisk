#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<string>

#include<iostream>
#include<Windows.h>
using namespace std;

#define SHARE_SIZE 2048	//�����ڴ��С
#define TARGET_PROC_NAME	L"simdisk"	//������

struct shm_msg
{
	bool to_shell;//Ŀ������Ƿ�Ϊshell
	bool req_input;//�Ƿ�ȴ���һ������
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
		printf("�����ڴ��ʧ��\n");
		exit(0);
	}
	//msg->init();
	msg = (shm_msg*)MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	printf("connected\n");

	//ִ�в���
	while (true)
	{
		if (msg->to_shell && !msg->is_read)
		{
			if (!strcmp(msg->buffer, "$wrong exit$"))
			{
				printf("�����˳�\n");
				exit(0);
			}
			if (!strcmp(msg->buffer, "$user exit$"))
			{
				printf("�����˳�\n");
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