#define _CRT_SECURE_NO_WARNINGS
#include "head.h"

_super_block super_block;				//������
i_node inode_table[total_inode_num];	//i���
bool block_bitmap[total_block_num];		//���λͼ
bool inode_bitmap[total_inode_num];		//i���λͼ
FILE* fp;								//ָ������ļ���ָ��
system_val sys_val;						//ϵͳ����
directory rt_dir;						//��Ŀ¼

int main()
{
	initFile();//��ʼ������
	loadDisk();//��ȡ����
	while (!login_success());//��¼
	//��ȡָ��
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
		case -1:printf("%s�������\n", command); continue;
		case 1: ss >> arg1; break;
		case 2: ss >> arg1; ss >> arg3; break;
		case 3: ss >> arg1; ss >> arg3; break;
		case 4: ss >> arg3; break;
		case 5: ss >> arg1; ss >> arg3; break;
		case 6: case 8: ss >> arg3; break; 
		case 7: ss >> arg1; ss >> arg2; break;
		case 10: ss >> arg3; break;
		case 11: write_back(); printf("�˳�\n"); exit(0);
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