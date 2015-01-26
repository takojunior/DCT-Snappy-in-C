/*
	Test 2-D DCT on 8x8 blocks.

	Use the following command to compile this file:

gcc dcttest.c -lm

*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>
#include 	<time.h>

#define		BLOCK_N		8
#define 	PI 	(3.141592653589793)

extern	void dct_2d_8x8_asm (float *buf, int n); // this is the function you need to implement 

/* You are going to use a larger buffer*/ 
float __attribute__ ((aligned (16))) buf[64] = 
{
-256, -183, -110, -37, 36, 109, 182, 255,
-250, -230, -190, -130, -50, 50, 170, 255,
10, 15, 99, 200, 70, 0, -100, -40,
20, 30, 200, 240, 88, 0, -100, -40,
200, 180, 150, 109, 17, 0, -100, -40,
171, 117, 73, -44, 0, 0, -133, -71,
100, 81, 60, -96, -33, 50, -150, -180,
0, 56, -35, -73, -14, -8, -15, -203,
};

float __attribute__ ((aligned (16))) bigbuf[640000];

float __attribute__ ((aligned (16))) dct_c[64];


void	print_block (float *p)
{
	int	i, j;

	for (i = 0; i < BLOCK_N; i ++) {
		for (j = 0; j < BLOCK_N; j ++) 
			printf("%12.5f, ", *p++);
		printf("\n");
	}
}

void 	dct_coeffients(float *p)
{
	int	k, n;

	double	t=M_PI/BLOCK_N;	// PI/N

	// for the first one, the scaling factor is sqrt(1/8) = 1 / (2*sqrt(1/2))
	for (k = 0; k < BLOCK_N; k ++) {
		*p ++ = 0.3535533905932737622;
	}

	// others have a scaling factor of sqrt(2/8) = 1/2 
	for (k = 1; k < BLOCK_N; k ++) {
		for (n = 0; n < BLOCK_N; n ++) 
			*p++ = cos(t * (n + 0.5) * k) / 2; 
	}
}


void	dct_1d(float *p)
{
	float	d[BLOCK_N];	 // Not a in-place transform. Need to copy the results back to p.
	float	*c;

	int	k, n;
	float	t;


	for (k = 0; k < BLOCK_N; k ++) {
		t = 0;
		c = dct_c + (k * BLOCK_N);
		for (n = 0; n < BLOCK_N; n ++) {
			t += c[n] * p[n];
		}
		d[k] = t;
	}

	for (k = 0; k < BLOCK_N; k ++)
		p[k] = d[k];
}


void    matrix_transpose (float *buf)
{
	int	i, j;
	for (i = 0; i < BLOCK_N; i ++) 
		for (j = i + 1; j < BLOCK_N; j ++) {
			float  tmp;

			tmp = buf[i * BLOCK_N + j];
			buf[i * BLOCK_N + j] = buf[j * BLOCK_N + i];
			buf[j * BLOCK_N + i] = tmp;
		}
}


void	dct_2d_c (float *buf) // only one block for demo purpose
{
	int	i;

	for (i = 0; i < BLOCK_N; i++) 
		dct_1d(buf + (i * BLOCK_N));  // for each row

	// matrix transpose
	matrix_transpose(buf);

	for (i = 0; i < BLOCK_N; i++) 
		dct_1d(buf + (i * BLOCK_N));  // for each row

	matrix_transpose(buf);

}


int	main()
{
	clock_t		tm;
	float *p;
	int i,j;
     
//	change 0 to 1 if you want to set buf to random values.
#if 0
	srand(100);
	for (i = 0; i < 64; i ++) {
		buf[i] = (float)rand()/RAND_MAX;
	}
#endif

	dct_coeffients(dct_c);
	puts("DCT coefficients:\n");
	print_block(dct_c);
	puts("\n");

	puts("Before DCT:\n");
	print_block(buf);
	puts("\n");
	
	for (i=0;i<10000;i++){
		for (j=0;j<64;j++){
			bigbuf[64*i+j]=buf[j];
		}	
	}

	tm=clock(); 
	// call your function 
      
    dct_2d_8x8_asm (bigbuf,10000);
//  for (i=0;i<10000;i++){
//	  p=bigbuf+64*i;
//	  dct_2d_c(p);
//  }
	tm = clock() - tm;

	puts("After DCT:\n");
	for(i=0;i<3;i++){
		p=bigbuf+64*i;
		print_block(p);
	}
	

	printf("time=%.1f\n", (double)tm/(CLOCKS_PER_SEC/1000.0));

	return 0;
}

