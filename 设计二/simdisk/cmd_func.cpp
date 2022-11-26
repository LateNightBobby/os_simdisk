#include"head.h"
//��ʾ����ϵͳ��Ϣ(�ο�Linux�ļ�ϵͳ��ϵͳ��Ϣ)���ļ����Ը����û����ж�д������Ŀ¼�����ļ���֧��ȫ·���������·������·�������������á�/��������
void info()
{
	char* text = new char[2 * max_path_length];
	write_shm("info��\n", false);

	write_shm("copyright @ 20�ƿ�2 ������ 202030442410\n", false);

	sprintf(text, "ʣ�����\t%d/%d\n", super_block.sb_free_block_num, super_block.sb_block_num);
	write_shm(text, false);


	sprintf(text, "ʣ��inode��(�������ļ���)\t%d/%d\n", super_block.sb_free_inode_num, super_block.sb_inode_num);
	write_shm(text, false);

	sprintf(text, "��ǰ·��: %s\n��ǰ�û���%s\n��ǰ����i����ţ�%d\n", sys_val.current_path, sys_val.current_user.u_name, sys_val.current_inode);
	write_shm(text, false);
	
	for (int i = 0; i < super_block.user_num; ++i)
	{
		sprintf(text, "%d: �û���: %s\t�û�ID: %d\n", i + 1, super_block.user[i].u_name, super_block.user[i].u_id);
		write_shm(text, false);
	}
	write_shm("�����ļ���Ϣ��\n", false);
	for (int i = 0; i < total_inode_num; ++i)
	{
		if (inode_bitmap[i] == used)
		{
			sprintf(text, "[\t�ļ�����:%s\t�ļ�����%s\t�ļ���С��%dB\t��дȨ��%s/%s\t������ID��%d\ti����:%d\t]\n", inode_table[i].i_ftype == normal_file ? "file" : "dir",
				inode_table[i].i_filename, inode_table[i].i_fsize, inode_table[i].rw.RW[0] ? "R" : "-", inode_table[i].rw.RW[1] ? "W" : "-", inode_table[i].i_uid, i);
			write_shm(text, false);
		}
	}
}
//�ı�Ŀ¼���ı䵱ǰ����Ŀ¼��Ŀ¼������ʱ����������Ϣ��
void cd(char* path_name)
{
	//printf("cd to %s\n", path_name);
	//����ָ��
	if (!strcmp(path_name, ""))		//���ظ�Ŀ¼
	{
		strcpy_s(sys_val.current_path, "root/");
		sys_val.current_inode = rt_dir.inode_num;
		return;
	}
	if (!strcmp(path_name, ".")) return;
	if (!strcmp(path_name, ".."))
	{
		sys_val.current_inode = inode_table[sys_val.current_inode].parent_inode_num;
		int pos = strlen(sys_val.current_path) - 2;//ĩβ��/����
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
			printf("�������Ŀ¼\n");
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
//��ʾĿ¼����ʾָ��Ŀ¼�»�ǰĿ¼�µ���Ϣ�������ļ����������ַ�������롢�ļ����ȡ���Ŀ¼�ȣ���/s������dir�����ʾ������Ŀ¼��
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
			printf("�Ҳ���ָ��Ŀ¼���ܾ���ʾ\n");
			write_shm("�Ҳ���ָ��Ŀ¼���ܾ���ʾ\n", false);
		}
		delete temp, origin;
	}
	else {
		printf("dirָ���ʽ����\n");
		write_shm("dirָ���ʽ����\n", false);
	}
}
//����Ŀ¼����ָ��·����ǰ·���´���ָ��Ŀ¼������ʱ��������Ϣ
void md(char* path, char* dir_name)
{
	if (!strcmp(path, ""))
	{
		printf("��������\n");
		write_shm("md��������\n", false);
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
		printf("�Ҳ���ָ��·�����޷�����\n");
		write_shm("�Ҳ���ָ��·�����޷�����\n", false);
	}
	delete temp, origin;
}
//ɾ��Ŀ¼��ɾ��ָ��Ŀ¼�������ļ�����Ŀ¼��ҪɾĿ¼����ʱ��Ҫ������ʾ�Ƿ�Ҫɾ��
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
				printf("�޷�ɾ����Ŀ¼\n");
				write_shm("�޷�ɾ����Ŀ¼\n", false);
				return;
			}
			remove_dir(temp->current_inode);
		}
		else {
			printf("�ܾ�ɾ��\n");
			write_shm("�ܾ�ɾ��\n", false);
		}
	}
	else {
		printf("�Ҳ���ָ��Ŀ¼���޷�ɾ��\n");
		write_shm("�Ҳ���ָ��Ŀ¼���޷�ɾ��\n", false);
	}
	delete temp;
}
//�½��ļ���ͬ��ʱ��ʾ
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
				printf("�����ɹ�\n");
				write_shm("�����ɹ�\n", false);
			}
			else {
				printf("�����ļ�ʧ��\n");
				write_shm("�����ļ�ʧ��\n", false);
			}

			sys_val.current_inode = origin->current_inode;
			strcpy_s(sys_val.current_path, origin->current_path);
		}
		else {
			printf("�Ҳ���ָ��Ŀ¼���޷�����\n");
			write_shm("�Ҳ���ָ��Ŀ¼���޷�����\n", false);
		}
		delete temp, origin;
		return;
	}
	printf("newfile in current dir with name %s\n", path);
	if (try_create_file(path))
	{
		if (init) file_init(path);
		printf("�����ɹ�\n");
		write_shm("�����ɹ�\n", false);
	}
	else {
		printf("�����ļ�ʧ��\n");
		write_shm("�����ļ�ʧ��\n", false);
	}
}
//���ļ�;
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
	else { printf("�Ҳ����ļ�\n"); write_shm("�Ҳ����ļ�\n", false); }
}
//�����ļ�����֧��ģ��Linux�ļ�ϵͳ�ڲ����ļ������⣬��֧��host�ļ�ϵͳ��ģ��Linux�ļ�ϵͳ����ļ�������host�ļ�ϵͳ���ļ�����Ϊ<host>����
//�磺��windows��D���̵��ļ�\data\sample\test.txt�ļ�������ģ��Linux�ļ�ϵͳ�е� / test / dataĿ¼��windows��D���̵ĵ�ǰĿ¼ΪD��\data����ʹ�����
//simdisk copy <host>D��\data\sample\test.txt / test / data
//���ߣ�simdisk copy <host>D��sample\test.txt / test / data
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
	if (pos < 0)//��'/'
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
//ɾ���ļ���ɾ��ָ���ļ���������ʱ����������Ϣ
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
	printf("no such file, ����ʧ��\n");
	write_shm("no such file, ����ʧ��\n", false);
}
//��Ⲣ�ָ��ļ�ϵͳ�����ļ�ϵͳ�е�����һ���Խ��м�⣬���Զ������ļ�ϵͳ�Ľṹ����Ϣ��������������
void check()
{
	printf("do check\n");
	inode_compaction();
	write_back();
}
//��ȡ����ʹ�÷�ʽ
void help(char* cmd_name)
{
	if (!strcmp(cmd_name, ""))
	{
		write_shm("�������ѯ��ָ����\n������info, cd, dir, md, rd, newfile, cat, copy, del, check, help, exit\n", false);
		return;
	}
	int cmd_num = find_cmd(cmd_name);
	switch (cmd_num)
	{
	case 0:write_shm("info\n��ʾ����ϵͳ��Ϣ,�Լ��ļ���Ϣ\n", false); break;
	case 1:write_shm("cd [���·��/����·��]\n�л���Ŀ��Ŀ¼��\n", false); break;
	case 2:write_shm("dir [Ҫ��ʾ��Ŀ¼��Ĭ��Ϊ��ǰĿ¼��] [ѡ�� /s]\n��ʾָ��Ŀ¼�»�ǰĿ¼�µ���Ϣ�������ļ����������ַ�������롢�ļ����ȡ���Ŀ¼�ȣ�ѡ��Ϊ/s������dir�����ʾ������Ŀ¼��\n", false); break;
	case 3:write_shm("md [����Ŀ¼��λ�ã�Ĭ��Ϊ��ǰĿ¼��] [Ŀ¼��]\n����Ŀ¼����ָ��·����ǰ·���´���ָ��Ŀ¼������ʱ��������Ϣ\n", false); break;
	case 4:write_shm("rd [Ŀ¼��]\nɾ��Ŀ¼��ɾ��ָ��Ŀ¼�������ļ�����Ŀ¼��ҪɾĿ¼����ʱ��������ʾ�Ƿ�Ҫɾ��\n", false); break;
	case 5:write_shm("newfile [�ļ���]\n�ڵ�ǰĿ¼�´����ļ�����������ʱ��ʾ\n", false); break;
	case 6:write_shm("cat [�ļ���]\n��ʾ��ǰĿ¼��ָ���ļ��ĵ�����\n", false); break;
	case 7:write_shm("copy [Դ�ļ���·��] [Ŀ��λ��]\n��֧��ģ��Linux�ļ�ϵͳ�ڲ����ļ������⣬��֧��host�ļ�ϵͳ��ģ��Linux�ļ�ϵͳ����ļ�������host�ļ�ϵͳ���ļ�����Ϊ<host>��\n", false); break;
	case 8:write_shm("del [�ļ���]\nɾ����ǰĿ¼��ָ�����ļ���������ʱ����\n", false); break;
	case 9:write_shm("check\n��Ⲣ�ָ��ļ�ϵͳ�����ļ�ϵͳ�е�����һ���Խ��м�⣬���Զ������ļ�ϵͳ�Ľṹ����Ϣ��������������\n", false); break;
	case 10:write_shm("help [����ָ����]\n��ȡָ���ʹ�÷���������\n", false); break;
	case 11:write_shm("exit\n�˳�ϵͳ", false); break;
	default: write_shm("�����ڸ�ָ��\n", false);
		break;
	}
}