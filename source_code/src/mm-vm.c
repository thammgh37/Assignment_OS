/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "mm.h"
#ifdef MM_PAGING
#include "string.h"
#include <stdlib.h>
#include <stdio.h>

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
// DIFF
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;


  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;

  /* Enlist and combine new region */
  rg_elmt->rg_next = NULL;
  // Free list empty
  if (rg_node == NULL) {
    mm->mmap->vm_freerg_list = rg_elmt;
  }
  
  else if(rg_elmt->rg_end <= rg_node->rg_start) {
    // If region have address before the head region of the list
    if(rg_elmt->rg_end < rg_node->rg_start) {
      rg_elmt->rg_next = rg_node;
      mm->mmap->vm_freerg_list = rg_elmt;
    }
    // Right after the head
    else {
      rg_node->rg_start = rg_elmt->rg_start;
      free(rg_elmt);
    }
  }
  else {
    while(1) {
      if(rg_node->rg_end < rg_elmt->rg_start) {
        if(rg_node->rg_next == NULL) {
          rg_node->rg_next = rg_elmt;
          break;
        }
        else {
          if(rg_node->rg_next->rg_start > rg_elmt->rg_end) {
            rg_elmt->rg_next = rg_node->rg_next;
            rg_node->rg_next = rg_elmt;
            break;
          }
          else if(rg_node->rg_next->rg_start == rg_elmt->rg_end) {
            rg_node->rg_next->rg_start = rg_elmt->rg_start;
            free(rg_elmt);
            break;
          }
        }
      }
      else if(rg_node->rg_end == rg_elmt->rg_start) {
        if(rg_node->rg_next == NULL) {
          rg_node->rg_end = rg_elmt->rg_end;
          free(rg_elmt);
          break;
        }
        else {
          if(rg_node->rg_next->rg_start > rg_elmt->rg_end) {
            rg_node->rg_end = rg_elmt->rg_end;
            free(rg_elmt);
            break;
          }
          else if(rg_node->rg_next->rg_start == rg_elmt->rg_end) {
            struct  vm_rg_struct *next_rg;
            next_rg = rg_node->rg_next;
            rg_node->rg_end = rg_node->rg_next->rg_end;
            rg_node->rg_next = rg_node->rg_next->rg_next;
            free(next_rg);
            free(rg_elmt);
            break;
          }
        }
      }
      rg_node = rg_node->rg_next;
    }
  }
  return 0;
}

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma= mm->mmap;
 
  if(mm->mmap == NULL)
    return NULL;

  int vmait = 0;
  
  while (vmait < vmaid)
  {
    if(pvma == NULL)
	    return NULL;

    pvma = pvma->vm_next;
    vmait++;
  }

  return pvma;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if(rgid < 0 || rgid >= PAGING_MAX_SYMTBL_SZ)
    return NULL;

  if(mm->allocated[rgid] == 0) {
    return NULL;
  }
  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  /* Allocating invalid size */
  if(size <= 0) {
    return -1;
  }

  /* Set allocated value to 1 */
  caller->mm->allocated[rgid] = 1;

  /*Allocate at the toproof */
  struct vm_rg_struct rgnode;

  // find free memory area
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;

    *alloc_addr = rgnode.rg_start;
    return 0;
  }

  /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /*Attempt to increate limit to get space */
  struct vm_area_struct *vma_current = get_vma_by_num(caller->mm, vmaid);
  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  //int inc_limit_ret
  int old_sbrk_pointer;

  old_sbrk_pointer = vma_current->sbrk;

  /* TODO INCREASE THE LIMIT*/
  inc_vma_limit(caller, vmaid, inc_sz);

  /*Successful increase limit */
  caller->mm->symrgtbl[rgid].rg_start = old_sbrk_pointer;
  caller->mm->symrgtbl[rgid].rg_end = old_sbrk_pointer + size;

  *alloc_addr = old_sbrk_pointer;

  // allocate free region is larger than the request
  if(old_sbrk_pointer + size < vma_current->sbrk) {
    struct vm_rg_struct* free_rg = malloc(sizeof(struct vm_rg_struct));
    free_rg->rg_start = old_sbrk_pointer + size;
    free_rg->rg_end = vma_current->sbrk;
    free_rg->rg_next = NULL;
    enlist_vm_freerg_list(caller->mm, free_rg);
  }
  return 0;
}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  struct vm_rg_struct* rgnode = malloc(sizeof(struct vm_rg_struct));

  if(rgid < 0 || rgid >= PAGING_MAX_SYMTBL_SZ)
    return -1;

  /* Check double free */
  if(caller->mm->allocated[rgid] == 0) {
    return -1;
  }

  /* TODO: Manage the collect freed region to freerg_list */
  rgnode->rg_start = caller->mm->symrgtbl[rgid].rg_start;
  rgnode->rg_end = caller->mm->symrgtbl[rgid].rg_end;

  /* Reset allocated value */
  caller->mm->allocated[rgid] = 0;

  /*enlist the obsoleted memory region */
  enlist_vm_freerg_list(caller->mm, rgnode);

  return 0;
}

/*pgalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int pgalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;
   /* By default using vmaid = 0 */
  int returned_val = __alloc(proc, 0, reg_index, size, &addr);
  if(returned_val < 0) {
    printf("allocating error: invalid size value\n");
    return returned_val;
  }
#ifdef IO_DUMP
  printf("allocate region=%d reg=%d\n", size, reg_index);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
#ifdef REG_DUMP
  printf("------FREE REGION CONTENT------\n");
  print_list_rg(proc->mm->mmap->vm_freerg_list);
#endif
#endif

  return returned_val;
}

/*pgfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int pgfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  int returned_val = __free(proc, 0, reg_index);
  if(returned_val < 0) {
    printf("free error: double free\n");
    return returned_val;
  }
#ifdef IO_DUMP
  printf("free reg=%d\n", reg_index);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
#ifdef REG_DUMP
  printf("------FREE REGION CONTENT------\n");
  print_list_rg(proc->mm->mmap->vm_freerg_list);
#endif
#endif
  return returned_val;
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];
 
  if (PAGING_PAGE_SWAPPED(pte))
  { /* Page is not online, make it actively living */
    int vicpgn; // victim page
    int vicfpn; // victim frame
    int tgtfpn; // target frame
    int swpfpn; // swap frame
    uint32_t vicpte;

    /* Find victim page */
    find_victim_page(caller->mm, &vicpgn);

    /* Find victim frame */
    vicfpn = REVISE_PAGING_FPN(mm->pgd[vicpgn]);
    
    /* The target frame storing our variable */
    tgtfpn = REVISE_PAGING_SWP(pte);

    /* Get free frame in MEMSWP */
    MEMPHY_get_freefp(caller->active_mswp, &swpfpn);

    /* Do swap frame from MEMRAM to MEMSWP and vice versa*/
    /* Copy victim frame to swap */
    __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
   
    /* Copy target frame from swap to mem */
    __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, vicfpn);
    MEMPHY_put_freefp(caller->active_mswp, tgtfpn);

    /* Update page table */
    // pte_set_swap() &mm->pgd;

    /* Update its online status of the target page */
    vicpte = 0;
    init_pte(&vicpte, 1, 0, 0, 1, 0, swpfpn);
    mm->pgd[vicpgn] = vicpte;

    pte = 0;
    init_pte(&pte, 1, vicfpn, 0, 0, 0, 0);
    mm->pgd[pgn] = pte;

    enlist_pgn_node(&caller->mm->fifo_pgn, &caller->mm->fifo_pgn_tail, pgn);
  }
  *fpn = REVISE_PAGING_FPN(pte);
  return 0;
}
/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if(pg_getpage(mm, pgn, &fpn, caller) != 0) 
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_read(caller->mram, phyaddr, data);

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_write(caller->mram, phyaddr, value);

  return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;

  /* Read out of region range */
  if(currg->rg_start + offset >= currg->rg_end) {
    return -1;
  }

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}

/*pgwrite - PAGING-based read a region memory */
int pgread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t destination)
{
  BYTE data;
  int returned_val = __read(proc, 0, source, offset, &data);
  if(returned_val < 0) {
    printf("reading error: not allocated or out of region range\n");
    return returned_val;
  }

  destination = (uint32_t) data;
#ifdef IO_DUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#endif
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  return returned_val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  
  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;

  /* Write out of region range */
  if(currg->rg_start + offset >= currg->rg_end) {
    return -1;
  }

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*pgwrite - PAGING-based write a region memory */
int pgwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
  int returned_val = __write(proc, 0, destination, offset, data);
  if(returned_val < 0) {
    printf("writing error: not allocated or out of region range\n");
    return returned_val;
  }
#ifdef IO_DUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
#ifdef MEM_DUMP
  MEMPHY_dump(proc->mram);
#endif
#endif
  return returned_val;
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  uint32_t start = 0;
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, 0);
  uint32_t end = cur_vma->vm_end;

  int pgn_start = PAGING_PGN(start);
  int pgn_end = PAGING_PGN(end);
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = pgn_end - 1; pagenum >= pgn_start; pagenum--)
  {
    pte = caller->mm->pgd[pagenum];
    if(PAGING_PAGE_PRESENT(pte))
    {
      if(PAGING_PAGE_SWAPPED(pte)) { 
        fpn = REVISE_PAGING_SWP(pte);
        MEMPHY_put_freefp(caller->active_mswp, fpn);    
      }
      else {
        fpn = REVISE_PAGING_FPN(pte);
        MEMPHY_put_freefp(caller->mram, fpn);
      }
    }
  }
  return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
 struct vm_rg_struct * newrg;
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  newrg = malloc(sizeof(struct vm_rg_struct));

  newrg->rg_start = cur_vma->sbrk;
  newrg->rg_end = newrg->rg_start + size;

  return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{
  struct vm_area_struct *vma = caller->mm->mmap;
  return 0;
  /* TODO validate the planned memory area is not overlapped */
  while (vma != NULL)
  {
    if (vma->vm_id != vmaid && vmaend > vma->vm_start && vmastart < vma->vm_end)
    {
      return -1; // overlap detected 
    }
      vma = vma->vm_next;
  }
  return 0;
}
  /*inc_vma_limit - increase vm area limits to reserve space for new variable
   *@caller: caller
   *@vmaid: ID vm area to alloc memory region
   *@inc_sz: increment size
   *
   */
  int inc_vma_limit(struct pcb_t * caller, int vmaid, int inc_sz)
  {
    struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
    int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
    int incnumpage =  inc_amt / PAGING_PAGESZ;

    // Get region from mm->sbrk => mm->sbrk + inc_sz
    struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

    int old_end = cur_vma->vm_end;

    /*Validate overlap of obtained region */
    if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
      return -1; /*Overlap and failed allocation */

    /* The obtained vm area (only) 
    * now will be alloc real ram region */
    cur_vma->vm_end += inc_sz;
    cur_vma->sbrk += inc_sz;
    if (vm_map_ram(caller, area->rg_start, area->rg_end, 
                      old_end, incnumpage , newrg) < 0)
      return -1; /* Map the memory to MEMRAM */

    return 0;
  }

  /*find_victim_page - find victim page
   *@caller: caller
   *@pgn: return page number
   *
   */
  int find_victim_page(struct mm_struct * mm, int *retpgn)
  {
    struct pgn_t *pg = mm->fifo_pgn;

    *retpgn = pg->pgn;

    /* TODO: Implement the theorical mechanism to find the victim page */
    if(pg->pg_next == NULL) {
      mm->fifo_pgn = NULL;
      mm->fifo_pgn_tail = NULL;
    }
    else {
      mm->fifo_pgn = mm->fifo_pgn->pg_next;
    }
    free(pg);

    return 0;
  }

  /*get_free_vmrg_area - get a free vm region
   *@caller: caller
   *@vmaid: ID vm area to alloc memory region
   *@size: allocated size
   *
   */
  int get_free_vmrg_area(struct pcb_t * caller, int vmaid, int size, struct vm_rg_struct *newrg)
  {
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL)
    return -1;

  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;

  /* Traverse on list of free vm region to find a fit space */
  while (rgit != NULL)
  {
    if (rgit->rg_start + size <= rgit->rg_end)
    { /* Current region has enough space */
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start + size;

      /* Update left space in chosen region */
      if (rgit->rg_start + size < rgit->rg_end)
      {
        rgit->rg_start = rgit->rg_start + size;
      }
      else
      { /*Use up all space, remove current node */
        /*Clone next rg node */
        struct vm_rg_struct *nextrg = rgit->rg_next;

        /*Cloning */
        if (nextrg != NULL)
        {
          rgit->rg_start = nextrg->rg_start;
          rgit->rg_end = nextrg->rg_end;

          rgit->rg_next = nextrg->rg_next;

          free(nextrg);
        }
        else
        { /*End of free list */
          rgit->rg_start = rgit->rg_end;	//dummy, size 0 region
          rgit->rg_next = NULL;
        }
      }
      break;
    }
    else
    {
      rgit = rgit->rg_next;	// Traverse next rg
    }
  }

  if(newrg->rg_start == -1) // new region not found
   return -1;

  return 0;
}

#endif
