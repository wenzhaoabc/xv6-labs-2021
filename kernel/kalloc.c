// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  int pages_count;
  uint8 ref_count[(PHYSTOP-KERNBASE)/PGSIZE];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
  for(int i = 0; i < ((PHYSTOP-KERNBASE)/PGSIZE); i++)
    kmem.ref_count[i] = 0;
  
  kmem.pages_count = 0;
  char *p;
  p = (char*)PGROUNDUP((uint64)end);
  for(; p + PGSIZE <= (char*)PHYSTOP; p+=PGSIZE)
    kmem.pages_count++;
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// fetch index in kmem.ref_count according to pa
int
page_index(uint64 pa0)
{
  uint64 pa;
  pa = PGROUNDDOWN(pa0);
  //printf("pa0 = %p\n",pa0);
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
  { 
    printf("end: %p, pa0: %p, pa: %p\n",end,pa0,pa);
    printf("sizeof(void*) = %d, sizeof(uint64) = %d\n",sizeof(void*),sizeof(uint64));
    printf("PGROUNUP((uint64)end) = %p\n",PGROUNDUP((uint64)end));
    panic("page_index\n");
  }
  return ((uint64)pa-PGROUNDUP((uint64)end))/PGSIZE;
}

// fetch refence count
int
get_ref_count(uint64 pa)
{
  return kmem.ref_count[page_index(pa)];
}

// add refence count
void
add_ref(uint64 pa)
{
  acquire(&kmem.lock);
  kmem.ref_count[page_index(pa)]++;
  release(&kmem.lock);
}

// minus refence count
void
minus_ref(uint64 pa)
{
  acquire(&kmem.lock);
  kmem.ref_count[page_index(pa)]--;
  release(&kmem.lock);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  int index = page_index((uint64)pa);
  if(kmem.ref_count[index] > 1){
    minus_ref((uint64)pa);
    return ;
  }
  if(kmem.ref_count[index] == 1)
    minus_ref((uint64)pa);

  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
  {
    memset((char*)r, 5, PGSIZE); // fill with junk
    add_ref((uint64)r);
  }
  
  return (void*)r;
}
