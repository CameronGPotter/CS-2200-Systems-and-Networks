#include "paging.h"
#include "pagesim.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/*  --------------------------------- PROBLEM 6 --------------------------------------
    Checkout PDF section 7 for this problem

    Page fault handler.

    When the CPU encounters an invalid address mapping in a page table,
    it invokes the OS via this handler.

    Your job is to put a mapping in place so that the translation can
    succeed. You can use free_frame() to make an available frame.
    Update the page table with the new frame, and don't forget
    to fill in the frame table.

    When you access a frame, you should update its referenced bit.

    Lastly, you must fill your newly-mapped page with data. If the page has
    never been mapped before, you must read an empty page from disk space and
    update the diskmap. Otherwise, the data will ahve been swapped to the disk
    when the page was evicted last. Call swap_read() to pull the back in from
    swap space.

    HINTS:
         - You will need to use the global variable current_process when
           setting the frame table entry.

    ----------------------------------------------------------------------------------
 */
void page_fault(vaddr_t address) {
   /* First, split the faulting address and locate the page table entry.
      Remember to keep a pointer to the entry so you can modify it later. */

   vpn_t vpn = vaddr_vpn(address);
   pte_t *pte = (pte_t*) ((PTBR * PAGE_SIZE) + mem) + vpn;
   
   /* It's a page fault, so the entry obviously won't be valid. Grab
      a frame to use by calling free_frame(). */

   pfn_t pfn = free_frame();

   /* Update the page table entry. Make sure you set any relevant values. */

   pte->pfn = pfn;
   pte->valid = 1;
   pte->dirty = 0;

   /* Update the frame table. Make sure you set any relevant values. */

   frame_table[pfn].vpn = vpn;
   frame_table[pfn].mapped = 1;
   frame_table[pfn].process = current_process;
   frame_table[pfn].referenced = 1;

   /* Update the timestamp of the appropriate frame table entry with the provided get_current_timestamp function. Timestamps values are used by the FIFO algorithm. */
    
   frame_table[pfn].timestamp = get_current_timestamp();

   /* Initialize the page's memory. On a page fault, it is not enough
    * just to allocate a new frame. We must load in the old data from
    * disk into the frame. If there was no old data on disk, then
    * we need to clear out the memory (why?).
    *
    * 1) Get a pointer to the new frame in memory.
    * 2) If the page has swap set, then we need to load in data from memory
    *    using swap_read().
    * 3) Else, read the page from the diskmap using diskmap_read()
    */

   paddr_t* nf = (paddr_t*) ((PAGE_SIZE * pfn) + mem);
   if (swap_exists(pte)) {
      swap_read(pte, nf);
   } 
   else {
      diskmap_read(vpn, current_process->diskmap, nf);
   }
   stats.page_faults += 1;
}

#pragma GCC diagnostic pop
