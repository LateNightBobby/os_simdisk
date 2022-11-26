#define _CRT_SECURE_NO_WARNINGS
#include"head.h"

void loadDisk()
{
	errno_t err = fopen_s(&fp, disk_name, "rb+");
	if (err != 0)	//�ļ�δ�򿪳ɹ�
	{
		printf("�������ļ�ϵͳ\t�Ƿ���г�ʼ��(y / n)\n");
		write_shm("�������ļ�ϵͳ\t�Ƿ���г�ʼ��(y / n)\n", true);
		char select[2];
		strcpy_s(select, read_shm().c_str());
		if (!strcmp(select, "y"))
		{
			initFile();
		}
		else {
			printf("exit\n");
			exit(0);
		}
		
	}
	printf("�Ѵ���\n");
	//write_shm("�Ѵ���\n", false);

	//�Ӵ��̶�ȡ���ݷ����ڴ�
	fseek(fp, 0, SEEK_SET);
	fread(&super_block, sizeof(_super_block), 1, fp);
	fread(&inode_table, sizeof(i_node), total_inode_num, fp);
	fread(&block_bitmap, sizeof(bool), total_block_num, fp);
	fread(&inode_bitmap, sizeof(bool), total_inode_num, fp);
	fread(&sys_val, sizeof(sys_val), 1, fp);
	fread(&rt_dir, sizeof(directory), 1, fp);
	fclose(fp);
	//printf(dir.dir_name);
}

void initFile()
{
	printf("��ʼ��simdisk\n");
	write_shm("��ʼ��simdisk\n", false);
	//f.open(disk_name, ios::out | ios::binary);

	errno_t err = fopen_s(&fp, disk_name, "wb+");
	if (err != 0)
	{
		printf("����simdisk�ļ�\n");
		write_shm("����simdisk�ļ�\n", false);
		//exit(0);
	}
	if (fp == NULL) {
		printf("�޷����ļ�\n");
		write_shm("�޷����ļ�\n", false);
		write_shm("$wrong exit$", false);
		exit(0);
	}
	long total_size = first_data_addr + block_size * total_block_num;		//���������顢i����λͼ�����ϵͳ�ܴ�С
	char* buffer = new char[total_size];									//��СΪҪ��Ļ�����

	//����������������ļ��У�����������̣��Զ�����д��
	fseek(fp, 0, SEEK_SET);
	fwrite(buffer, total_size, 1, fp);
	fclose(fp);
	//f.seekp(SEEK_SET);
	//f.write(buffer, total_size);
	//f.close();
	delete[] buffer;

	//�������ʼ��
	super_block.sb_block_num = total_block_num;
	super_block.sb_block_size = block_size;
	super_block.sb_first_data_addr = first_data_addr;
	super_block.sb_free_block_num = total_block_num;
	super_block.sb_free_inode_num = total_inode_num;
	super_block.sb_inode_num = total_inode_num;
	super_block.user_num = 3;
	//�û����ݳ�ʼ��
	super_block.user[0].u_id = 111;
	strcpy_s(super_block.user[0].u_name, "admin");
	strcpy_s(super_block.user[0].u_pwd, "123456");

	super_block.user[1].u_id = 222;
	strcpy_s(super_block.user[1].u_name, "user1");
	strcpy_s(super_block.user[1].u_pwd, "123456");

	super_block.user[2].u_id = 333;
	strcpy_s(super_block.user[2].u_name, "user2");
	strcpy_s(super_block.user[2].u_pwd, "123456");

	//i�����ʼ��
	for (int i = 0; i < total_inode_num; ++i)
	{
		inode_table[i].i_blocknum = 0;
		inode_table[i].i_first_block = -1;
		inode_table[i].i_fsize = 0;
		inode_table[i].i_ftype = no_file;
		inode_table[i].i_start_addr = 0;
		inode_table[i].rw.init();
		inode_table[i].i_is_changed = 0;
		inode_table[i].parent_inode_num = -1;
	}

	//��λͼ��ʼ��
	for (int i = 0; i < total_block_num; ++i)
		block_bitmap[i] = not_used;

	//i���λͼ��ʼ��
	for (int i = 0; i < total_inode_num; ++i)
		inode_bitmap[i] = not_used;

	//�޸�ϵͳ����
	system_val sys_val;
	strcpy_s(sys_val.current_path, "root/");
	sys_val.current_inode = 0;

	//��Ŀ¼��ʼ��
	char dir_name[max_file_name_length];
	strcpy_s(dir_name, "root");
	create_dir(dir_name);

	//д�����
	err = fopen_s(&fp, disk_name, "rb+");
	if (err != 0) {
		printf("cannot open\n");
		exit(0);
	}
	fseek(fp, 0, SEEK_SET);
	fwrite(&super_block, sizeof(_super_block), 1, fp);
	fwrite(&inode_table, sizeof(i_node), total_inode_num, fp);
	fwrite(&block_bitmap, sizeof(bool), total_block_num, fp);
	fwrite(&inode_bitmap, sizeof(bool), total_inode_num, fp);
	fwrite(&sys_val, sizeof(sys_val), 1, fp);
	fclose(fp);

}