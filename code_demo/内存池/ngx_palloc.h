
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

// 定于小块内存  默认大小是4095
// 小内存 < NGX_MAX_ALLOC_FROM_POOL +sizeof(ngx_pool_s)
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)
//内存池的默认大小，目前是16K
#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)

//为了传参，申请内存时的参数，是16的倍数 对齐字节数
#define NGX_POOL_ALIGNMENT       16
//这个这里没有用到
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)

//取得最后的字节对齐地址
// #define ngx_align_ptr(p, a)                                                   \
//     (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    //回调函数清理方法
    ngx_pool_cleanup_pt   handler;
    //handler回调函数的参数
    void                 *data;
    //构造成单链表，由ngx_pool_cleanup_add函数实现
    ngx_pool_cleanup_t   *next;
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

struct ngx_pool_large_s {
    ngx_pool_large_t     *next;
    void                 *alloc;
};


typedef struct {
    //未分配的空闲类型首地址
    u_char               *last;
    //当前小块内存池的尾部
    u_char               *end;
    //同属于一个pool的多个小块内存池，构成连败哦
    ngx_pool_t           *next;
    //当剩余空间不足以分配小块内存时，failed就会+1，当大于4后
    //current 会移向下一个内存池
    ngx_uint_t            failed;
} ngx_pool_data_t;


struct ngx_pool_s {
    //小块内存池  单链表
    ngx_pool_data_t       d;
    //申请内存是小块还是大块的标准
    size_t                max;
    //多个小块内存池构成链表，指向分配内存时遍历的第一个小块内存池
    ngx_pool_t           *current;
    //用于ngx_output_chain
    ngx_chain_t          *chain;
    //大块内存池从进程的堆中分配，单链表
    ngx_pool_large_t     *large;
    //所有待清理资源，单链表
    ngx_pool_cleanup_t   *cleanup;
    //内存池执行中输出日志的对象
    ngx_log_t            *log;
};


typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;


ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
