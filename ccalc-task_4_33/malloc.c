#include "malloc.h"
#include "syscall.h"
#include "mmap.h"
#include "util.h"

struct htable_list_item {
    void *addr;
    unsigned long len;
    struct htable_list_item *next; 
};

enum { hash_table_prime = 6151 };

struct htable_list_item *hash_table[hash_table_prime];


unsigned long get_size_in_pages(int size)
{
    unsigned long page_count;
    page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    return page_count * PAGE_SIZE;
}

static void htable_list_add(void *addr, unsigned long len, 
                            struct htable_list_item **lst)
{
    struct htable_list_item *tmp;
    unsigned long item_len;

    item_len = get_size_in_pages(sizeof(*tmp));
    tmp = mmap(NULL, item_len, PROT_READ|PROT_WRITE,
               MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    tmp->addr = addr;
    tmp->len = len;
    tmp->next = *lst;
    *lst = tmp;
}

static void htable_list_remove(void *addr, struct htable_list_item **lst)
{
    struct htable_list_item **item_ptr;
    struct htable_list_item *tmp;
    unsigned long item_len;

    item_ptr = lst;
    while (*item_ptr != NULL) {
        if ((*item_ptr)->addr != addr)
            item_ptr = &(*item_ptr)->next;

        tmp = *item_ptr;
        *item_ptr = tmp->next;
        item_len = get_size_in_pages(sizeof(*tmp));
        munmap(tmp->addr, tmp->len);
        munmap(tmp, item_len);
        break;
    }
}

static void hash_table_add(void *addr, unsigned long len)
{
    unsigned long pos;
    struct htable_list_item **lst_ptr;

    pos = (unsigned long)addr % hash_table_prime;
    lst_ptr = &hash_table[pos];
    htable_list_add(addr, len, lst_ptr);
}

static void hash_table_remove(void *addr)
{
    unsigned long pos;
    struct htable_list_item **lst_ptr;

    pos = (unsigned long)addr % hash_table_prime;
    lst_ptr = &hash_table[pos];
    if (!*lst_ptr)
        return;
    htable_list_remove(addr, lst_ptr);
}

void *malloc(unsigned int size)
{
    void *addr;
    unsigned long len;
    len = get_size_in_pages(size);
    addr = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,
                -1, 0);
    hash_table_add(addr, len);

    return addr;
}

void free(void *ptr)
{
    hash_table_remove(ptr);
}
