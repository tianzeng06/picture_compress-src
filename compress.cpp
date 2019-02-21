#include "compress.h"

//1.堆排序部分
//heap_min_adjust,huffman_node_array是一位数组，里面存放的是指针变量
int heap_min_adjust(HuffmanNode **huffman_node_array,long data_start,long data_end)
{
	if(huffman_node_array==NULL||data_start<0||data_end<data_start)
	{
		fprintf(stderr,"heap_min_adjust: argument error\n");
		exit(1);
	}

	if(data_end==data_start)
		return 1;
	
	HuffmanNode *current_data_tobe_adjust=huffman_node_array[data_start];
	long current_indexof_data=data_start;
	
	long cur;
	for(cur=2*data_start;cur<=data_end;cur=2*cur)
	{
		if(cur<data_end)//找最小权的结点,左孩子或右孩子
			if((huffman_node_array[cur]->data_8bit_count)>(huffman_node_array[cur+1]->data_8bit_count))
				cur+=1;

		//如果当前结点的权值比上一步找到的权值最小的权值小，则跳出
		if((current_data_tobe_adjust->data_8bit_count)<=(huffman_node_array[cur]->data_8bit_count))
			break;
		
		//设置当前结点为权值最小的结点
		huffman_node_array[current_indexof_data]=huffman_node_array[cur];
		current_indexof_data=cur;
	}
	huffman_node_array[current_indexof_data]=current_data_tobe_adjust;

	return 1;   //return 1 means everything is ok
}

//heap_min_counstruct
int heap_min_construct(HuffmanNode **huffman_node_array,long array_size)
{
	//valid data start from index 1 not 0,check error for argument
	if(huffman_node_array==NULL||array_size<=0||array_size>256)
	{
		fprintf(stderr,"heap_min_construct:argument error\n");
		exit(1);
	}

	//完全二叉树的最后一个非叶子节点为/2
	for(long heap_root=(long)floor(array_size/2);heap_root>=1;--heap_root)
		heap_min_adjust(huffman_node_array,heap_root,array_size);

	return 1;
}

//heap_min_get2min
int heap_min_get2min(HuffmanNode **huffman_node_array,HuffmanNode **min_first,
						HuffmanNode **min_second,long *heap_size)
{
	if(huffman_node_array==NULL)
	{
		fprintf(stderr,"heap_min_get2min: argument error\n");
		exit(1);
	}

	//堆排序过程，根结点与最后一个结点交换
	*min_first=huffman_node_array[1];
	huffman_node_array[1]=huffman_node_array[*heap_size];
	(*heap_size)-=1;
	//after we get the min data,we shuold again adjust the heap to make a min-heap
	heap_min_adjust(huffman_node_array,1,*heap_size);

	*min_second=huffman_node_array[1];
	huffman_node_array[1]=huffman_node_array[*heap_size];
	(*heap_size)-=1;

	if(*heap_size>0)
		heap_min_adjust(huffman_node_array,1,*heap_size);

	return 1;
}

//2.huffman编码实现部分
//构造huffman树，huffman_tree
int huffman_tree(HuffmanNode **huffman_tree_root,HuffmanNode **huffman_node_array,long array_size)
{
	if(huffman_node_array==NULL||array_size<1||array_size>256)
	{
		fprintf(stderr,"huffman_tree:argument error\n");
		exit(1);
	}

	if(array_size==1)
	{
		printf("huffman_tree:\nnode size: %ld too small to construct huffman tree\ntry a big file\n",array_size);
		exit(1);
	}

	HuffmanNode *huffman_parent_node=(HuffmanNode *)malloc(sizeof(HuffmanNode)*(array_size-1+1));

	long i;
	for(i=1;i<array_size;++i)
	{
		(huffman_parent_node+i)->data_8bit=0x00;
		(huffman_parent_node+i)->data_8bit_count=0;
		(huffman_parent_node+i)->left_child=NULL;
		(huffman_parent_node+i)->right_child=NULL;
		(huffman_parent_node+i)->parent=NULL;
	}

	//先构造一颗huffman树
	heap_min_construct(huffman_node_array,array_size);

	HuffmanNode *min1,*min2;

	long heap_size=array_size;
	for(i=1;i<array_size;++i)
	{
		heap_min_get2min(huffman_node_array,&min1,&min2,&heap_size);

		huffman_parent_node[i].data_8bit=0x00;
		huffman_parent_node[i].data_8bit_count=min1->data_8bit_count+min2->data_8bit_count;

		huffman_parent_node[i].left_child=min1;
		huffman_parent_node[i].right_child=min2;
		huffman_parent_node[i].parent=NULL;

		min1->parent=&(huffman_parent_node[i]);
		min2->parent=&(huffman_parent_node[i]);
		
		//insert parent node and again make min heap
		heap_size+=1;
		huffman_node_array[heap_size]=&(huffman_parent_node[i]);
		heap_min_construct(huffman_node_array,heap_size);
	}

	if(huffman_parent_node[array_size-1].parent==NULL)//
	{
		*huffman_tree_root=&huffman_parent_node[array_size-1];//构造完毕的huffman树根结点的指向
	}
	else
	{
		fprintf(stderr,"huffman tree construct failed\n");
		exit(1);
	}
	return 1;
}

//huffman_encode,根据huffman_tree()构造的树，对每个叶子结点到根结点的huffman逆序编码
int huffman_encode(HuffmanNode **huffman_node_array,long huffman_node_array_size,HuffmanEncode *huffman_encode_array)
{
	//check error for argumet
	if(huffman_node_array==NULL||huffman_encode_array==NULL||
		huffman_node_array_size<1||huffman_node_array_size>256)
	{
		fprintf(stderr,"huffman_encode: argument error \n");
		exit(1);
	}

	if(huffman_node_array_size==1)
	{
		fprintf(stderr,"huffman_encode: node array size too small to construct huffman encode\n");
		exit(1);
	}
	
	HuffmanNode **huffman_node_array_backup=(HuffmanNode **)malloc((huffman_node_array_size+1)*sizeof(HuffmanNode *));

	if(huffman_node_array_backup==NULL)
	{
		fprintf(stderr,"malloc failed\n");
		exit(1);
	}

	/*
	 * leater we will use this backup to make huffman encode
	 * note:backup should before invoking huffman_tree(),because this 
	 *		function will change array huffman_node_array
	 */
	
	long i;
	for(i=1;i<=huffman_node_array_size;++i)
		huffman_node_array_backup[i]=huffman_node_array[i];

	HuffmanNode *huffman_tree_root=NULL;
	huffman_tree(&huffman_tree_root,huffman_node_array,huffman_node_array_size);

	//construct huffmanencode,traverse huffman tree,left encode 0,right 1

	char encode_temp[260];
	long start_temp=259;

	for(i=1;i<=huffman_node_array_size;++i)
	{
		start_temp=259;
		HuffmanNode *leaf=huffman_node_array_backup[i];

		while(leaf->parent!=NULL)
		{
			HuffmanNode *parent=leaf->parent;
			if(parent->left_child==leaf)
				encode_temp[start_temp--]='0';
			else if(parent->right_child==leaf)
				encode_temp[start_temp--]='1';
			else
			{
				fprintf(stderr,"the huffman tree reference seems not correct\n");
				exit(1);
			}
			leaf=leaf->parent;
		}
		
		//以每个字符‘01’串的索引
		long huffman_encode_array_index=huffman_node_array_backup[i]->data_8bit;
		//start_temp从259开始减，所以字符串的大小位259-start_temp
		huffman_encode_array[huffman_encode_array_index].encode=(char*)malloc((259-start_temp)*sizeof(char));
		if(huffman_encode_array[huffman_encode_array_index].encode==NULL)
		{
			fprintf(stderr,"huffman_encode: malloc failed\n");
			exit(1);
		}

		huffman_encode_array[huffman_encode_array_index].length_of_encode=259-start_temp;
		//跳出循环时，start_temp又一次减一，所以字符串的起始地址+1
		memcpy(huffman_encode_array[huffman_encode_array_index].encode,encode_temp+start_temp+1,259-start_temp);//最终字符串由encode_temp拷贝到huffman_encode_array的encode记录中
	}

	if(huffman_node_array_backup!=NULL)
	{
		free(huffman_node_array_backup);
		huffman_node_array_backup=NULL;
	}
	return 1;
}

//3.huffman解码部分
//huffman_decode_internal,把磁盘文件读入内存，根据内存bit值从huffman根结点往下走
int huffman_decode_internal(HuffmanNode **huffman_tree_current_root,long direction)
{
	//check error for argument
	//当前某个特定的结点
	if(huffman_tree_current_root==NULL||*huffman_tree_current_root==NULL||!(direction==0||direction==1))
	{
		fprintf(stderr,"huffman_decode_internal: argument error\n");
		exit(1);
	}

	if(direction==0)
	{
		(*huffman_tree_current_root)=(*huffman_tree_current_root)->left_child;
		if((*huffman_tree_current_root)->left_child==NULL&&(*huffman_tree_current_root)->right_child==NULL)//走到叶子结点，返回到调用函数中
			return 1;//返回1，判断当前已经解析出一个unsigend char
		else
			return 0;
	}
	else if(direction==1)
	{
		(*huffman_tree_current_root)=(*huffman_tree_current_root)->right_child;

		if((*huffman_tree_current_root)->left_child==NULL&&(*huffman_tree_current_root)->right_child==NULL)
			return 1;
		else
			return 0;
	}
	else
	{
		fprintf(stderr,"\ndirection error: %ld\n",direction);
		exit(1);
	}
}

//huffman_decode
/*huffman_tree_root重建出来的huffman树根结点指针，重建动作由huffman_decode()的调用者
 * 函数负责构造，encode_buffer是内存缓冲区，haffuman_decode()函数
 * 调用者会把压缩文件分块读入encode_buffer，解码结果放到decode_buffer代表的内存缓冲区
 */
int huffman_decode(HuffmanNode *huffman_tree_root,HuffmanNode **last_stop_at,EncodeBuffer *encode_buffer,DecodeBuffer *decode_buffer)
{
	//check error for argument
	if(last_stop_at==NULL||encode_buffer==NULL||decode_buffer==NULL)
	{
		fprintf(stderr,"huffman_decode: argument error\n");
		exit(1);
	}

	long not_full_byte=0;
	long bits_num_ofLastByte=0;
	
	if(encode_buffer->bits_num_lastbytes>0)
	{
		not_full_byte=1;
		bits_num_ofLastByte=encode_buffer->bits_num_lastbytes;
	}

	if(*last_stop_at==NULL)
		*last_stop_at=huffman_tree_root;

	if(decode_buffer->size!=0)
		decode_buffer->size=0;

	long buffer_size_real=encode_buffer->size;//该buffer的有效字节存储量

	unsigned char *ptr_current_buffer=encode_buffer->buffer;
	unsigned char *ptr_endOf_real_buffer=encode_buffer->buffer+buffer_size_real-1;
	long return_value=-1;

	for(;ptr_current_buffer<=ptr_endOf_real_buffer;++ptr_current_buffer)
	{
		unsigned char temp=*ptr_current_buffer;
		long i;
		for(i=0;i<8;++i)//一个字节8位
		{
			unsigned char bit_for_test=0x80;//10000000
			bit_for_test=bit_for_test>>i;//把unsigned char转换为2进制，即01串，从高位开始取
			long direction;

			if(bit_for_test&temp)//如果此位有数字，则规定向右走
				direction=1;
			else
				direction=0;

			//last_stop_at记录走到当前huffman树的哪一个结点
			return_value=huffman_decode_internal(last_stop_at,direction);
			if(return_value==1)//internal函数返回1，表示解码成功，取道unsigned char字符，存入decode_buffer
			{
				decode_buffer->buffer[decode_buffer->size]=(*last_stop_at)->data_8bit;
				(decode_buffer->size)+=1;
				*last_stop_at=huffman_tree_root;
			}
		}
	}

	//bits有剩余，因为在编码的时候bit不一定是8的倍数
	if(not_full_byte==1)
	{
		unsigned char temp=encode_buffer->buffer[encode_buffer->size];
		long i;
		for(i=0;i<bits_num_ofLastByte;++i)
		{
			unsigned char bit_for_test=0x80;
			bit_for_test=bit_for_test>>i;
			long direction;

			if(bit_for_test&temp)//按位取，如果此位是1，向右
				direction=1;
			else
				direction=0;

			//如果结果返回1，则调用函数中取到了unsigned char，存入结果缓冲区
			return_value=huffman_decode_internal(last_stop_at,direction);
			if(return_value==1)
			{
				decode_buffer->buffer[decode_buffer->size]=(*last_stop_at)->data_8bit;
				(decode_buffer->size)+=1;
				*last_stop_at=huffman_tree_root;
			}
		}
	}
	return 1;
}

//4.业务逻辑部分
//bit_set
int bit_set(unsigned char *ch,long index,long flag)
{
	if(flag==1)
		(*ch)|=(0x80>>index);
	else if(flag==0)
		(*ch)&=~(0x80>>index);
	else
	{
		fprintf(stderr,"bit_set: flag not correct\n");
		exit(1);
	}
	return 1;
}

//open_file
int open_file(char *filename,FILE **fp)
{
	if(filename==NULL||fp==NULL)
	{
		fprintf(stderr,"open_file: argument error\n");
		exit(1);
	}

	if(access(filename,0)!=0)//判断文件是否存在
	{
		fprintf(stderr,"open_file: file %s not exist\n",filename);
		exit(1);
	}

	*fp=fopen(filename,"rb");//以2进制读的方式打开
	if((*fp)==NULL)
	{
		fprintf(stderr,"open file %s error\n",filename);
		exit(1);
	}
	return 0;
}

//create_Uncompress_file
int create_Uncompress_file(char *filename,FILE **fp)
{
	if(filename==NULL||fp==NULL)
	{
		fprintf(stderr,"create_Uncompress_file: argument error\n");
		exit(1);
	}

	if(access(filename,0)==0)
	{
		long temp=strlen(filename);
		char *filename_new=(char*)malloc(sizeof(char)*(temp+2));
		memcpy(filename_new,filename,temp);
		long j=0,k=0,i;
		for(i=0;i<temp;++i)
		{
			if(filename[i]=='.'&&k==0)
			{
				++k;
				filename_new[j++]='1';
				filename_new[j++]='0';
			}
			else
				filename_new[j++]=filename[i];
		}
		filename_new[j]='\0';
		filename=filename_new;
	}

	if((*fp=fopen(filename,"wb"))==NULL)
	{
		fprintf(stderr,"create_Uncompress_file:create file failed!\n");
		exit(1);
	}
	return 1;
}

//create_file
int create_file(char *filename,FILE **fp)
{
	if(filename==NULL)
	{
		fprintf(stderr,"create_file: argument error\n");
		exit(1);
	}

	if(access(filename,0)==0)
		if(remove(filename)!=0)
		{
			fprintf(stderr,"error occur when delete file\n");
			exit(1);
		}
	
	if((*fp=fopen(filename,"wb"))==NULL)
	{
		fprintf(stderr,"can not open file!\n");
		exit(1);
	}
	return 1;
}

//make_uchar_weight,统计每个unsigned char的权值，然后调用
//huffman_encode得到每个unsigned char的哈夫曼编码
int make_uchar_weight(FILE *fp,HuffmanNode **huffman_node,long *size)
{
	if(fp==NULL||huffman_node==NULL||size==NULL)
	{
		fprintf(stderr,"make_uchar_weight: argument error\n");
		exit(1);
	}

	*size=0;
	const long buffer_size=40*1024*1024;
	unsigned char *buffer=(unsigned char *)malloc(buffer_size*sizeof(unsigned char));
	
	if(buffer==NULL)
	{
		fprintf(stderr,"make_uchar_weight: malloc falied\n");
		exit(1);
	}

	HuffmanNode *huffman_node_array0=(HuffmanNode *)malloc(257*sizeof(HuffmanNode));
	if(huffman_node_array0==NULL)
	{
		fprintf(stderr,"make_uchar_weight: malloc failed\n");
		exit(1);
	}

	long i;
	for(i=0;i<257;++i)
	{
		huffman_node_array0[i].data_8bit=0x00;
		huffman_node_array0[i].data_8bit_count=0;
		huffman_node_array0[i].parent=NULL;
		huffman_node_array0[i].left_child=NULL;
		huffman_node_array0[i].right_child=NULL;
	}

	long read_bytes=0;
	while((read_bytes=fread(buffer,sizeof(unsigned char),buffer_size,fp))!=0)
	{
		unsigned char *ptr=buffer;
		unsigned char *ptr_end=buffer+read_bytes;
		for(;ptr!=ptr_end;++ptr)
		{
			huffman_node_array0[(long)(*ptr)].data_8bit=*ptr;
			huffman_node_array0[(long)(*ptr)].data_8bit_count+=1;
		}
	}

	for(i=0;i<257;++i)
		if(huffman_node_array0[i].data_8bit_count>0)
		{
			(*size)++;
			huffman_node[*size]=&(huffman_node_array0[i]);
		}

	if(buffer!=NULL)
	{
		free(buffer);
		buffer=NULL;
	}
	return 1;
}

//4.业务逻辑实现
int compress(char *src_name)
{
	if(src_name==NULL)
	{
		fprintf(stderr,"compress: argument error\n");
		exit(1);
	}

	printf("---------------compress begin to execut---------------\n");
	printf("source file name: %s\n",src_name);
	char *dst_name=(char *)malloc(strlen(src_name)+6+1);
	memcpy(dst_name,src_name,strlen(src_name));
	char *cur0=dst_name+strlen(src_name);
	memcpy(cur0,".cprs",5);
	*(cur0+5)='\0';
	printf("compress file name: %s\n",dst_name);

	FILE *src_fp=NULL;
	FILE *dst_fp=NULL;
	open_file(src_name,&src_fp);
	create_file(dst_name,&dst_fp);

	HuffmanNode **huffman_node_array=(HuffmanNode **)malloc(257*sizeof(HuffmanNode *));//构造一颗huffman树
	if(huffman_node_array==NULL)
	{
		fprintf(stderr,"compress: malloc failed\n");
		exit(1);
	}

	//统计每个unsigned char 出现的个数
	long huffman_node_num=0;
	make_uchar_weight(src_fp,huffman_node_array,&huffman_node_num);
	
	HuffmanEncode *encode_array=(HuffmanEncode *)malloc(257*sizeof(HuffmanEncode));
	if(encode_array==NULL)
	{
		fprintf(stderr,"compress: malloc failed\n");
		exit(1);
	}

	long i;
	for(i=0;i<257;++i)
	{
		encode_array[i].encode=NULL;
		encode_array[i].length_of_encode=0;
	}

	//对每个unsigned char进行编码
	huffman_encode(huffman_node_array,huffman_node_num,encode_array);

	/*
	 * begin to deal with the header of the compress file,later the
	 * uncompress routine can read the header and restore the huffman then
	 * uncompress the file
	 */
	CompressFileHeader CFH;
	CFH.length_of_author_name=12;
	CFH.author_name="ChenTianZeng";
	CFH.routine_name_length=8;
	CFH.routine_name="compress";
	CFH.suffix_length=4;
	CFH.suffix="cprs";
	CFH.file_name_length=strlen(src_name);
	CFH.bits_of_lastByte=0x00;

	long length_of_header1=0;
	long length_of_header2=2;
	char *cur=src_name;

	while(cur[length_of_header1++]!='\0');

	length_of_header1+=2*sizeof(long)+12+10+6;
	length_of_header2=length_of_header1;

	for(i=0;i<256;++i)//length_of_encode==0此字符没有出现过
		if(encode_array[i].length_of_encode==0)
			length_of_header2+=1;
		else if(encode_array[i].length_of_encode>0)
			length_of_header2+=1+encode_array[i].length_of_encode;

	unsigned char *buffer_for_header=(unsigned char *)malloc(sizeof(unsigned char)*length_of_header2);
	if(buffer_for_header==NULL)
	{
		fprintf(stderr,"compress: malloc failed for buffer_for_header\n");
		exit(1);
	}
	//ptr先存放压缩头文件信息
	unsigned char *ptr=buffer_for_header;
	*((long *)ptr)=length_of_header1;
	*((long *)(ptr+sizeof(long)))=length_of_header2;
	ptr+=2*sizeof(long);
	*(ptr++)=(unsigned char)CFH.length_of_author_name;
	*(ptr++)=(unsigned char)CFH.routine_name_length;
	*(ptr++)=(unsigned char)CFH.suffix_length;
	*(ptr++)=(unsigned char)CFH.file_name_length;
	*(ptr++)=(unsigned char)CFH.bits_of_lastByte;

	char *ch="ChenTianZengCompresscprs";
	memcpy(ptr,ch,24);
	ptr+=24;
	memcpy(ptr,src_name,CFH.file_name_length);
	ptr+=CFH.file_name_length;

	//再存放每个unsigned char的编码
	for(i=0;i<256;++i)
		if(encode_array[i].length_of_encode==0)
		{
			*ptr=0x00;
			ptr+=1;
		}
		else
		{
			*ptr=encode_array[i].length_of_encode;
			ptr++;
			memcpy(ptr,encode_array[i].encode,encode_array[i].length_of_encode);
			ptr+=encode_array[i].length_of_encode;
		}

	//把编码头文件信息写入目标文件
	long temp=fwrite(buffer_for_header,length_of_header2,1,dst_fp);
	if(temp!=1)
	{
		fprintf(stderr,"compress:fwrite for compress file header error\n");
		exit(1);
	}

	//'内存',编码后的先存在'内存'中，然后在写入磁盘文件,相当于缓冲区
	EncodeBuffer *encode_buffer=(EncodeBuffer *)malloc(sizeof(EncodeBuffer));
	if(encode_buffer==NULL)
	{
		fprintf(stderr,"compress: malloc failed\n");
		exit(1);
	}
	
	encode_buffer->bits_num_lastbytes=0;
	encode_buffer->buffer=NULL;
	encode_buffer->size=0;
	const long fread_buffer_size=2*1024*1024;
	const long encode_buffer_size=32*fread_buffer_size;
	encode_buffer->buffer=(unsigned char *)malloc(encode_buffer_size*sizeof(unsigned char));
	if(encode_buffer->buffer==NULL)
	{
		fprintf(stderr,"compress:malloc failed\n");
		exit(1);
	}

	unsigned char *fread_buffer=(unsigned char *)malloc(fread_buffer_size*sizeof(unsigned char));
	if(fread_buffer==NULL)
	{
		fprintf(stderr,"compress: malloc failed\n");
		exit(1);
	}

	long read_bytes=0;
	fseek(src_fp,0L,0);
	long long num_ofbits_write=-1;
	while((read_bytes=fread(fread_buffer,sizeof(unsigned char),fread_buffer_size,src_fp))!=0)
	{
		unsigned char *ptr=fread_buffer;
		unsigned char *ptr_end=fread_buffer+read_bytes;
		long num_ofbits_write_per_while=-1;
		
		/*
		 * 为了不覆盖上一次不是8的倍数的后面的那几个bit，产生一个偏移量
		 * 在下一次写数据时在偏移里后写数据
		 */
		if(num_ofbits_write>=0)
			num_ofbits_write_per_while+=(num_ofbits_write+1)%8;//@
		
		for(;ptr!=ptr_end;++ptr)
		{
			long index=(long)(*ptr);
			if(encode_array[index].length_of_encode<=0)
			{
				fprintf(stderr,"compress: encode_array not correct\n");
				exit(1);
			}

			long i;
			for(i=0;i<encode_array[index].length_of_encode;++i)
			{
				++num_ofbits_write;//记录当前字符的当前bit
				++num_ofbits_write_per_while;//记录共写了多少字bit
				//如果写够了8个bit，从下一个字符开始
				long byte_index=num_ofbits_write_per_while/8;
				//写够了8个bit，从第一位开始
				long bit_index=num_ofbits_write%8;
				//判断当前bit是0还是1
				long flag=(long)(encode_array[index].encode[i]-'0');
				//把特定字节*ch第index个bit设为0或1
				bit_set((encode_buffer->buffer)+byte_index,bit_index,flag);
			}
		}

		encode_buffer->size=((num_ofbits_write_per_while+1)/8);//字节数
		//剩余bit数
		encode_buffer->bits_num_lastbytes=((num_ofbits_write+1)%8);
		//把‘内存’中的编码写入磁盘，即缓冲区
		long fwrite_size=encode_buffer->size;
		long real_write_num=fwrite(encode_buffer->buffer,sizeof(unsigned char),fwrite_size,dst_fp);
		if(real_write_num!=fwrite_size)
		{
			fprintf(stderr,"compress:fwrite error\n");
			exit(1);
		}

		/*
		 * 压缩缓冲区即‘内存’中的bit数量不一定是8的倍数，但写入磁盘的最小
		 * 单位是字节，解决办法：把8的倍数那一部分写入磁盘，不足8的倍数的
		 * bit从新移到缓冲区开头处，下一轮fread数据编码紧接着刚才那几个bit
		 * 后面存储，上文@处是避免下一次fread覆盖上一次fread产生的不能整除
		 * 8后面那几个bit
		 */
		if(encode_buffer->bits_num_lastbytes>0)
			*(encode_buffer->buffer)=*(encode_buffer->buffer+encode_buffer->size);

	}

	/*
	 * 判断当前处理bit总数，不是8的整倍数，则将缓冲区开头第一个字节写入磁盘
	 *否则不写入磁盘，并记录最后一个字节包含的有效的bit数，写入头文件对应
     *字段处
	 */
	if((num_ofbits_write+1)%8!=0)
	{
		long temp=fwrite(encode_buffer->buffer,sizeof(unsigned char),1,dst_fp);
		if(temp!=1)
		{
			fprintf(stderr,"compress:fwrite error\n");
			exit(1);
		}
		CFH.bits_of_lastByte=(num_ofbits_write+1)%8;
	}
		
	//set the correct bits_of_lastByte of the compress file header
	*(buffer_for_header+2*sizeof(long)+4)=CFH.bits_of_lastByte;
	fseek(dst_fp,0L,0);
	long temp0=fwrite(buffer_for_header,1,2*sizeof(long)+5,dst_fp);
	if(temp0!=2*sizeof(long)+5)
	{
		fprintf(stderr,"compress: fwrite error\n");
		exit(1);
	}

	//释放动态申请的内存，避免内存泄露
	if(dst_name!=NULL)
	{
		free(dst_name);
		dst_name=NULL;
	}
	if(huffman_node_array!=NULL)
	{
		free(huffman_node_array);
		huffman_node_array=NULL;
	}
	int k;
	for(k=0;k<257;++k)
	{
		if(encode_array[k].encode!=NULL)
		{
			free(encode_array[k].encode);
			encode_array[k].encode=NULL;
		}
	}
	if(encode_array!=NULL)
	{
		free(encode_array);
		encode_array=NULL;
	}
	if(buffer_for_header!=NULL)
	{
		free(buffer_for_header);
		buffer_for_header=NULL;
	}
	if(encode_buffer!=NULL)
	{
		free(encode_buffer);
		encode_buffer=NULL;
	}
	if(fread_buffer!=NULL)
	{
		free(fread_buffer);
		fread_buffer=NULL;
	}
	if(fclose(src_fp)!=0)
		fprintf(stderr,"fclose src_fp error\n");

	if(fclose(dst_fp)!=0)
		fprintf(stderr,"fclost dst_fp error\n");

	printf("compress:success\n");
	return 1;
}

//重建huffman树，根据参数huffman_encode_array存储的huffman编码表来重建
//restore_huffman_tree
int restore_huffman_tree(HuffmanNode **huffman_tree_root,HuffmanEncode *huffman_encode_array)
{
	if(huffman_tree_root==NULL||huffman_encode_array==NULL)
	{
		fprintf(stderr,"restore_huffman_tree: argument error\n");
		exit(1);
	}

	//统计huffman编码数组中有多少字符
	long valid_encode_num=0,i;
	for(i=0;i<256;++i)
		if(huffman_encode_array[i].length_of_encode>0)
			++valid_encode_num;
	
	HuffmanNode *huffman_nodes=(HuffmanNode *)malloc(sizeof(HuffmanNode)*(valid_encode_num*2-1));
	if(huffman_nodes==NULL)
	{
		fprintf(stderr,"\nrestore_huffman_tree: malloc failed\n");
		exit(1);
	}

	for(i=0;i<valid_encode_num*2-1;++i)
	{
		huffman_nodes[i].left_child=NULL;
		huffman_nodes[i].right_child=NULL;
		huffman_nodes[i].parent=NULL;
	}

	//重建huffman树的根结点的指针存在huffman_tree_root中
	long huffman_nodes_index=0;
	(*huffman_tree_root)=&(huffman_nodes[huffman_nodes_index++]);

	for(i=0;i<256;++i)
	{
		if(huffman_encode_array[i].length_of_encode>0)
		{
			HuffmanNode *current=(*huffman_tree_root);
			long j;
			for(j=0;j<huffman_encode_array[i].length_of_encode;++j)
			{
				if(huffman_encode_array[i].encode[j]=='0')
				{
					if(current->left_child!=NULL)
						current=current->left_child;
					else
					{
						current->left_child=&(huffman_nodes[huffman_nodes_index++]);
						current->left_child->parent=current;
						current=current->left_child;
					}
				}
				else if(huffman_encode_array[i].encode[j]=='1')
				{
					if(current->right_child!=NULL)
						current=current->right_child;
					else
					{
						current->right_child=&(huffman_nodes[huffman_nodes_index++]);
						current->right_child->parent=current;
						current=current->right_child;
					}
				}
				else
				{
					fprintf(stderr,"restore_huffman_tree:encode array error\n");
					exit(1);
				}
			}
			current->data_8bit=(unsigned char)i;
		}
	}
	return 1;
}
//解压缩过程中分析压缩文件的头部
//pares_head()
int parse_head(FILE **fp,HuffmanEncode *huffman_encode_array,char **file_name,long *last_bits)
{
	if(fp==NULL||*fp==NULL||huffman_encode_array==NULL||file_name==NULL||last_bits==NULL)
	{
		fprintf(stderr,"parse_head:argument error\n");
		exit(1);
	}

	long sizeof_header1=0;
	long sizeof_header2=0;

	unsigned char *buffer=(unsigned char *)malloc(sizeof(long)*2);
	long read_num=fread(buffer,sizeof(long),2,*fp);
	if(read_num!=2)
	{
		fprintf(stderr,"parse_head: read first two long of header failed\n");
		exit(1);
	}
	sizeof_header1=*((long *)buffer);
	sizeof_header2=*((long *)(buffer+sizeof(long)));

	if(buffer!=NULL)
	{
		free(buffer);
		buffer=NULL;
	}

	unsigned char *buffer_header=(unsigned char *)malloc(sizeof(unsigned char)*sizeof_header2);
	if(buffer_header==NULL)
	{
		fprintf(stderr,"parse_head malloc for buffer_header failed\n");
		exit(1);
	}

	fseek(*fp,0L,0);

	read_num=fread(buffer_header,sizeof(unsigned char),sizeof_header2,*fp);
	if(read_num!=sizeof_header2)
	{
		fprintf(stderr,"parse_header: fread faild\n");
		exit(1);
	}

	unsigned char *ptr_header1=buffer_header;
	unsigned char *ptr_header1_end=buffer_header+sizeof_header1;

	unsigned char *ptr_header2=buffer_header+sizeof_header1;
	unsigned char *ptr_header2_end=buffer_header+sizeof_header2;

	ptr_header1+=sizeof(long)*2;

	long length_of_author_name=*(ptr_header1);
	long length_of_routine_name=*(ptr_header1+1);
	long length_of_suffix=*(ptr_header1+2);
	long length_of_file_name=*(ptr_header1+3);
	long length_of_lastbits=*(ptr_header1+4);
	ptr_header1+=5;

	*last_bits=length_of_lastbits;

	printf("\ncompress author: ");
	unsigned char *ptr=NULL;
	long i=0;
	for(i=1,ptr=ptr_header1;i<=length_of_author_name;++ptr,++i)
		printf("%c",*ptr);

	printf("\ncompress suffix is: ");
	for(i=1;i<=length_of_suffix;++ptr,++i)
		printf("%c",*ptr);
	
	(*file_name)=(char *)malloc(sizeof(char)*(length_of_file_name+1));
	if(file_name==NULL)
	{
		fprintf(stderr,"parse_header: malloc failed\n");
		exit(1);
	}
	memcpy((*file_name),ptr,length_of_file_name);
	(*file_name)[length_of_file_name]='\0';

	printf("\noriginal file name is: ");
	for(i=1;i<=length_of_file_name;++ptr,++i)
		printf("%c",*ptr);
	printf("\n");

	long index=-1;
	unsigned char *cur=ptr_header2;
	while(cur<ptr_header2_end)
	{
		++index;
		if(*cur==0x00)
			++cur;
		else
		{
			huffman_encode_array[index].encode=(char *)malloc((*cur)*sizeof(char));
			if(huffman_encode_array[index].encode==NULL)
			{
				fprintf(stderr,"parse_header malloc failed\n");
				exit(1);
			}
			
			huffman_encode_array[index].length_of_encode=*cur;
			memcpy(huffman_encode_array[index].encode,cur+1,*cur);
			cur=cur+1+huffman_encode_array[index].length_of_encode;
		}
	}
	
	if(buffer_header!=NULL)
	{
		free(buffer_header);
		buffer_header=NULL;
	}
	return 1;
}

//Uncompress()
int Uncompress(char *src_name)
{
	if(access(src_name,0)!=0)
	{
		fprintf(stderr,"file not exist\n");
		exit(1);
	}

	printf("---------------Uncompress file begin--------------\n");
	printf("compress file name is：%s\n",src_name);
	FILE *ptr_src=NULL;
	open_file(src_name,&ptr_src);
	long compress_file_size=0;
	fseek(ptr_src,0L,SEEK_END);
	compress_file_size=ftell(ptr_src);//文件字节数
	if(compress_file_size==-1)
		fprintf(stderr,"compress file too big to dispaly its size(bigger than 2.1G)\nbut compress will run normally\n");
	else
		printf("compress file size:%ld bytes\n",compress_file_size);
	fseek(ptr_src,0L,SEEK_SET);

	long last_bits=0;
	char *file_name=NULL;
	HuffmanEncode *huffman_encode_array=(HuffmanEncode *)malloc(sizeof(HuffmanEncode)*256);
	if(huffman_encode_array==NULL)
	{
		fprintf(stderr,"Uncompress: malloc huffman_encode_array failed\n");
		exit(1);
	}
	
	int i;
	for(i=0;i<256;++i)
	{
		huffman_encode_array[i].encode=NULL;
		huffman_encode_array[i].length_of_encode=0;
	}

	parse_head(&ptr_src,huffman_encode_array,&file_name,&last_bits);
	HuffmanNode *huffman_tree_root=NULL;
	restore_huffman_tree(&huffman_tree_root,huffman_encode_array);

	FILE *ptr_dst=NULL;
	create_Uncompress_file(file_name,&ptr_dst);

	EncodeBuffer *encode_buffer=(EncodeBuffer *)malloc(sizeof(EncodeBuffer));
	DecodeBuffer *decode_buffer=(DecodeBuffer *)malloc(sizeof(DecodeBuffer));
	if(encode_buffer==NULL||decode_buffer==NULL)
	{
		fprintf(stderr,"Uncompress: encode/decode buffer malloc failed\n");
		exit(1);
	}

	encode_buffer->bits_num_lastbytes=0;
	encode_buffer->buffer=NULL;
	encode_buffer->size=0;

	const long enbuffer_size=8*1024*1024;
	const long debuffer_size=10*enbuffer_size;
	encode_buffer->buffer=(unsigned char *)malloc(enbuffer_size*sizeof(unsigned char));
	if(encode_buffer->buffer==NULL)
	{
		fprintf(stderr,"Uncompress: EncodeBuffer->buffer malloc failed\n");
		exit(1);
	}

	decode_buffer->size=0;
	decode_buffer->buffer=(unsigned char *)malloc(debuffer_size*sizeof(unsigned char));
	if(decode_buffer->buffer==NULL)
	{
		fprintf(stderr,"Uncompress: EncodeBuffer->buffer malloc failed\n");
		exit(1);
	}

	long long read_compress_file_size=0;
	long read_bytes=0;
	HuffmanNode *last_stop_at=NULL;
	while((read_bytes=fread(encode_buffer->buffer,sizeof(unsigned char),enbuffer_size,ptr_src))!=0)
	{
		read_compress_file_size+=read_bytes;
		if(read_compress_file_size==compress_file_size)
			if(last_bits>0)
			{
				encode_buffer->bits_num_lastbytes=last_bits;
				encode_buffer->size=read_bytes-1;
			}
			else
			{
				encode_buffer->bits_num_lastbytes=0;
				encode_buffer->size=read_bytes;
			}
		else
		{
			encode_buffer->bits_num_lastbytes=0;
			encode_buffer->size=read_bytes;
		}

		huffman_decode(huffman_tree_root,&last_stop_at,encode_buffer,decode_buffer);
		long write_bytes=fwrite(decode_buffer->buffer,sizeof(unsigned char),decode_buffer->size,ptr_dst);

		if(write_bytes!=decode_buffer->size)
		{
			fprintf(stderr,"Uncompress: fwrite failed\n");
			exit(1);
		}
	}
	printf("Uncompress file succeed\n");
	if(huffman_encode_array!=NULL)
	{
		free(huffman_encode_array);
		huffman_encode_array=NULL;
	}
	if(encode_buffer->buffer!=NULL)
	{
		free(encode_buffer->buffer);
		encode_buffer->buffer=NULL;
	}
	if(decode_buffer->buffer!=NULL)
	{
		free(decode_buffer->buffer);
		decode_buffer->buffer=NULL;
	}
	if(decode_buffer!=NULL)
	{
		free(decode_buffer);
		decode_buffer=NULL;
	}

	if(fclose(ptr_src)!=0)
	{
		fprintf(stderr,"uncompress: fclose error\n");
		exit(1);
	}
	return 1;
}
