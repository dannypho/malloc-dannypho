#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)     ((b) + 1)
#define BLOCK_HEADER(ptr) ((struct _block *)(ptr) - 1)

static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes     */
   struct _block *next;  /* Pointer to the next _block of allocated memory      */
   struct _block *prev;  /* Pointer to the previous _block of allocated memory  */
   bool   free;          /* Is this _block free?                                */
   char   padding[3];    /* Padding: IENTRTMzMjAgU3jMDEED                       */
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   //
   // While we haven't run off the end of the linked list and
   // while the current node we point to isn't free or isn't big enough
   // then continue to iterate over the list.  This loop ends either
   // with curr pointing to NULL, meaning we've run to the end of the list
   // without finding a node or it ends pointing to a free node that has enough
   // space for the request.
   // 
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

// \TODO Put your Best Fit code in this #ifdef block
#if defined BEST && BEST == 0
	struct _block *best_block = NULL;
	size_t smallest_size = SIZE_MAX;

	while (curr)
	{
		if (curr->free && curr->size >= size && curr->size < smallest_size)
		{
			best_block = curr;
			smallest_size = curr->size;
		}
		*last = curr;
		curr = curr->next;
	}
	curr = best_block;
#endif

// \TODO Put your Worst Fit code in this #ifdef block
#if defined WORST && WORST == 0
	struct _block *worst_block = NULL;
	size_t largest_size = 0;

	while (curr)
	{
		if (curr->free && curr->size >= size && curr->size > largest_size)
		{
			worst_block = curr;
			largest_size = curr->size;
		}
		*last = curr;
		curr = curr->next;
	}
	curr = worst_block;
#endif

// \TODO Put your Next Fit code in this #ifdef block
#if defined NEXT && NEXT == 0
   static struct _block *last_allocated = NULL;

   // If the heapList is empty, return NULL
   if (heapList == NULL) 
   {
      return NULL;
   }

   // Start from the last allocated block or the beginning of the heap
   if (last_allocated == NULL) 
   {
      last_allocated = heapList;
   }
   curr = last_allocated;

   // Traverse the list of blocks
   do 
   {
      if (curr->free && curr->size >= size) 
      {
         last_allocated = curr; // Update the last allocated pointer
         break;                 // Suitable block found
      }
         *last = curr;
         curr = curr->next ? curr->next : heapList; // Wrap around
   } 
   while (curr != last_allocated);
   // If no suitable block was found, set curr to NULL
   if (!curr->free || curr->size < size) 
   {
      curr = NULL;
   }
#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to previous _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata:
      Set the size of the new block and initialize the new block to "free".
      Set its next pointer to NULL since it's now the tail of the linked list.
   */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   curr->prev = last; // Fix: Set the previous pointer
   num_grows++;
   num_blocks++;
   max_heap += size + sizeof(struct _block);
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block.  If a free block isn't found then we need to grow our heap. */

   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   if (next)
   {
      num_reuses++;
   }

   /* TODO: If the block found by findFreeBlock is larger than we need then:
            If the leftover space in the new block is greater than the sizeof(_block)+4 then
            split the block.
            If the leftover space in the new block is less than the sizeof(_block)+4 then
            don't split the block.
   */
   if (next && next->size > size + sizeof(struct _block) + 4)
	{
		struct _block *new_block = (struct _block *)((char *)BLOCK_DATA(next) + size);
		new_block->size = next->size - size - sizeof(struct _block);
		new_block->free = true;
		new_block->next = next->next;
      new_block->prev = next;
      if (new_block->next)
      {
         new_block->next->prev = new_block;
      }
		next->next = new_block;
		next->size = size;

		num_splits++;
		num_blocks++;
	}

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;

   num_requested += size;
   num_mallocs++;

   /* Return data address associated with _block to the user */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;


   /* TODO: Coalesce free _blocks.  If the next block or previous block 
            are free then combine them with this block being freed.
   */
   if (curr->prev && curr->prev->free) 
   {
      curr->prev->size += sizeof(struct _block) + curr->size;
      curr->prev->next = curr->next;
      if (curr->next)
      {
         curr->next->prev = curr->prev;
      }
      curr = curr->prev; // Update curr to the coalesced block
      num_coalesces++;
      num_blocks--;
   }
   // Coalesce with the next block if it's free
   if (curr->next && curr->next->free) 
   {
      curr->size += sizeof(struct _block) + curr->next->size;
      curr->next = curr->next->next;
      if (curr->next)
      {
         curr->next->prev = curr;
      }
      num_coalesces++;
      num_blocks--;
   }
   num_frees++;
}

void *calloc( size_t nmemb, size_t size )
{
   size_t total_size = nmemb * size;
   void *ptr = malloc(total_size);
   if (ptr) 
   {
      memset(ptr, 0, total_size);
   }
   return ptr;
}

void *realloc( void *ptr, size_t size )
{
   if (ptr == NULL) 
   {
      return malloc(size);
   }

   if (size == 0) 
   {
      free(ptr);
      return NULL;
   }

   struct _block *curr = BLOCK_HEADER(ptr);
   if (curr->size >= size) 
   {
      return ptr; // Current block is sufficient
   }

   void *new_ptr = malloc(size);
   if (new_ptr) 
   {
      memcpy(new_ptr, ptr, curr->size); // Copy old data
      free(ptr);
   }
   return new_ptr;
}



/* vim: IENTRTMzMjAgU3ByaW5nIDIwM001= ----------------------------------------*/
/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
