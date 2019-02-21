#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
//#include <io.h>

typedef struct HuffmanNode
{
	unsigned char data_8bit;//图片的存储是由一系列十六进制数组成
	unsigned long long data_8bit_count;//每个十六进制数出现的次数，需要遍历一次文件即可得到
	struct HuffmanNode *parent;//该结点的父节点
	struct HuffmanNode *left_child;//该结点的左孩子
	struct HuffmanNode *right_child;//该节点的有右孩子
}HuffmanNode;

//HuffmanEncode代表每个unsigned char的编码结果，以数组的方式来使用，huffman_code[256]
//unsigned char 共有256种情况，用数组方式比较方便，下表代表了对应的unsigned char
//假设unsigendchar 0x255的编码为字符串"00101"，我们在压缩的时候需要把"00101"以二进制'00101'
//的形式写入内存缓冲区，继而写入磁盘文件。我们不能直接把"00101"以\0为结束标志的字符串的形
//式写入内存缓冲区中，那样的话就达不到压缩的目的了，变成一个字符用5个字符来表示，显然不行
//我们应该挨个遍历字符如果是字符‘0’则就往内存对应的bit写入0，‘1’则对应的bit写入1
typedef struct HuffmanEncode
{
	char *encode;//由‘0’‘1’字符组成的字符串
	long length_of_encode;//字符串的长度
}HuffmanEncode;

//编码缓冲区，在压缩的时候：最初文件经过编码后存在内存中，再用fwrite把内存写入磁盘文件中
//			  在解压的时候：把压缩文件读入内存中，在根据HUffman树来恢复数据
//内存以EncodeBuffer数据结构来表示，示最后一个字节中包含的有效bit数量。这是因为一个文件经
//过编码后包含的bit数量不一定是8的整数倍，也就是说除以8后，余数可能是1--7中的任意一个，
//我们需要记住这个数字，最后一个字节的剩下的bit是不需要进行处理的。压缩、解压缩的过程都
//会使用到这个数据结构
typedef struct EncodeBuffer
{
	unsigned char *buffer;//通过malloc函数分配的一块内存
	long size;//该buffer存储的有效字节数量
	long bits_num_lastbytes;
}EncodeBuffer;

//解码缓冲区，我们把原始文件读入EncodeBuffer类型的内存缓冲区，然后根据恢复出来的huffman
//树来找到编码对应的‘原码’，这个‘原码’当然不能直接写入磁盘中的压缩文件中，这样写磁盘次数
//太多，性能会下降。而应该先把解码出来的数据写入内存缓冲区中，积累多了再一次写入磁盘中
//DecodeBuffer便是用来表示该缓冲区的
//与EncodeBuffer不同，这里没有bits_num_lastbytes这个字段，因为无论压缩文件的bit数是不是8
//的倍数，压缩前的文件bit数量肯定是8的倍数，因为存储于磁盘上的文件的最小的单位就是字节了
//因此解压出来的数据无需担心最后一个bit的有效数量的问题。
typedef struct DecodeBuffer
{
	unsigned char *buffer;
	long size;
}DecodeBuffer;

//CompressFileHeader类型的格式来存储压缩文件的元信息，以便解压的时候根据这些信息来恢复原文件
typedef struct CompressFileHeader
{
	long length_of_header1;//结构体自身长度，计入author_name等实际字符的个数
	long length_of_header2;//截至压缩文件正文前的文件的长度。2-1

	unsigned char length_of_author_name;//compress作者姓名字节长度
	unsigned char routine_name_length;//压缩文件的名称长度
	unsigned char suffix_length;//压缩文件的后缀长度，jcprs
	unsigned char file_name_length;//原始文件名的长度，子节数
	unsigned char bits_of_lastByte;

	char *author_name;//软件作者名
	char *routine_name;//压缩软件名
	char *suffix;//后缀，jcprs
	char *file_name;//原始文件名
}CompressFileHeader;

//1.堆排序部分
//	每一个接口函数的第一个参数huffman_node_array是一个HuffmanNode **类型的数据，实际上
//huffman_node_array是一个数组，而数组元素又是一个指针，该指针指向具体的HuffmanNode类型的
//结点数据。数组元素被设计为指针，是为了方便堆排序，排序的时候可以直接交换指针
//data_start和data_end标志了huffman_node_array的起始和结束的位置，这样比较灵活，我们可以
//选择性的对其中某一段数据进行堆排序。array_size表示了用于构造最小堆的数组的长度.
//	min_first和min_second是两个指针，其指向的内容又是一个指针，该指针指向得到的两个最小的堆
//元素。heap_size存储的是当前最小堆的长度。注意参数min_first和min_second都是指向指针的指
//针，为什么这么做呢？首先这个接口函数为了得到最小的两个堆元素的指针，那么在传递参数的时
//候如果传递的是HuffmanNode *min_first，那么即使函数内部把min_first设置成了正确的值，调用
//结束后，在主调函数里min_first依然保持了调用之前的状态，而没有发生丝毫改变，这是因为c函
//数是传值调用，参数均以值的形式进行传递，指针也不例外。这里有一个误区，很多人可能见过这
//样的情形：在调用函数里定义了一个变量：int a; 然后以func(&a)的形式来调用
//int func( int *a)，func内部做了动作*a=8,然后主调函数里面就会得到a的值为8.为什么这里不行
//在这个例子中，传递进来的是a的地址&a, &a的值始终没有变，只是改变了它指向的内存的内容；而
//对于heap_min_get2min()来说，如果传递的是HuffmanNode *类型的数据min_first，则min_first的
//本身的值永远不会变，只会改变它所指向的内存的内容，而我们的代码逻辑是要改名指针本身，为
//了达到这个目的，我们只能传递指针的指针，也即HuffmanNode **类型的min_first，在函数内部我
//们做类似这样的操作*min_first=xxxx,就可以做到改变min_first指向的内容，也即改
//变HuffmanNode *类型的数据，这正是我们要的结果
int heap_min_adjust(HuffmanNode **huffman_node_array,long data_start,long data_end);
int heap_min_construct(HuffmanNode **huffman_node_array,long array_size);
int heap_min_get2min(HuffmanNode **huffman_node_array,HuffmanNode **min_first,
					 HuffmanNode **min_second,long *heap_size);

//2.hufman编码部分
//huffman编码是根据huffman树而来的，走一条从根结点到叶子结点的路径就能得到叶子结点所代表
//的数据对应的编码。因此上述函数huffman_tree()的作用便是构造一颗huffman树，构造完毕
//的huffman树的根节点的指针会写入第一个参数指向的内存，第二个参数是调用者传递的huffman结
//点数组，第三个参数array_size表示huffman结点数组的元素个数。
//  函数huffman_encode的作用是：根据huffman_tree()构造出的huffman树，对于每一个叶子结点，
//走一条去往跟结点的路径，这样便得到了该叶子结点对应的huffman编码的逆序列，有了逆序列，自
//然就可以得到正序列了。第一个参数是一个存储叶子结点的数组，第二个参数是数组元素个数，第
//三个参数会返回最终每一个叶子结点对应的huffman编码
int huffman_tree(HuffmanNode **huffman_tree_root,HuffmanNode **huffman_node_array,
														long array_size);
int huffman_encode(HuffmanNode **huffman_node_array,long huffman_node_array_size,HuffmanEncode *huffman_encode_array);

//3.huffman解码部分
//huffman解码部分对应着业务逻辑中的‘解压缩’这个概念，为了使得代码逻辑清晰，解码功能被拆分
//为两个函数。huffman_decode()会在内部调用huffman_decode_internal() 。其具体功能将在后文
//的源代码部分有比较详细的解析，这里大概描述下解码的功能：首先解码需要根据huffman树来解码
//因此会有其他函数来负责根据压缩文件头里的编码表来恢复哈夫曼树。其次需要读取压缩文件的正
//文到内存缓冲区里面，这正是我们要解码的对象。最后解码后的结果要存储在两外一块内存缓冲区
//里面，等待最后写入磁盘
int huffman_decode_internal(HuffmanNode **huffman_tree_current_root,long direction);
int huffman_decode(HuffmanNode *huffman_tree_root,HuffmanNode **last_stop_at,
					EncodeBuffer *encode_buffer,DecodeBuffer *decode_buffer);

//4.业务逻辑部分
//特定字节*ch的第index个bit设置为0或者1，具体根据flag的指示
int bit_set(unsigned char *ch,long index,long flag);
int open_file(char *filename,FILE **fp);
int create_file(char *filename,FILE **fp);
int make_uchar_weight(FILE *fp,HuffmanNode **huffman_node,long *size);
int create_Uncompress_file(char *filename,FILE **fp);

//以下四个函数他们在1，2，3 中描述的函数的基础上负责上层的压缩、解压缩逻辑。compress()是
//用来对文件进行压缩的，而后三个函数是为了解压缩的，首先JparseHead()会读取压缩文件的头部
//并且解析出必要的信息，比如编码表，然后restore_huffman_tree()会根据解析出的编码表来重建
//huffman树，这个重建的huffman树与压缩时创建的huffman树一模一样，否则的话解码出的结果就
//是不正确的。而Uncompress()会在这两个函数的基础上做具体的解压缩操作。
int parse_head(FILE **fp,HuffmanEncode *huffman_encode_array,char **file_name,
																	long *last_bits);
int compress(char *src_name);
int Uncompress(char *src_name);
int restore_huffman_tree(HuffmanNode **huffman_tree_root,
										HuffmanEncode *huffman_encode_array);

