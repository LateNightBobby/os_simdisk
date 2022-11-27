#include"head.h"
int find_cmd(char* command) 
{
	for (int i = 0; i < num_CMD; ++i)
	{
		if (!(strcmp(command, CMD[i])))
		{
			return i;
		}
	}
	return -1;
}
directory read_dir(int num)
{
	directory dir;
	errno_t err = fopen_s(&fp, disk_name, "rb");
	if (err != 0)
	{
		printf("open fail\n");
		return dir;
	}
	fseek(fp, inode_table[num].i_start_addr, SEEK_SET);
	fread(&dir, sizeof(directory), 1, fp);
	fclose(fp);
	return dir;
}
void reload()
{
	errno_t err = fopen_s(&fp, disk_name, "rb");
	if (err != 0)
	{
		printf("open fail\n");
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fread(&super_block, sizeof(_super_block), 1, fp);
	fread(&inode_table, sizeof(i_node), total_inode_num, fp);
	fread(&block_bitmap, sizeof(bool), total_block_num, fp);
	fread(&inode_bitmap, sizeof(bool), total_inode_num, fp);
	fread(&sys_val, sizeof(sys_val), 1, fp);
	fread(&rt_dir, sizeof(directory), 1, fp);
	fclose(fp);
}
int select_inode()
{
	for (int i = 0; i < total_inode_num; ++i)//选择i结点
		if (inode_bitmap[i] == not_used)
			return i;
}
int select_first_block(int block_num)
{
	int cnt = 0;
	for (int i = 0; i < total_block_num; ++i)		//初次匹配找块
	{
		if (block_bitmap[i] == not_used)
		{
			++cnt;
			if (cnt == dir_size)
				return (i - cnt + 1);
		}
		else cnt = 0;
	}
}
void create_dir(char* dir_name)//当前目录下创建目录
{
	if (strcmp(dir_name, "root"))
	{
		directory curr_dir = read_dir(sys_val.current_inode);

		for (int i = 0; i < max_file_num; ++i)
		{
			if (curr_dir.File_entry[i].exist && !strcmp(dir_name, curr_dir.File_entry[i].file_name) && inode_table[curr_dir.File_entry[i].inode_num].i_ftype == dir_file)
			{
				printf("已有同名目录，无法创建\n");
				return;
			}
		}
	}

	int first_block_pos = select_first_block(dir_size), inode_number = select_inode();//可用的第一个块编号,i结点号

	//i结点赋值
	inode_table[inode_number].i_blocknum = dir_size;
	inode_table[inode_number].i_first_block = first_block_pos;
	inode_table[inode_number].i_fsize = sizeof(directory);
	inode_table[inode_number].i_ftype = dir_file;
	inode_table[inode_number].i_is_changed = false;
	inode_table[inode_number].i_start_addr = first_data_addr + first_block_pos * block_size;
	inode_table[inode_number].i_uid = sys_val.current_user.u_id;
	strcpy_s(inode_table[inode_number].i_filename, dir_name);
	inode_table[inode_number].parent_inode_num = sys_val.current_inode;
	//超级块修改
	super_block.sb_free_block_num -= dir_size;
	super_block.sb_free_inode_num -= 1;
	//位图修改
	for (int i = 0; i < dir_size; i++)
		block_bitmap[first_block_pos + i] = used;
	inode_bitmap[inode_number] = used;

	//创建目录项
	directory dir;
	strcpy_s(dir.dir_name, dir_name);
	dir.file_num = 0;
	dir.inode_num = inode_number;
	//不是根目录则在父目录进行修改并加入入口
	if (strcmp(dir_name, "root"))
	{
		directory par_dir;
		errno_t err = fopen_s(&fp, disk_name, "rb+");
		if (err != 0)
		{
			printf("failed\n");
			return;
		}
		fseek(fp, inode_table[sys_val.current_inode].i_start_addr, SEEK_SET);
		fread(&par_dir, sizeof(directory), 1, fp);

		par_dir.file_num += 1;
		int pos;
		for (pos = 0; pos < max_file_num; ++pos)
		{
			if (!par_dir.File_entry[pos].exist)
			{
				strcpy_s(par_dir.File_entry[pos].file_name, dir_name);
				par_dir.File_entry[pos].inode_num = inode_number;
				par_dir.File_entry[pos].exist = true;
				break;
			}
		}
		if (pos == max_file_num)
		{
			fclose(fp);
			printf("目录已满，无法新建\n");
			return;
		}
		//strcpy_s(par_dir.File_entry[par_dir.file_num - 1].file_name, dir_name);
		//par_dir.File_entry[par_dir.file_num - 1].inode_num = inode_number;

		fseek(fp, inode_table[sys_val.current_inode].i_start_addr, SEEK_SET);
		fwrite(&par_dir, sizeof(directory), 1, fp);
		fclose(fp);
	}
	
	//修改后的写入
	errno_t err = fopen_s(&fp, disk_name, "rb+");	
	if (err != 0) {
		printf("cannot open \n");
		exit(0);
	}
	//std::cout << first_block_pos << endl;
	fseek(fp, 0, SEEK_SET);
	fwrite(&super_block, sizeof(_super_block), 1, fp);
	fwrite(&inode_table, sizeof(i_node), total_inode_num, fp);
	fwrite(&block_bitmap, sizeof(bool), total_block_num, fp);
	fwrite(&inode_bitmap, sizeof(bool), total_inode_num, fp);

	fseek(fp, inode_table[inode_number].i_start_addr, SEEK_SET);
	fwrite(&dir, sizeof(directory), 1, fp);
	fclose(fp);
}
bool find_path(char* path_name, system_val* temp)
{	
	if (!strcmp(path_name, ".") || !strcmp(path_name, "") || !strcmp(path_name, "/")) return true;
	//路径切割
	char next_dir[max_file_name_length];
	char rest_path[max_path_length];
	int pos;
	for (pos = 0; pos < strlen(path_name); ++pos)
		if (path_name[pos] == '/')
			break;
	if (pos < strlen(path_name))
	{
		strncpy_s(next_dir, path_name, pos - 1 >= 0 ? pos  : 0);
		if (pos == strlen(path_name) - 1) strcpy_s(rest_path, "");
		else strncpy_s(rest_path, path_name + pos + 1, strlen(path_name) - pos - 1);
	}
	else //无‘/’ 
	{
		strcpy_s(next_dir, path_name);
		strcpy_s(rest_path, "");
	}
	//std::cout << next_dir << ' ' << rest_path << endl;

	//判断情况
	if (!strcmp(next_dir, ".") || !strcmp(next_dir, "") || !strcmp(next_dir, "/"))
	{
		;
		//return find_path(rest_path, temp);
	}//当前目录
	else if (!strcmp(next_dir, ".."))
	{
		temp->current_inode = inode_table[temp->current_inode].parent_inode_num;
		int pos = strlen(rest_path) - 2;//末尾的/跳过
		for (; pos >= 0; --pos)
		{
			if (temp->current_path[pos] == '/')
				break;
		}
		if (pos >= 0) {
			char temp_dir[max_file_name_length];
			strncpy_s(temp_dir, temp->current_path, pos + 1);
			strcpy_s(temp->current_path, temp_dir);
			//return find_path(rest_path, temp);
		}
		else {
			printf("%s已在最顶层目录\n", path_name);
			//return find_path(rest_path, temp);
		}
	}//上一级目录
	else if (!strcmp(next_dir, "root"))
	{
		temp->current_inode = 0;
		strcpy_s(temp->current_path, "root/");
		//return true;
	}//根目录
	
	else {
		//读取当前目录
		directory dir = read_dir(sys_val.current_inode);

		//寻找对应路径
		for (int i = 0; i < max_file_num; ++i)
		{
			//std::cout << next_dir << ' ' << dir.File_entry[i].file_name << endl;
			if (dir.File_entry[i].exist && !strcmp(dir.File_entry[i].file_name, next_dir) && inode_table[dir.File_entry[i].inode_num].i_ftype == dir_file)//目录下有该路径
			{
				temp->current_inode = dir.File_entry[i].inode_num;
				strcat_s(temp->current_path, next_dir);
				strcat_s(temp->current_path, "/");
				return find_path(rest_path, temp);
			}
		}
		return false;
	}
	return find_path(rest_path, temp);
}
void show_dir(int inode_num, bool sub, const int level)
{
	directory dir = read_dir(inode_num);

	if (dir.file_num == 0)
	{
		printf("\n");
		return;
	}
	for (int i = 0; i < max_file_num; ++i)
	{
		if (dir.File_entry[i].exist) {
			printf("|");
			for (int j = 0; j < level; ++j) printf("__");
			printf("%s\t\ti结点号: %d\n", inode_table[dir.File_entry[i].inode_num].i_filename, dir.File_entry[i].inode_num);
			if (inode_table[dir.File_entry[i].inode_num].i_ftype == dir_file && sub) 
				show_dir(dir.File_entry[i].inode_num, sub, level + 1);
		}
	}
}
bool try_remove(int inode_num)
{
	directory dir = read_dir(inode_num);

	if (dir.file_num == 0) return true;//目录下为空，可直接删除
	printf("目标目录非空，是否删除该目录下所有文件？(y / n)\n");
	char select[2];
	std::cin.getline(select, 2);
	if (!strcmp(select, "y")) return true;
	return false;
}
void remove_file(int inode_num)
{
	//超级块修改
	super_block.sb_free_block_num += inode_table[inode_num].i_blocknum;
	super_block.sb_free_inode_num += 1;

	//位图修改
	for (int j = 0; j < inode_table[inode_num].i_blocknum; ++j)
		block_bitmap[inode_table[inode_num].i_first_block + j] = not_used;
	inode_bitmap[inode_num] = not_used;
	//父目录修改
	errno_t err = fopen_s(&fp, disk_name, "rb+");
	if (err != 0)
	{
		printf("can't open disk\n");
		return;
	}
		//读取父目录
	directory dir;
	fseek(fp, inode_table[inode_table[inode_num].parent_inode_num].i_start_addr, SEEK_SET);
	fread(&dir, sizeof(directory), 1, fp);

	for (int i = 0; i < max_file_num; ++i) 
	{
		if (dir.File_entry[i].inode_num == inode_num) 
		{
			//printf("%s xiugai\n", dir.File_entry[i].file_name);
			dir.file_num -= 1;
			dir.File_entry[i].exist = false;
			dir.File_entry[i].inode_num = 99999999;
			strcpy_s(dir.File_entry[i].file_name, "");
			break;
		}
	}
	fseek(fp, inode_table[inode_table[inode_num].parent_inode_num].i_start_addr, SEEK_SET);
	fwrite(&dir, sizeof(directory), 1, fp);

	//删除对应文件
	fseek(fp, inode_table[inode_num].i_start_addr, SEEK_SET);
	fwrite("", sizeof(char), inode_table[inode_num].i_fsize, fp);
	//i结点初始化
	inode_table[inode_num].init();

	//修改后的写入
	fseek(fp, 0, SEEK_SET);
	fwrite(&super_block, sizeof(_super_block), 1, fp);
	fwrite(&inode_table, sizeof(i_node), total_inode_num, fp);
	fwrite(&block_bitmap, sizeof(bool), total_block_num, fp);
	fwrite(&inode_bitmap, sizeof(bool), total_inode_num, fp);
	fclose(fp);
}
void remove_dir(int inode_num)
{
	directory dir = read_dir(inode_num);
	
	//清除目录下的文件和目录
	for (int i = 0; i < max_file_num; ++i)
	{
		if (dir.File_entry[i].exist)
		{
			if (inode_table[dir.File_entry[i].inode_num].i_ftype == dir_file)//删除目录
			{
				remove_dir(dir.File_entry[i].inode_num);
			}
			else//删除文件
			{
				remove_file(dir.File_entry[i].inode_num);
			}
		}
	}
	remove_file(inode_num);
}
bool try_create_file(char* file_name)
{
	directory dir = read_dir(sys_val.current_inode);

	for (int i = 0; i < max_file_num; ++i)
	{
		if (dir.File_entry[i].exist && inode_table[dir.File_entry[i].inode_num].i_ftype == normal_file && !strcmp(dir.File_entry[i].file_name, file_name))
		{
			const char* appd = "(new)";
			strcat_s(file_name, sizeof(file_name), appd);
			printf("已有同名文件，是否以%s为名字创建？(y / n)\n", file_name);
			char select[2];
			std::cin.getline(select, 2);
			if (!strcmp(select, "y")) {
				create_file(file_name);
				return true;
			}
			else
			{
				printf("取消文件创建\n");
				return false;
			}
		}
	}
	create_file(file_name);
	return true;
}
void create_file(char* file_name)
{
	directory dir = read_dir(sys_val.current_inode);

	for (int i = 0; i < max_file_num; ++i)
	{
		if (!dir.File_entry[i].exist)
		{
			//分配i结点和块,初始只给1个块
			int inode_num = select_inode(), first_block_pos = select_first_block(init_file_block_num);
			//修改位图、超级块
			super_block.sb_free_block_num -= init_file_block_num;
			super_block.sb_free_inode_num -= 1;
			inode_bitmap[inode_num] = used;
			block_bitmap[first_block_pos] = used;
			//修改i结点表
			strcpy_s(inode_table[inode_num].i_filename, file_name);
			//printf("%s", inode_table[inode_num].i_filename);
			inode_table[inode_num].i_uid = sys_val.current_user.u_id;
			inode_table[inode_num].i_ftype = normal_file;
			inode_table[inode_num].parent_inode_num = dir.inode_num;
			inode_table[inode_num].i_blocknum = init_file_block_num;		//初始文件默认只占一个块，连续存储
			inode_table[inode_num].i_fsize = strlen(file_name) + 1;
			inode_table[inode_num].i_first_block = first_block_pos;
			inode_table[inode_num].i_start_addr = super_block.sb_first_data_addr + first_block_pos * block_size;
			//修改当前目录
			dir.File_entry[i].exist = true;
			dir.File_entry[i].inode_num = inode_num;
			dir.file_num += 1;
			strcpy_s(dir.File_entry[i].file_name, file_name);
			//写回磁盘
			errno_t err = fopen_s(&fp, disk_name, "rb+");
			if (err != 0) { printf("error when write back\n"); return; }
			fseek(fp, 0, SEEK_SET);
			fwrite(&super_block, sizeof(_super_block), 1, fp);
			fwrite(&inode_table, sizeof(i_node), total_inode_num, fp);
			fwrite(&block_bitmap, sizeof(bool), total_block_num, fp);
			fwrite(&inode_bitmap, sizeof(bool), total_inode_num, fp);
			//写回当前目录
			fseek(fp, inode_table[sys_val.current_inode].i_start_addr, SEEK_SET);
			fwrite(&dir, sizeof(directory), 1, fp);

			fseek(fp, inode_table[inode_num].i_start_addr, SEEK_SET);
			fwrite(file_name, sizeof(char), inode_table[inode_num].i_blocknum * block_size, fp);
			fclose(fp);
			return;
		}
	}
	printf("该目录已满，无法创建\n");
}
char* read_file(char* file_name)
{
	directory dir = read_dir(sys_val.current_inode);

	for (int i = 0; i < max_file_num; ++i)
	{
		if (dir.File_entry[i].exist && inode_table[dir.File_entry[i].inode_num].i_ftype == normal_file && !strcmp(dir.File_entry[i].file_name, file_name))
		{
			int block_num = inode_table[dir.File_entry[i].inode_num].i_blocknum;
			char* content = new char[block_size * block_num];

			errno_t err = fopen_s(&fp, disk_name, "rb");
			//if (err != 0) { printf("open disk fail\n"); return; }
			fseek(fp, inode_table[dir.File_entry[i].inode_num].i_start_addr, SEEK_SET);
			fread(content, sizeof(char), inode_table[dir.File_entry[i].inode_num].i_fsize, fp);
			fclose(fp);

			//printf("%d\n", strlen(content));
			//*(content + inode_table[dir.File_entry[i].inode_num].i_fsize) = '\0';
			//std::cout << content << 1 << endl;
			return content;
		}
	}
	//printf("no such file named %s\n", file_name);
	char res[10];
	strcpy_s(res, "");
	return res;
}
void reallocate(int inode_num, int num_blk)
{
	//原内容
	char* content = new char[inode_table[inode_num].i_blocknum * block_size];
	errno_t err = fopen_s(&fp, disk_name, "rb+");
	fseek(fp, inode_table[inode_num].i_start_addr, SEEK_SET);
	fread(content, sizeof(char), inode_table[inode_num].i_fsize, fp);
	fseek(fp, inode_table[inode_num].i_start_addr, SEEK_SET);
	fwrite("", sizeof(char), inode_table[inode_num].i_fsize, fp);
	//fclose(fp);

	super_block.sb_free_block_num += inode_table[inode_num].i_blocknum;
	for(int i = 0; i < inode_table[inode_num].i_blocknum; ++i)
		block_bitmap[inode_table[inode_num].i_first_block + i] = not_used;
	inode_table[inode_num].i_first_block = select_first_block(num_blk);
	inode_table[inode_num].i_blocknum = num_blk;
	inode_table[inode_num].i_is_changed = true;
	inode_table[inode_num].i_start_addr = inode_table[inode_num].i_first_block * block_size + super_block.sb_first_data_addr;
	for (int i = 0; i < inode_table[inode_num].i_blocknum; ++i)
		block_bitmap[inode_table[inode_num].i_first_block + i] = used;
	super_block.sb_free_block_num -= inode_table[inode_num].i_blocknum;

	fseek(fp, inode_table[inode_num].i_start_addr, SEEK_SET);
	fwrite(content, sizeof(char), inode_table[inode_num].i_fsize, fp);
	fclose(fp);
	write_back();
}
void write_back()
{
	errno_t err = fopen_s(&fp, disk_name, "rb+");
	if (err != 0) { printf("error when write back\n"); return; }
	fseek(fp, 0, SEEK_SET);
	fwrite(&super_block, sizeof(_super_block), 1, fp);
	fwrite(&inode_table, sizeof(i_node), total_inode_num, fp);
	fwrite(&block_bitmap, sizeof(bool), total_block_num, fp);
	fwrite(&inode_bitmap, sizeof(bool), total_inode_num, fp);
	fclose(fp);
}
char* get_src_file(char* src)
{
	char file_name[max_file_name_length];
	char src_path[max_path_length];
	//路径分隔
	int pos;
	for (pos = strlen(src) - 1; pos >= 0; --pos)
	{
		if (src[pos] == '/')
		{
			break;
		}
	}
	if (pos < 0)//无'/'
	{
		strcpy_s(file_name, src);
	}
	else {
		strncpy_s(src_path, src, pos);
		strncpy_s(file_name, src + pos + 1, strlen(src) - pos - 1);
		cd(src_path);
	}
	//printf("%s", file_name);

	return read_file(file_name);
}
void do_copy(char* file_name, char* content)
{
	directory dir = read_dir(sys_val.current_inode);
	int size = strlen(content) + 1, num_blk = size / block_size + 1;
	//printf("%d\n", size);


	for (int i = 0; i < max_file_num; ++i)
	{
		if(dir.File_entry[i].exist && !strcmp(file_name, dir.File_entry[i].file_name) && inode_table[dir.File_entry[i].inode_num].i_ftype == normal_file)
		{
			if (num_blk > inode_table[dir.File_entry[i].inode_num].i_blocknum)
			{
				reallocate(dir.File_entry[i].inode_num, num_blk);
			}
			errno_t err = fopen_s(&fp, disk_name, "rb+");
			fseek(fp, inode_table[dir.File_entry[i].inode_num].i_start_addr, SEEK_SET);
			fwrite(content, sizeof(char), size + 1, fp);
			fclose(fp);
			inode_table[dir.File_entry[i].inode_num].i_fsize = strlen(content) + 1;
			write_back();
		}
	}
}
char* src_from_host(char* source)
{
	char src[max_path_length];
	strncpy_s(src, source + 6, strlen(source) - 6);
	errno_t err = fopen_s(&fp, src, "rb");
	if (err != 0) { printf("cannot open %s\n", src); }
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp);
	char* content = new char[fsize];
	fseek(fp, 0, SEEK_SET);
	fread(content, sizeof(char), fsize + 1, fp);
	fclose(fp);
	return content;
}
//void inode_compaction()
//{
//	int start_pos = 0;
//	for (int i = 0; i < total_inode_num; ++i)
//	{
//		if (inode_bitmap[i] == used)
//		{
//			for (int j = start_pos + 1; j < i; ++j)
//			{
//				if (inode_bitmap[j] == not_used)
//				{
//					inode_table[j] = inode_table[i];
//					inode_table[i].init();
//					inode_bitmap[j] = used;
//					inode_bitmap[i] = not_used;
//					start_pos = j;
//					break;
//				}
//			}
//		}
//	}
//}
