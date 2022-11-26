#include "head.h"

void create_shm()
{
	handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_EXECUTE_READWRITE, 0, SHARE_SIZE, PROC_NAME);
	msg = (shm_msg*)MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, SHARE_SIZE);
	msg->init();
	printf("访问开始 \n");
}

void close_shm()
{
	UnmapViewOfFile(msg);
	CloseHandle(handle);
	printf("共享关闭\n");
}

//void write_shm(string content, bool req_input)
//{
//	while (!msg->is_read);
//	strcpy_s(msg->buffer, content.c_str());
//	msg->req_input = req_input;
//	msg->to_shell = true;
//	msg->is_read = false;
//	//Sleep(50);
//}

void write_shm(const char* content, bool req_input)
{
	cout << content;
	int epoch = strlen(content) / SHARE_SIZE;
	for (int i = 0; i <= epoch; ++i)
	{
		while (!msg->is_read) Sleep(5);

		strncpy(msg->buffer, content + i * SHARE_SIZE, SHARE_SIZE);

		msg->to_shell = true;
		msg->req_input = req_input;
		msg->is_read = false;
	}
	printf("finish\n");
}

string read_shm()
{
	while (msg->to_shell) Sleep(5);
	return msg->buffer;
}