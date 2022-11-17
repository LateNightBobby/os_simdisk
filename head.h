#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<Windows.h>

#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
using namespace std;

#define SHARE_SIZE 4096	//共享内存大小
#define PROC_NAME	"simdisk"	//进程名

#define block_size 1024		//块大小
#define block_per_group 1024//组大小(每组1024个块)
#define inode_per_group 1024
#define group_num 100		//组数量
#define total_block_num block_per_group * group_num		//块总数量
#define total_inode_num inode_per_group * group_num		//i结点总数量
#define max_file_name_length 128		//文件名最大长度
#define max_path_length 1024			//路径最大长度
#define max_file_num	10		//一个目录下最大文件数量
#define init_file_block_num 1	//文件初始化时占用块数
#define first_data_addr sizeof(_super_block) + sizeof(bool) * (total_block_num + total_inode_num) + sizeof(i_node) * total_inode_num + sizeof(system_val)		//第一个数据块位置
#define dir_size 1 + sizeof(directory) / block_size		//每个目录文件占的块数
#define disk_name "virtual_disk.bin"		//磁盘名
#define max_CMD_length 8	//最大指令长度
#define num_CMD 12			//指令数
#define max_user_num 10		//最大用户数

//文件类型：目录和普通文件
enum file_type {
	dir_file, normal_file, no_file
};
//位图中状态
enum status_in_bitmap {
	not_used, used
};

struct user		//用户
{
	char u_name[max_file_name_length];
	unsigned int u_id;
	char u_pwd[max_file_name_length];
};

struct _super_block			//超级块
{
	unsigned int sb_block_size;	//每个块大小
	unsigned int sb_block_num;	//块的总数量
	unsigned int sb_inode_num;	//i结点数量
	unsigned int sb_free_block_num;	//空闲的块数量
	unsigned int sb_free_inode_num;	//空闲i结点数量
	long sb_first_data_addr;		//第一个数据块地址
	short user_num;					//用户数
	user user[max_user_num];		//用户数据
};
//读写权限
struct Permission {
	bool RW[2];
	void init() {
		for (int i = 0; i < 2; ++i)
			RW[i] = 1;
	}
};
//系统变量
struct system_val {
	char current_path[500];
	user current_user;
	int current_inode;
	void init(system_val sv)
	{
		strcpy_s(current_path, sv.current_path);
		current_inode = sv.current_inode;
		current_user = sv.current_user;
	}
};
struct i_node				//i结点
{		//每个i结点对应一个文件，可以占用多个块，连续存储
	unsigned short i_ftype;		//对应文件类型
	int i_first_block;	//使用的第一个块块号
	unsigned int i_blocknum;	//文件占用块数
	long i_start_addr;			//文件起始地址
	unsigned int i_fsize;		//文件大小
	unsigned int i_uid;			//所有者用户id
	struct Permission rw;		//非所有者的读写权限
	bool i_is_changed;			//是否被修改
	char i_filename[max_file_name_length];//文件名
	int parent_inode_num;		//上一级目录i结点号
	void init()
	{
		i_ftype = no_file;
		i_first_block = -1;
		i_blocknum = 0;
		i_start_addr = first_data_addr;
		i_fsize = 0;
		i_uid = 99999999;
		i_is_changed = 1;
		strcpy_s(i_filename, "");
		parent_inode_num = 0;
	}
};

struct file_entry			//目录中文件入口
{
	char file_name[max_file_name_length];	//文件名
	unsigned int inode_num;					//所在i结点编号
	bool exist;								//文件是否存在
	file_entry() {
		exist = false;
		inode_num = 99999999;
	}
};

struct directory			//目录文件
{
	char dir_name[max_file_name_length];		//目录名
	unsigned int inode_num;						//i结点编号
	unsigned int file_num;						//目录下文件数量
	file_entry File_entry[max_file_num];		//文件入口
};

extern HANDLE handle;		//
extern LPVOID share_ptr;	//

extern _super_block super_block;				//超级块
extern i_node inode_table[total_inode_num];		//i结点
extern bool block_bitmap[total_block_num];		//块的位图
extern bool inode_bitmap[total_inode_num];		//i结点位图
extern FILE* fp;								//指向磁盘文件的指针
extern system_val sys_val;						//系统变量
extern directory rt_dir;						//根目录


//所有指令
static const char* CMD[] = 
{ "info", "cd", "dir", 
"md", "rd", "newfile", 
"cat", "copy", "del", 
"check", "help", "exit" };


bool login_success();//登录
void loadDisk();			//加载虚拟磁盘文件
void initFile();			//初始化磁盘	

//指令函数
void info();
void cd(char* path_name);
void dir(char* path, char* opt);
void md(char* path, char* dir_name);
void rd(char* dir_name);
void newfile(char* path , char* file_name);
void cat(char* file_name);
void copy(char* source, char* target);
void del(char* file_name);
void check();
void help(char* cmd_name);

void reload();//
directory read_dir(int inode_num);//读取目录
void write_back();//写回磁盘
int find_cmd(char* command);//查找对应指令
void create_dir(char* dir_name);//创建目录
bool find_path(char* path, system_val* temp);//路径是否存在
void show_dir(int inode_num, bool sub, int level);//显示指定目录
bool try_remove(int inode_num);//检查是否非空，询问删除
void remove_dir(int inode_num);//进行目录删除
void remove_file(int inode_num);//最底层文件删除
bool try_create_file(char* file_name);//尝试创建文件
void create_file(char* file_name);//新建文件
void reallocate(int inode_num, int num_blk);//
int select_first_block(int block_num);//选择第一个块
int select_inode();//选择i结点
char* read_file(char* file_name);//读取文件内容
char* get_src_file(char* src);//获取源文件
char* src_from_host(char* source);//
void do_copy(char* src, char* dst_dir);//copy粘贴
//void inode_compaction();//将空闲i结点尽量放到一起

