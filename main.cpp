#define _CRT_SECURE_NO_WARNINGS
#include "head.h"

_super_block super_block;				//超级块
i_node inode_table[total_inode_num];	//i结点
bool block_bitmap[total_block_num];		//块的位图
bool inode_bitmap[total_inode_num];		//i结点位图
FILE* fp;								//指向磁盘文件的指针
system_val sys_val;						//系统变量
directory rt_dir;						//根目录

int main()
{
	initFile();//初始化磁盘
	loadDisk();//读取磁盘
	while (!login_success());//登录
	//读取指令
	while (true)
	{
		printf("%s @ %s>~#", sys_val.current_user.u_name, sys_val.current_path); 
		string in;
		getline(std::cin, in);
		stringstream ss(in);
		char command[max_CMD_length], arg1[max_path_length], arg2[max_path_length], arg3[max_file_name_length];
		ss >> command;
		int cmd_number = find_cmd(command);
		switch (cmd_number)
		{
		case -1:printf("%s命令不存在\n", command); continue;
		case 1: ss >> arg1; break;
		case 2: ss >> arg1; ss >> arg3; break;
		case 3: ss >> arg1; ss >> arg3; break;
		case 4: ss >> arg3; break;
		case 5: ss >> arg1; ss >> arg3; break;
		case 6: case 8: ss >> arg3; break; 
		case 7: ss >> arg1; ss >> arg2; break;
		case 10: ss >> arg3; break;
		case 11: write_back(); printf("退出\n"); exit(0);
		//default:printf("...\n");
		}
		switch (cmd_number)
		{
		case 0:info(); break;
		case 1:cd(arg1); break;
		case 2:dir(arg1, arg3); break;
		case 3:md(arg1, arg3); break;
		case 4:rd(arg3); break;
		case 5:newfile(arg1, arg3); break;
		case 6:cat(arg3); break;
		case 7:copy(arg1, arg2); break;
		case 8:del(arg3); break;
		case 9:check(); break;
		case 10:help(arg3); break;
		default:
			printf("???\n");
			break;
		}
		strcpy_s(arg1, "");
		strcpy_s(arg2, "");
		strcpy_s(arg3, "");
		//std::cout << command << endl << arg1 << endl << arg2 << endl;
	} 
}