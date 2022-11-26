#include "head.h"
bool login_success()
{
	printf("请输入用户名：\n");

	write_shm("请输入用户名：\n", true);

	char u_name[max_file_name_length];
	//std::cin >> u_name;
	strcpy_s(u_name, read_shm().c_str());

	int pos;
	for (pos = 0; pos < super_block.user_num; ++pos)
	{
		if (!strcmp(u_name, super_block.user[pos].u_name))
			break;
	}
	if (pos == super_block.user_num) 
	{
		write_shm("用户名不存在\n", false);

		printf("用户名不存在\n"); 

		return false; 
	}
	printf("请输入密码\n");

	write_shm("请输入密码\n", true);

	char u_pwd[max_file_name_length];
	//std::cin >> u_pwd;
	strcpy_s(u_pwd, read_shm().c_str());

	if (strcmp(u_pwd, super_block.user[pos].u_pwd)) 
	{ 
		printf("密码错误\n"); 

		write_shm("密码错误\n", false);

		return false; 
	}
	sys_val.current_user.u_id = super_block.user[pos].u_id;
	strcpy_s(sys_val.current_user.u_name, super_block.user[pos].u_name);
	strcpy_s(sys_val.current_user.u_pwd, super_block.user[pos].u_pwd);
	write_shm("登录成功\n", false);
	return true;
}