#include "userprog/segdir.h"

//segment number
int segmentnumber = 0;

//allocate memory block
SEGMENTBLOCK *allocateMemBlock(size_t size){
    SEGMENTBLOCK *block = sbrk(0);
    if(sbrk(BLOCK_SIZE + size) == (void*)-1){
        return NULL;
    }else{
        block->next = NULL;
        block->prev = NULL;
        block->isfree = false;
        block->size = size;
        return block;
    }
}

//allocate memory block for Segmentation
MEMSEGMENTBLOCKS *allocateMemSegmentBlock(size_t size)
{
    MEMSEGMENTBLOCKS *block = sbrk(0);
    if(sbrk(MEM_SEGMENT_BLOCK_SIZE + size) == (void*) - 1){ return NULL;    }
    else{
        sbrk(MEM_SEGMENT_BLOCK_SIZE + size);
        block->next = NULL;
        block->isfree = false;
        block->size = size;
        block->memoryaddress = sbrk(0);
        return block;
    }
}

//allocate next memory block for Segmentation
void allocateNextMemSegmentBlock(size_t size, MEMSEGMENTBLOCKS **head)
{
    MEMSEGMENTBLOCKS *current = *head;
    void *allocate_mem = NULL;
    if(current == NULL){
        *head = allocateMemSegmentBlock(size);
    }else{
        while(current->next != NULL){
            current = current->next;
        }
        MEMSEGMENTBLOCKS *newblock = sbrk(0);

        allocate_mem = sbrk(MEM_SEGMENT_BLOCK_SIZE + size);      
        if(allocate_mem == (void*) - 1){ }
        else{
            sbrk(MEM_SEGMENT_BLOCK_SIZE + size);
            newblock->next = NULL;
            newblock->isfree = false;
            newblock->size = size;
            newblock->memoryaddress = sbrk(0);
            current->next = newblock;
      }
    }
}


// allocate memory using segmentation by creating parts of each process
void allocateMemory(PROCINTNODE **procinthead, MEMSEGMENTBLOCKS **memSegBlockshead)
{
    PROCINTNODE *current = *procinthead;
    int total_size = 0;

    printf("\nsbrk(0) = %p\n", sbrk(0));

    while(current != NULL){
        if(current->size <= MAX_SEGMENT_SIZE){
            void *start_address = sbrk(0) + 1;
            allocateNextMemSegmentBlock(current->size + 1, &(*memSegBlockshead));
            void *end_address = sbrk(0);
            total_size += current->size;
            current->size = 0;
            current = current->next;
            segmentnumber++;
        }else{
                if(total_size + MAX_SEGMENT_SIZE < MAX_MEM_SIZE){
                    void *start_address = sbrk(0) + 1;
                    allocateNextMemSegmentBlock(MAX_SEGMENT_SIZE, &(*memSegBlockshead));
                    void *end_address = sbrk(0);
                    total_size += MAX_SEGMENT_SIZE;
                    current->size = current->size - MAX_SEGMENT_SIZE;
                    segmentnumber++;
                }else{
                    total_size = 0;
                    break;
                }
            }
    }

    //delete memory nodes of size 0 or used
    current = *procinthead;
    deleteProcIntNodeZeroData(&current);
    current = *procinthead;

    if(countPROCINTNodes(current) == 1){
        if(current->size == 0){ *procinthead = NULL;}
    }else{
            *procinthead = (*procinthead)->next;
    }

    //call function to continue process to all processes
    allocateMemoryRemain(&(*procinthead), &(*memSegBlockshead));
}

// get the start_address of free memory for next segment of process
void getFreeMemoryAddress(MEMSEGMENTBLOCKS *memSegBlockshead, unsigned int size, void **start_address)
{
    MEMSEGMENTBLOCKS *current = memSegBlockshead;

    int size_temp = current->size;

    while(current != NULL){
        if(current->isfree){
            if(size_temp >= size){
                *start_address=current->memoryaddress;
                size_temp = 0;
                break;
            }
            size_temp += current->size;
            current = current->next;
        }else{ current = current->next; }
    }
}

// allocate memory to remaining segments
void allocateMemoryRemain(PROCINTNODE **procinthead, MEMSEGMENTBLOCKS **memSegBlockshead)
{
    PROCINTNODE *current = *procinthead;
    int total_size = 0;
    MEMSEGMENTBLOCKS *segcurrent = *memSegBlockshead;

    //lets free the used memory
    while(segcurrent != NULL){
        segcurrent->isfree = true;
        segcurrent = segcurrent->next;
    }
    
    segcurrent = *memSegBlockshead;
    void *start_address;
    
    while(current != NULL){
        if(current->size <= MAX_SEGMENT_SIZE){
            if(total_size + current->size < MAX_MEM_SIZE){
                
                getFreeMemoryAddress(*memSegBlockshead, current->size, &start_address);

                void *end_address = start_address + current->size;

                total_size += current->size;

                current->size = 0;
                current = current->next;
                segmentnumber++;
            }else{
                total_size = 0;
                break;
            }
        }else{
                if(total_size + MAX_SEGMENT_SIZE < MAX_MEM_SIZE){

                    if(segcurrent->next == NULL)
                        segcurrent = *memSegBlockshead;
                    else
                        segcurrent = segcurrent->next;

                    getFreeMemoryAddress(segcurrent, MAX_SEGMENT_SIZE, &start_address);

                    void *end_address = start_address + MAX_SEGMENT_SIZE;

                    total_size += MAX_SEGMENT_SIZE;

                    current->size = current->size - MAX_SEGMENT_SIZE;
                    segmentnumber++;
                }else{
                    total_size = 0;
                    break;
                }
            }
    }    
}

void Init(SEGMENTBLOCK *s_blockHead, MEMSEGMENTBLOCKS *memSegBlocksHead, INTNODE *dataList)
{
    //get data from input file
    PROCINTNODE *procintHead = NULL;
    procintHead = getDataFromINTNODEList(dataList, &procintHead);
    do{
        if(procintHead->next == NULL){
            if(procintHead->size == 0)
                break;
        }else{
            deleteProcIntNodeZeroData(&procintHead);
            allocateMemory(&procintHead, &memSegBlocksHead);
        }
    }while(procintHead != NULL);

}
