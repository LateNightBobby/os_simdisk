#include "head.h"
bool login_success()
{
	printf("请输入用户名：\n");
	char u_name[max_file_name_length];
	std::cin >> u_name;
	int pos;
	for (pos = 0; pos < super_block.user_num; ++pos)
	{
		if (!strcmp(u_name, super_block.user[pos].u_name))
			break;
	}
	if (pos == super_block.user_num) { printf("用户名不存在\n"); return false; }
	printf("请输入密码\n");
	char u_pwd[max_file_name_length];
	std::cin >> u_pwd;
	if (strcmp(u_pwd, super_block.user[pos].u_pwd)) { printf("密码错误\n"); return false; }
	sys_val.current_user.u_id = super_block.user[pos].u_id;
	strcpy_s(sys_val.current_user.u_name, super_block.user[pos].u_name);
	strcpy_s(sys_val.current_user.u_pwd, super_block.user[pos].u_pwd);
	string ss;
	getline(std::cin, ss);
	return true;
}