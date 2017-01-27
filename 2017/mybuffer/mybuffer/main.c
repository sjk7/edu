// main.c
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <assert.h>

#define T char

typedef struct type4 { T x[16]; } static_buffer16;
typedef struct type5 { T x[32]; } static_buffer32;
typedef struct type6 { T x[64]; } static_buffer64;
typedef struct type7 { T x[128]; } static_buffer128;
typedef struct type8 { T x[256]; } static_buffer256;
typedef struct type9 { T x[512]; } static_buffer512;
typedef struct type10 { T x[1024]; } static_buffer1024;
typedef struct type11 { T x[2048]; } static_buffer2048;
typedef struct type12 { T x[4096]; } static_buffer4096;
typedef struct type13 { T x[8192]; } static_buffer8192;

typedef struct ptr
{
	T* p; T* e; T* c;
}PTR, *PPTR;

typedef struct buf
{
	PTR b;

}BUF, *PBUF;

typedef struct statbuf16
{
	static_buffer16 sb;
	PTR stat;
	PTR dyn;
}B16, *B16;


void buf_init(PBUF b) { memset(b, 0, sizeof(BUF)); }
int buf_alloc(PBUF b, int sz) { 
	
	if (!b->b.p) {
		b->b.p = malloc(sz);
		b->b.c = b->b.p + sz;
		b->b.e = b->b.p;
	}
	else {
		int oldsz = b->b.c - b->b.p;
		int old_diff = b->b.e - b->b.p;
		assert(old_diff >= 0);
		T* p = realloc(b->b.p, sz);
		if (p) {
			b->b.p = p;
			b->b.e = b->b.p + old_diff;
			b->b.c = b->b.p + sz;
		}
		else {
			sz = oldsz;
		}
	}
 
	return b->b.p != 0 ? sz : 0; 
}

int buf_copy_in(PBUF b, PTR pp, int dst_start)
{
	int bsz = (b->b.c - b->b.p) - dst_start;
	if (bsz <= 0) return bsz;
	int ppsz = pp.e - pp.p;
	int cpy_sz = bsz <= ppsz ? bsz : ppsz;
	assert(cpy_sz >= 0);
	if (cpy_sz) {
		memcpy(b->b.p + dst_start, pp.p, cpy_sz);
		b->b.e += cpy_sz;
	}

	return cpy_sz;
}

int ptr_sz(PTR* p) { return p->e - p->p; }
int ptr_cap(PTR* p) { return p->c - p->p; }
void buf_free(PBUF p) { if (p->b.p) free(p->b.p); buf_init(p); }

int main() {
	puts("hello\n");
	BUF b;
	buf_init(&b);
	buf_alloc(&b, 1024);
	PTR pt; pt.p = "Hello buffer!"; pt.e = pt.p + strlen("Hello, buffer!") + 1;
	int cp = buf_copy_in(&b, pt, 0);
	int sz = ptr_sz(&b.b);
	puts(b.b.p);
	assert(sz == ptr_sz(&pt));

	buf_free(&b);
	return 0;
}