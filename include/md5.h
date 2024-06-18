#ifndef __MD5_H__
#define __MD5_H__

#include <stdio.h>
#include <string.h>

// 定义MD5相关的宏
#define MD5_LONG unsigned long
#define MD5_CBLOCK    64  // MD5处理的块大小（字节）
#define MD5_LBLOCK    (MD5_CBLOCK/4)  // MD5处理的块大小（长整型）
#define MD5_DIGEST_LENGTH 16  // MD5摘要长度（字节）

#define MD32_REG_T long
#define DATA_ORDER_IS_LITTLE_ENDIAN
#define INIT_DATA_A (unsigned long)0x67452301L  // MD5初始数据A
#define INIT_DATA_B (unsigned long)0xefcdab89L  // MD5初始数据B
#define INIT_DATA_C (unsigned long)0x98badcfeL  // MD5初始数据C
#define INIT_DATA_D (unsigned long)0x10325476L  // MD5初始数据D

// 定义位旋转宏
#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
// 将字符转换为长整型
#define HOST_c2l(c,l)    (l =(((unsigned long)(*((c)++)))    ),    \
             l|=(((unsigned long)(*((c)++)))<< 8),    \
             l|=(((unsigned long)(*((c)++)))<<16),    \
             l|=(((unsigned long)(*((c)++)))<<24)    )
// 将长整型转换为字符
#define HOST_l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff),    \
             *((c)++)=(unsigned char)(((l)>> 8)&0xff),    \
             *((c)++)=(unsigned char)(((l)>>16)&0xff),    \
             *((c)++)=(unsigned char)(((l)>>24)&0xff),    \
             l)
// 将哈希值转换为字符串
#define	HASH_MAKE_STRING(c,s)    do {    \
    unsigned long ll;        \
    ll=(c)->A; (void)HOST_l2c(ll,(s)); \
    ll=(c)->B; (void)HOST_l2c(ll,(s)); \
    ll=(c)->C; (void)HOST_l2c(ll,(s)); \
    ll=(c)->D; (void)HOST_l2c(ll,(s)); \
    } while (0)

// 定义MD5的四个基本函数
#define	F(b,c,d)    ((((c) ^ (d)) & (b)) ^ (d))
#define	G(b,c,d)    ((((b) ^ (c)) & (d)) ^ (c))
#define	H(b,c,d)    ((b) ^ (c) ^ (d))
#define	I(b,c,d)    (((~(d)) | (b)) ^ (c))

// 定义MD5的四个轮次函数
#define R0(a,b,c,d,k,s,t) { \
    a+=((k)+(t)+F((b),(c),(d))); \
    a=ROTATE(a,s); \
    a+=b; };
#define R1(a,b,c,d,k,s,t) { \
    a+=((k)+(t)+G((b),(c),(d))); \
    a=ROTATE(a,s); \
    a+=b; };

#define R2(a,b,c,d,k,s,t) { \
    a+=((k)+(t)+H((b),(c),(d))); \
    a=ROTATE(a,s); \
    a+=b; };

#define R3(a,b,c,d,k,s,t) { \
    a+=((k)+(t)+I((b),(c),(d))); \
    a=ROTATE(a,s); \
    a+=b; };

// 定义MD5上下文结构体
typedef struct MD5state_st1 {
    MD5_LONG A, B, C, D;  // MD5的四个寄存器
    MD5_LONG Nl, Nh;  // 处理的位数
    MD5_LONG data[MD5_LBLOCK];  // 数据块
    unsigned int num;  // 数据块中的字节数
} MD5_CTX;

// 函数声明
int MD5_Init(MD5_CTX *c);  // 初始化MD5上下文

void md5_block_data_order(MD5_CTX *c, const void *data_, size_t num);  // 处理数据块

int MD5_Update(MD5_CTX *c, const void *data_, size_t len);  // 更新MD5上下文

int MD5_Final(unsigned char *md, MD5_CTX *c);  // 生成MD5摘要

void OPENSSL_cleanse(void *ptr, size_t len);  // 清除内存

unsigned char *MD5(const unsigned char *d, size_t n, unsigned char *md);  // 计算MD5摘要

#endif