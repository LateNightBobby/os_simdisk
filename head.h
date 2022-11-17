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

#define SHARE_SIZE 4096	//�����ڴ��С
#define PROC_NAME	"simdisk"	//������

#define block_size 1024		//���С
#define block_per_group 1024//���С(ÿ��1024����)
#define inode_per_group 1024
#define group_num 100		//������
#define total_block_num block_per_group * group_num		//��������
#define total_inode_num inode_per_group * group_num		//i���������
#define max_file_name_length 128		//�ļ�����󳤶�
#define max_path_length 1024			//·����󳤶�
#define max_file_num	10		//һ��Ŀ¼������ļ�����
#define init_file_block_num 1	//�ļ���ʼ��ʱռ�ÿ���
#define first_data_addr sizeof(_super_block) + sizeof(bool) * (total_block_num + total_inode_num) + sizeof(i_node) * total_inode_num + sizeof(system_val)		//��һ�����ݿ�λ��
#define dir_size 1 + sizeof(directory) / block_size		//ÿ��Ŀ¼�ļ�ռ�Ŀ���
#define disk_name "virtual_disk.bin"		//������
#define max_CMD_length 8	//���ָ���
#define num_CMD 12			//ָ����
#define max_user_num 10		//����û���

//�ļ����ͣ�Ŀ¼����ͨ�ļ�
enum file_type {
	dir_file, normal_file, no_file
};
//λͼ��״̬
enum status_in_bitmap {
	not_used, used
};

struct user		//�û�
{
	char u_name[max_file_name_length];
	unsigned int u_id;
	char u_pwd[max_file_name_length];
};

struct _super_block			//������
{
	unsigned int sb_block_size;	//ÿ�����С
	unsigned int sb_block_num;	//���������
	unsigned int sb_inode_num;	//i�������
	unsigned int sb_free_block_num;	//���еĿ�����
	unsigned int sb_free_inode_num;	//����i�������
	long sb_first_data_addr;		//��һ�����ݿ��ַ
	short user_num;					//�û���
	user user[max_user_num];		//�û�����
};
//��дȨ��
struct Permission {
	bool RW[2];
	void init() {
		for (int i = 0; i < 2; ++i)
			RW[i] = 1;
	}
};
//ϵͳ����
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
struct i_node				//i���
{		//ÿ��i����Ӧһ���ļ�������ռ�ö���飬�����洢
	unsigned short i_ftype;		//��Ӧ�ļ�����
	int i_first_block;	//ʹ�õĵ�һ������
	unsigned int i_blocknum;	//�ļ�ռ�ÿ���
	long i_start_addr;			//�ļ���ʼ��ַ
	unsigned int i_fsize;		//�ļ���С
	unsigned int i_uid;			//�������û�id
	struct Permission rw;		//�������ߵĶ�дȨ��
	bool i_is_changed;			//�Ƿ��޸�
	char i_filename[max_file_name_length];//�ļ���
	int parent_inode_num;		//��һ��Ŀ¼i����
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

struct file_entry			//Ŀ¼���ļ����
{
	char file_name[max_file_name_length];	//�ļ���
	unsigned int inode_num;					//����i�����
	bool exist;								//�ļ��Ƿ����
	file_entry() {
		exist = false;
		inode_num = 99999999;
	}
};

struct directory			//Ŀ¼�ļ�
{
	char dir_name[max_file_name_length];		//Ŀ¼��
	unsigned int inode_num;						//i�����
	unsigned int file_num;						//Ŀ¼���ļ�����
	file_entry File_entry[max_file_num];		//�ļ����
};

extern HANDLE handle;		//
extern LPVOID share_ptr;	//

extern _super_block super_block;				//������
extern i_node inode_table[total_inode_num];		//i���
extern bool block_bitmap[total_block_num];		//���λͼ
extern bool inode_bitmap[total_inode_num];		//i���λͼ
extern FILE* fp;								//ָ������ļ���ָ��
extern system_val sys_val;						//ϵͳ����
extern directory rt_dir;						//��Ŀ¼


//����ָ��
static const char* CMD[] = 
{ "info", "cd", "dir", 
"md", "rd", "newfile", 
"cat", "copy", "del", 
"check", "help", "exit" };


bool login_success();//��¼
void loadDisk();			//������������ļ�
void initFile();			//��ʼ������	

//ָ���
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
directory read_dir(int inode_num);//��ȡĿ¼
void write_back();//д�ش���
int find_cmd(char* command);//���Ҷ�Ӧָ��
void create_dir(char* dir_name);//����Ŀ¼
bool find_path(char* path, system_val* temp);//·���Ƿ����
void show_dir(int inode_num, bool sub, int level);//��ʾָ��Ŀ¼
bool try_remove(int inode_num);//����Ƿ�ǿգ�ѯ��ɾ��
void remove_dir(int inode_num);//����Ŀ¼ɾ��
void remove_file(int inode_num);//��ײ��ļ�ɾ��
bool try_create_file(char* file_name);//���Դ����ļ�
void create_file(char* file_name);//�½��ļ�
void reallocate(int inode_num, int num_blk);//
int select_first_block(int block_num);//ѡ���һ����
int select_inode();//ѡ��i���
char* read_file(char* file_name);//��ȡ�ļ�����
char* get_src_file(char* src);//��ȡԴ�ļ�
char* src_from_host(char* source);//
void do_copy(char* src, char* dst_dir);//copyճ��
//void inode_compaction();//������i��㾡���ŵ�һ��

