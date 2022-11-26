#include"head.h"
void execute_cmd()
{
	printf("%s @ %s>~#", sys_val.current_user.u_name, sys_val.current_path);

	char header[max_path_length];
	sprintf(header, "%s @ %s>~#", sys_val.current_user.u_name, sys_val.current_path);
	write_shm(header, true);
	string in = read_shm();
	//getline(std::cin, in);
	stringstream ss(in);
	char command[max_CMD_length], arg1[max_path_length], arg2[max_path_length], arg3[max_file_name_length];
	ss >> command;
	//printf("%s\n", command);
	int cmd_number = find_cmd(command);
	switch (cmd_number)
	{
	case -1:char text[20]; sprintf(text, "%s命令不存在\n", command); write_shm(text, false); return;
	case 1: ss >> arg1; break;
	case 2: ss >> arg1; ss >> arg3; break;
	case 3: ss >> arg1; ss >> arg3; break;
	case 4: ss >> arg3; break;
	case 5: ss >> arg1; ss >> arg3; break;
	case 6: case 8: ss >> arg3; break;
	case 7: ss >> arg1; ss >> arg2; break;
	case 10: ss >> arg3; break;
	case 11: write_back();
		printf("退出\n");
		write_shm("$user exit$", false);
		close_shm();//关闭共享停止通信
		exit(0);
		//default:printf("...\n");
	}
	switch (cmd_number)
	{
	case 0:info(); break;
	case 1:cd(arg1); break;
	case 2:dir(arg1, arg3); break;
	case 3:md(arg1, arg3); break;
	case 4:rd(arg3); break;
	case 5:newfile(arg1, arg3, true); break;
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
}