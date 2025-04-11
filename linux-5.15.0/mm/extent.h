#include "linux/types.h"
#include <linux/list.h>
#include <linux/rbtree.h>

#ifndef EXTENT_H        
#define EXTENT_H        

struct extent {
        /* Metadata for the extent */
        unsigned long start_phys;     // Starting physical address
        unsigned long end_phys;       // Ending physical address
        unsigned long start_pfn;      // Starting page frame number
        unsigned long end_pfn;        // Ending page frame number
        struct page *first_page;      // Pointer to the first page in this extent
        unsigned int npages;          // Number of pages in this extent
        unsigned int id;              // Unique extent ID (incremental)

        /* For grouping contiguous physical pages */
        struct list_head page_list;   // Head of list for contiguous pages
        /* For red-black tree insertion */
        struct rb_node rb_node;
};

struct extent_page {
        struct page *page;            // Pointer to the page
        struct list_head list;       // List head for linking pages
};

extern int insert_phys_page(struct rb_root *root, struct page *page);

#endif