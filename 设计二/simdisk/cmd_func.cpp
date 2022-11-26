#include"head.h"
//显示整个系统信息(参考Linux文件系统的系统信息)，文件可以根据用户进行读写保护。目录名和文件名支持全路径名和相对路径名，路径名各分量间用“/”隔开。
void info()
{
	char* text = new char[2 * max_path_length];
	write_shm("info：\n", false);

	write_shm("copyright @ 20计科2 王政扬 202030442410\n", false);

	sprintf(text, "剩余块数\t%d/%d\n", super_block.sb_free_block_num, super_block.sb_block_num);
	write_shm(text, false);


	sprintf(text, "剩余inode数(可增加文件数)\t%d/%d\n", super_block.sb_free_inode_num, super_block.sb_inode_num);
	write_shm(text, false);

	sprintf(text, "当前路径: %s\n当前用户：%s\n当前所在i结点编号：%d\n", sys_val.current_path, sys_val.current_user.u_name, sys_val.current_inode);
	write_shm(text, false);
	
	for (int i = 0; i < super_block.user_num; ++i)
	{
		sprintf(text, "%d: 用户名: %s\t用户ID: %d\n", i + 1, super_block.user[i].u_name, super_block.user[i].u_id);
		write_shm(text, false);
	}
	write_shm("所有文件信息：\n", false);
	for (int i = 0; i < total_inode_num; ++i)
	{
		if (inode_bitmap[i] == used)
		{
			sprintf(text, "[\t文件类型:%s\t文件名：%s\t文件大小：%dB\t读写权限%s/%s\t所有者ID：%d\ti结点号:%d\t]\n", inode_table[i].i_ftype == normal_file ? "file" : "dir",
				inode_table[i].i_filename, inode_table[i].i_fsize, inode_table[i].rw.RW[0] ? "R" : "-", inode_table[i].rw.RW[1] ? "W" : "-", inode_table[i].i_uid, i);
			write_shm(text, false);
		}
	}
}
//改变目录：改变当前工作目录，目录不存在时给出出错信息。
void cd(char* path_name)
{
	//printf("cd to %s\n", path_name);
	//基础指令
	if (!strcmp(path_name, ""))		//返回根目录
	{
		strcpy_s(sys_val.current_path, "root/");
		sys_val.current_inode = rt_dir.inode_num;
		return;
	}
	if (!strcmp(path_name, ".")) return;
	if (!strcmp(path_name, ".."))
	{
		sys_val.current_inode = inode_table[sys_val.current_inode].parent_inode_num;
		int pos = strlen(sys_val.current_path) - 2;//末尾的/跳过
		for (; pos >= 0; --pos)
		{
			if (sys_val.current_path[pos] == '/')
				break;
		}
		if (pos >= 0) {
			char temp_dir[max_file_name_length];
			strncpy_s(temp_dir, sys_val.current_path, pos + 1);
			strcpy_s(sys_val.current_path, temp_dir);
			return;
		}
		else {
			printf("已在最顶层目录\n");
			return;
		}
	}
	system_val* temp = new system_val;
	temp->init(sys_val);

	if (find_path(path_name, temp))
	{
		sys_val.current_inode = temp->current_inode;
		strcpy_s(sys_val.current_path, temp->current_path);
	}
	else { 
		printf("no such path\n"); 
		write_shm("no such path\n", false);
	}
	delete temp;
}
//显示目录：显示指定目录下或当前目录下的信息，包括文件名、物理地址、保护码、文件长度、子目录等（带/s参数的dir命令，显示所有子目录）
void dir(char* path, char* opt)
{
	if (!strcmp(path, "") && !strcmp(opt, "")) show_dir(sys_val.current_inode, false, 0);
	else if (!strcmp(path, "/s") && !strcmp(opt, "")) show_dir(sys_val.current_inode, true, 0);
	else if (strcmp(path, ""))
	{
		system_val* temp = new system_val;
		system_val* origin = new system_val;
		temp->init(sys_val);
		origin->init(sys_val);
		if (find_path(path, temp))
		{
			sys_val.current_inode = temp->current_inode;
			strcpy_s(sys_val.current_path, temp->current_path);

			show_dir(sys_val.current_inode, !strcmp(opt, "/s"), 0);

			sys_val.current_inode = origin->current_inode;
			strcpy_s(sys_val.current_path, origin->current_path);
		}
		else {
			printf("找不到指定目录，拒绝显示\n");
			write_shm("找不到指定目录，拒绝显示\n", false);
		}
		delete temp, origin;
	}
	else {
		printf("dir指令格式错误\n");
		write_shm("dir指令格式错误\n", false);
	}
}
//创建目录：在指定路径或当前路径下创建指定目录。重名时给出错信息
void md(char* path, char* dir_name)
{
	if (!strcmp(path, ""))
	{
		printf("参数过少\n");
		write_shm("md参数过少\n", false);
		return;
	}
	if (!strcmp(dir_name, ""))
	{
		create_dir(path);
		//cd(path);
		return;
	}	
	system_val* temp = new system_val;
	system_val* origin = new system_val;
	temp->init(sys_val);
	origin->init(sys_val);
	if (find_path(path, temp))
	{
		sys_val.current_inode = temp->current_inode;
		strcpy_s(sys_val.current_path, temp->current_path);
		create_dir(dir_name);

		sys_val.current_inode = origin->current_inode;
		strcpy_s(sys_val.current_path, origin->current_path);
	}
	else {
		printf("找不到指定路径，无法创建\n");
		write_shm("找不到指定路径，无法创建\n", false);
	}
	delete temp, origin;
}
//删除目录：删除指定目录下所有文件和子目录。要删目录不空时，要给出提示是否要删除
void rd(char* dir_name)
{
	system_val* temp = new system_val;
	temp->init(sys_val);
	if (find_path(dir_name, temp))
	{		
		if (try_remove(temp->current_inode))
		{
			printf("remove dir with name %s\n", dir_name);

			if (temp->current_inode == 0)
			{
				printf("无法删除根目录\n");
				write_shm("无法删除根目录\n", false);
				return;
			}
			remove_dir(temp->current_inode);
		}
		else {
			printf("拒绝删除\n");
			write_shm("拒绝删除\n", false);
		}
	}
	else {
		printf("找不到指定目录，无法删除\n");
		write_shm("找不到指定目录，无法删除\n", false);
	}
	delete temp;
}
//新建文件，同名时提示
void newfile(char* path, char* file_name, bool init)
{
	if (strcmp(file_name, "")) {
		system_val* temp = new system_val;
		system_val* origin = new system_val;
		temp->init(sys_val);
		origin->init(sys_val);
		if (find_path(path, temp))
		{
			sys_val.current_inode = temp->current_inode;
			strcpy_s(sys_val.current_path, temp->current_path);

			printf("newfile: path : %s  with name %s\n", path, file_name);
			if (try_create_file(file_name))
			{
				if(init) file_init(file_name);
				printf("创建成功\n");
				write_shm("创建成功\n", false);
			}
			else {
				printf("创建文件失败\n");
				write_shm("创建文件失败\n", false);
			}

			sys_val.current_inode = origin->current_inode;
			strcpy_s(sys_val.current_path, origin->current_path);
		}
		else {
			printf("找不到指定目录，无法创建\n");
			write_shm("找不到指定目录，无法创建\n", false);
		}
		delete temp, origin;
		return;
	}
	printf("newfile in current dir with name %s\n", path);
	if (try_create_file(path))
	{
		if (init) file_init(path);
		printf("创建成功\n");
		write_shm("创建成功\n", false);
	}
	else {
		printf("创建文件失败\n");
		write_shm("创建文件失败\n", false);
	}
}
//打开文件;
void cat(char* file_name)
{
	printf("show file named %s\n", file_name);
	char* content = read_file(file_name);
	if (strcmp(content, "")) {
		//printf("%d\n", strlen(content));
		//printf("%s\n", content);
		char* text = new char[sizeof(content) + max_path_length];
		sprintf(text, "in file %s:\n%s\n", file_name, content);
		write_shm(text, false);
	}
	else { printf("找不到文件\n"); write_shm("找不到文件\n", false); }
}
//拷贝文件，除支持模拟Linux文件系统内部的文件拷贝外，还支持host文件系统与模拟Linux文件系统间的文件拷贝，host文件系统的文件命名为<host>…，
//如：将windows下D：盘的文件\data\sample\test.txt文件拷贝到模拟Linux文件系统中的 / test / data目录，windows下D：盘的当前目录为D：\data，则使用命令：
//simdisk copy <host>D：\data\sample\test.txt / test / data
//或者：simdisk copy <host>D：sample\test.txt / test / data
void copy(char* source, char* target)
{
	printf("copy file from %s to %s\n", source, target);
	char src_location[7], tgt_location[7];
	strncpy_s(src_location, source, 6);
	strncpy_s(tgt_location, target, 6);
	*(src_location + 6) = '\0';
	*(tgt_location + 6) = '\0';

	char* content = new char[2 * block_size];
	if (!strcmp(src_location, "<host>"))
	{
		content = src_from_host(source);
	}
	else
	{
		system_val* temp = new system_val;
		temp->init(sys_val);
		content = get_src_file(source);
		sys_val.current_inode = temp->current_inode;
		strcpy_s(sys_val.current_path, temp->current_path);
		delete temp;
	}

	char file_name[max_file_name_length];
	int pos;
	for (pos = strlen(source) - 1; pos >= 0; --pos)
	{
		if (source[pos] == '/' || source[pos] == '\\')
		{
			break;
		}
	}
	if (pos < 0)//无'/'
	{
		strcpy_s(file_name, source);
	}
	else {
		strncpy_s(file_name, source + pos + 1, strlen(source) - pos - 1);
	}
	if (!strcmp(tgt_location, "<host>"))
	{
		char host_path[max_path_length];
		strncpy_s(host_path, target + 6, strlen(target) - 6);
		string s = host_path;
		s.append("\\");
		s.append(file_name);
		char res[max_path_length];
		s.copy(res, s.length(), 0);
		*(res + s.length()) = '\0';
		errno_t err = fopen_s(&fp, res, "w+");
		if (err != 0) { printf("cannot open %s\n", res); return; }
		fseek(fp, 0, SEEK_SET);
		fwrite(content, sizeof(char), strlen(content) + 1, fp);
		fclose(fp);
	}
	else
	{
		if (strcmp(content, ""))
		{
			//printf("%s in copy %d\n", content, strlen(content));
			system_val* origin = new system_val;
			system_val* temp = new system_val;
			temp->init(sys_val);
			origin->init(sys_val);
			if (find_path(target, temp)) {
				newfile(target, file_name, 0);
				cd(target);
				do_copy(file_name, content);
				sys_val.current_inode = origin->current_inode;
				strcpy_s(sys_val.current_path, origin->current_path);
			}
			else { printf("no path\n"); write_shm("no path\n", false); }
			delete temp;
		}
	}

}
//删除文件：删除指定文件，不存在时给出出错信息
void del(char* file_name)
{
	printf("delete file name %s\n", file_name);
	directory dir;
	errno_t err = fopen_s(&fp, disk_name, "rb");
	if (err != 0)
	{
		printf("can not open disk\n");
		write_shm("can not open disk\n", false);
		return;
	}
	fseek(fp, inode_table[sys_val.current_inode].i_start_addr, SEEK_SET);
	fread(&dir, sizeof(directory), 1, fp);
	fclose(fp);

	for (int i = 0; i < max_file_num; ++i)
	{
		if (dir.File_entry[i].exist && inode_table[dir.File_entry[i].inode_num].i_ftype == normal_file && !strcmp(dir.File_entry[i].file_name, file_name))
		{
			remove_file(dir.File_entry[i].inode_num);
			return;
		}
	}
	printf("no such file, 操作失败\n");
	write_shm("no such file, 操作失败\n", false);
}
//检测并恢复文件系统：对文件系统中的数据一致性进行检测，并自动根据文件系统的结构和信息进行数据再整理
void check()
{
	printf("do check\n");
	inode_compaction();
	write_back();
}
//获取命令使用方式
void help(char* cmd_name)
{
	if (!strcmp(cmd_name, ""))
	{
		write_shm("请输入查询的指令名\n包括：info, cd, dir, md, rd, newfile, cat, copy, del, check, help, exit\n", false);
		return;
	}
	int cmd_num = find_cmd(cmd_name);
	switch (cmd_num)
	{
	case 0:write_shm("info\n显示整个系统信息,以及文件信息\n", false); break;
	case 1:write_shm("cd [相对路径/绝对路径]\n切换到目标目录下\n", false); break;
	case 2:write_shm("dir [要显示的目录（默认为当前目录）] [选项 /s]\n显示指定目录下或当前目录下的信息，包括文件名、物理地址、保护码、文件长度、子目录等（选项为/s参数的dir命令，显示所有子目录）\n", false); break;
	case 3:write_shm("md [创建目录的位置（默认为当前目录）] [目录名]\n创建目录：在指定路径或当前路径下创建指定目录。重名时给出错信息\n", false); break;
	case 4:write_shm("rd [目录名]\n删除目录：删除指定目录下所有文件和子目录。要删目录不空时，给出提示是否要删除\n", false); break;
	case 5:write_shm("newfile [文件名]\n在当前目录下创建文件，遇到重名时提示\n", false); break;
	case 6:write_shm("cat [文件名]\n显示当前目录下指定文件的的内容\n", false); break;
	case 7:write_shm("copy [源文件及路径] [目标位置]\n除支持模拟Linux文件系统内部的文件拷贝外，还支持host文件系统与模拟Linux文件系统间的文件拷贝，host文件系统的文件命名为<host>…\n", false); break;
	case 8:write_shm("del [文件名]\n删除当前目录下指定的文件，不存在时报错\n", false); break;
	case 9:write_shm("check\n检测并恢复文件系统：对文件系统中的数据一致性进行检测，并自动根据文件系统的结构和信息进行数据再整理\n", false); break;
	case 10:write_shm("help [操作指令名]\n获取指令的使用方法及作用\n", false); break;
	case 11:write_shm("exit\n退出系统", false); break;
	default: write_shm("不存在该指令\n", false);
		break;
	}
}