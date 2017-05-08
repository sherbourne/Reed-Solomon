// ------------------------------------------------------------------
// File:    RScode.c
// Author:  Vladimir Gerasik
// Date:    January 2016
// 
// The code implements Reed-Solomon encoder and decoder. 
//
// 
// ------------------------------------------------------------------



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


#define PRINT_GF
#define PRINT_POLY
#define PRINT_SYNDR


int m, n, k, poly_m, init_zero=1;      // dim, codelength, message length, prim. polynomial decimal representation, generating poly parameter
int alog_tab[1024], log_tab[1024], g[1024];
int data[1024], b[1024], r[1024], r_clean[1024];
int i, j;

void generate_gf(void);

void gen_poly(void);

int dec_bin(int n);
void primpoly_print(int decimalNumber);
int primpoly(int dim);
void encoder_rs(void);
void decoder_rs(void);


int main()
{
	
	// input m, so that we generate GF(2^m), and primitive polynomial in dec, e.g.  poly_m=369 for m=8, poly_m=11 for m=3
	printf("REED_SOLOMON CODE RS(2^m-1,k) ENCODER AND DECODER. \n\nEnter parameters m (symbol length) and k (message length): \n");
	scanf("%d%d", &m, &k);
	n=(int)(pow(2,m) + 0.5)-1;
	
	
	poly_m = primpoly(m);
	
	
	printf("Reed-Solomon code RS(%d,%d) over the field GF(2^%d)  \n",n,k,m);
	printf("generated by the primitive polynomial: ");	primpoly_print(poly_m);
	printf("is capable of correcting of up to %d errors. \n\n",(n-k)/2);
	printf("The roots of the code are: ");
	for (int rt=init_zero; rt<n-k-1+init_zero; rt++){
		printf("a^%d, ", rt);
	}
	printf("a^%d. \n\n", n-k-1+init_zero);
	
	
	generate_gf();
	
	gen_poly();
	

    printf("\nRandomly generated message of length %d sent:\n",k);
    srand ( time(NULL) );
	for (i = 0; i < k; i++)   
	{
	data[i] = rand()%(n+1) ;
	
	// data[0]=2; data[1]=0; data[2]=3; data[3]=4;

	printf("%d ",data[i]);
    }
    
    
    
    
	printf("\n");
	
	encoder_rs();
	
	
	for (i = 0; i < k; i++)  {
	         r[i]=data[i];
	    
	}
	for (i=n-k-1; i>=0; i--) {
		    r[n-1-i]=b[i];
	    	
	}
	
	
    
	
	printf("\nEncoded codeword of length %d \n",n);
	 for (i = 0; i < n; i++)    {   r_clean[i]=r[i]; printf("%d ",r[i]);   }
	 
	 // introduce errors by hand
	 
	 /*  r[0]= 2;
	  r[1]= 0; 
	  r[2]= 3;
	  r[3]= 4;  
	 r[136]=(r[3]+4)%n; 
	  r[425]=(r[3]+2)%n;                                     
	  r[14]=(r[6]+2)%n;
	  r[20]=(r[14]+2)%n;
	  r[25]=(r[24]+2)%n;   
	  r[134]=(r[63]+2)%n;
	  r[240]=(r[134]+2)%n;
	  r[235]=(r[234]+2)%n; */
	  
	  // or randomly 
	            for(i=0;i<(n-k)/2;i++)
	                 r[rand()%n]=rand()%n;  
	  
	  
	  
	 	 
	 printf("\n\nEncoded codeword of length %d passed through the noisy channel\n",n);
	 for (i = 0; i < n; i++)    printf("%d ",r[i]);  


     decoder_rs();
	
     
}




int primpoly(int dim)
{     
                              //  Alternative primitive polynomials to choose from
      if (dim==3) return 11;  //  13
      if (dim==4) return 25;  //  19  
      if (dim==5) return 37;  //  41   47   55   59   61
      if (dim==6) return 67;  //  91   97   103  109  115  
      if (dim==7) return 137; //  131    143    145    157    167    171    185    191    193    203    211    213    229   239    241   247   253
      if (dim==8) return 285; //  see the notes
      if (dim==9) return 529;
      if (dim==10) return 1033;
      
}

void generate_gf()
{
	int lfsr = 0x01;
    unsigned msb, period = 0;
        
    do {
    	  alog_tab[period]=lfsr;
    	  log_tab[lfsr]=period;
    	  
     	  lfsr <<= 1;                     /* Shift register */
          msb = lfsr & (n+1);             /* Get msb (i.e., the output bit). */
          
		    if (msb == n+1)  lfsr ^= poly_m;               /* Only apply toggle mask if output bit is 1. */
             			  /* Apply toggle mask, value has 1 at bits corresponding to taps, 0 elsewhere. */
           ++period;      
		     
    } while(period != n);
     log_tab[0]=-1;
   
  #ifdef PRINT_GF  
  printf("Table of GF(2^%d)\n",m);
  printf("-------------------------------------\n");
    for (int j=0; j<=n; j++)
           printf("   \n%4d \t %0*d \t %3d \t %4d", j, m,dec_bin(alog_tab[j]), alog_tab[j],log_tab[j] );
           printf("\n-------------------------------------\n");
  #endif
}





void gen_poly()
// Compute the generator polynomial of the k-error correcting, length
// n=(2^m -1) Reed-Solomon code from the product of (x+alpha^i), for
// i = init_zero, init_zero + 1, ..., init_zero+length-k-1
{
register int i,j; 


   g[0] = alog_tab[init_zero];  //  <--- vector form of alpha^init_zero
   g[1] = 1;     // g(x) = (X+alpha^init_zero)
   for (i=2; i<=n-k; i++)
    { 
      g[i] = 1;
      for (j=i-1; j>0; j--)
        if (g[j] != 0)  
          g[j] = g[j-1]^alog_tab[(log_tab[g[j]]+i+init_zero-1)%n]; 
        else 
          g[j] = g[j-1]; 
      g[0] = alog_tab[(log_tab[g[0]]+i+init_zero-1)%n];
    }

  
 //  for (i=0; i<=length-k; i++)  g[i] = index_of[g[i]]; 

#ifdef PRINT_POLY
printf("Generator polynomial (independent term first):\ng(x) = ");
for (i=0; i<=n-k; i++) printf("a^%d ", log_tab[g[i]]);
printf("\n");
#endif
}




void encoder_rs()
// Compute the n-k parity symbols in b[0]..b[n-k-1]
// data[] is input and b[] is output in polynomial form.
// Encoding is done by using LFSR with connections
// specified by the elements of g[].

{
   register int i,j; 
   int feedback; 

   for (i=0; i<=n-k; i++)       g[i] = log_tab[g[i]]; 
   for (i=0; i<n-k; i++)        b[i] = 0; 
     
   for (i=0; i<k; i++)
    {
    feedback = log_tab[data[i]^b[n-k-1]]; 
      if (feedback != -1)
        { 
        for (j=n-k-1; j>0; j--)
          if (g[j] != -1)
            b[j] = b[j-1]^alog_tab[(g[j]+feedback)%n]; 
          else
            b[j] = b[j-1];
            b[0] = alog_tab[(g[0]+feedback)%n]; 
        }
       else
        { 
        for (j=n-k-1; j>0; j--)
          b[j] = b[j-1]; 
        b[0] = 0; 
        }
        
      /*  printf("\nRegistry content\n");
        for (int l=0; l<n-k; l++) printf("%4d", b[l]);  */
    }
    // printf("\n Finally \n");
}

void decoder_rs(){
 // int i, j;
  int s[1024]={}, ecp[1024]={}, ecp_new[1024]={}, corr[1024]={};   // syndromes, error correction polynomial, correction term
  int l, d, num_err;                                                        // registry length, discrepancy
  int err_loc[1024]={};                                          // store error locations
  int lam_sig[1024]={};                                          // product Lambda and sigma polynomial 
  int err_val[1024]={};                                            // error values storage
  
  int LSig=0, denom=0;
  // Calculate the syndromes using the roots of the code 
  
   
     
    #ifdef PRINT_SYNDR 
    printf("\n\nSyndromes are: \n");
    #endif
     for (i=init_zero; i<n-k+init_zero; i++)
       {
     	for (j=0; j<n; j++) { 
		  if (log_tab[r[j]] != -1) s[i-init_zero]^=alog_tab[(log_tab[r[j]] + i*(n-1-j))%n];
     	// printf("%d\n ", s[i-init_zero]);
		 }	
		 #ifdef PRINT_SYNDR 
		     	printf("S%d = %2d \t",i-init_zero, s[i-init_zero]);
     	 #endif
        }
        
        int SSS=0;
        for (i=init_zero; i<n-k+init_zero; i++) SSS ^=s[i-init_zero];
        printf("\nSUM of Syndromes %d \t", SSS);
        
        
        // Berlecamp-Massey algorithm 
        
        //initialize BMA
        i=0; ecp[0]=1; ecp_new[0]=1; corr[1]=1; l=0; 
        
        // printf("\n Correction poly initially %d, %d, %d, %d \n", corr[0],corr[1],corr[2],corr[3]);
        while (i<n-k){
        	++i; // printf("\n i = %d \n", i);
        	
        	
        // calculate discrepancy d	
        d=s[i-1];
        if (i>1) {
                  for (j=1;j<=l;j++) if (ecp[j]!=0 && s[i-1-j]!=0) d^=alog_tab[(log_tab[ecp[j]]+log_tab[s[i-1-j]])%n];
                 }
     
        
       // printf("\n calc d = %d\n",d);
        
        
        // update ecp
        
            if (d==0) {  for (j=n-k;j>0;j--) corr[j]=corr[j-1];    corr[0]=0;  }            // shift correction polynomial
          
          
            else  {   for (j=0;j<n-k;j++){                                     
			            if (corr[j]!=0)
                   	    ecp_new[j]^=alog_tab[(log_tab[corr[j]]+log_tab[d])%n];              // update ecp
					  }   
                   	    
                   	    if (2*l<i) {
                   	    	l=i-l;
                   	        for (j=0;j<n-k;j++){
                   	          corr[j]=0;
                   	          if (ecp[j]!=0) corr[j]=alog_tab[(log_tab[ecp[j]] - log_tab[d]+n)%n];    // update correction (divide ecp by d)
							}
                   	    }
                   	    
                   	    for (j=n-k;j>0;j--) corr[j]=corr[j-1];    corr[0]=0; // shift correction polynomial
                   	    
                   	    for (j=0;j<n-k;j++) ecp[j]=ecp_new[j];              // update ecp polynomial
                   	    
                 }
              
          	/*  printf("Correction poly  %d, %d, %d, %d \n", corr[0],corr[1],corr[2],corr[3]);
        	 printf("\n\n             ECP polynomial coefficients  \t");
		      for (j=0; j<=(n-k)/2; j++) printf("%d ", ecp[j]);   */
          }
          
             
   
   
   // CHIEN SEARCH (determines error locations by solving ECP)
   
              printf("\n\nERROR POSITIONS OBTAINED FROM CHIEN SEARCH. ACTUAL ERROR VALUES PROVIDED FOR TESTING PURPOSES:");
             for (i=0; i<n; i++){
             	
                 for (j=0; j<=(n-k)/2; j++)    if (ecp[j]!=0) err_loc[i]^=alog_tab[(log_tab[ecp[j]]+i*j)%n];
				   
 			          if (err_loc[i]==0) {
 			          	  num_err = n-1-abs(i-n)%n;     // this is error locations from the left, (n-i)%n is from the right
 			               printf("\nERROR DETERMINED AT POSITION %d: \t %3d instead of %d \t ERROR VALUE CHECK %d", num_err, r[num_err], 
						          r_clean[num_err], r[num_err]^r_clean[num_err]);
 			          }
				}
		
		
				
  //  FORNEY ALGORITHM (determines the error values)
  
         // 1. Create syndrome polynomial S(x)
         for (i=n-k; i>0; i--) s[i]=s[i-1];
         s[0]=1;
           
         
         
         // 2. Find the expression for the product of Lambda(x) and sigma(x) mod x^(2t+1)
         
         for (i=0; i<=n-k; i++)   {
                            // Multiply the current term of ECP polynomial
                            // with every term of syndrome polynomial S(x) using tables
            for (j=0; j<=(n-k)/2+1; j++)
                     if ((s[i]!=0) && (ecp[j]!=0))   lam_sig[i+j] ^= alog_tab[(log_tab[s[i]]+log_tab[ecp[j]])%n];
        }
        /* printf("\nLambda x sigma polynomial mod x^(2t - 1)  \n"); 
		for (j=0; j<=n-k; j++) printf("%d ", lam_sig[j]);	*/
        
        //________________________________________________________________________________ 
         
        // 3. ECP formal derivative calculation (shift odd terms one position to the left)
		
		 
         for (i=0; i<(n-k)/2; i++)   {
         ecp[2*i]=ecp[2*i+1];
         ecp[2*i+1]=0;
         }
         
      //  printf("\nECP prime  \n"); 
	 //	  for (j=0; j<=(n-k)/2; j++) printf("%d ", ecp[j]);		
				
		//_________________________________________________________________________________		
		
		printf("\n_____________________________________________________________________");   
		// 4. Get the error values
		printf("\n\nERROR POSITIONS FROM CHIEN SEARCH.");
		printf(" ERROR VALUES FROM FORNEY ALGORITHM:\n"); 
		for (i=0;i<n;i++) if (err_loc[i]==0) {
		
		printf("\nERROR POSITION %d",n-1-abs(i-n)%n); //       use (n-i)%n instead of n-1-abs(i-n))%n to get the position from the right
		                  // a. find (alpha^err)^(2-init_zero)
		                  
					        LSig=0; denom=0;
						  for (j=0;j<=n-k;j++){
						      if (lam_sig[j]!=0) LSig^=alog_tab[(log_tab[lam_sig[j]]+j*i)%n];    // evaluate lambda*sigma polynomial
							 // printf("\nLSIGMA %d",LSig);
							  if (ecp[j]!=0) denom^=alog_tab[(log_tab[ecp[j]]+j*i)%n];           // evaluate denominator 
						  }
						  
						 
						   
						  err_val[n-i]=alog_tab[((n-i)*(2-init_zero) + log_tab[LSig] + n - log_tab[denom])%n];  // final expression
						  
					printf("\t\t   ERROR VALUE %d",err_val[n-i]);
                                        
						  	
		}
		
		    
		
		
				
}

int dec_bin(int n) { /* Function to convert decimal to binary. Used to display GF table*/

    int rem, i=1, binary=0;
    while (n!=0)
    {
        rem=n%2;
        n/=2;
        binary+=rem*i;
        i*=10;
    }
    return binary;
}


  
   // this converts dec to binary and thus displays the coefficients of primitive polynomial 
   void primpoly_print(int decimalNumber){

    long int remainder,quotient;
    int binaryNumber[100],i=1,j;


    quotient = decimalNumber;


    while(quotient!=0){

         binaryNumber[i++]= quotient % 2;

         quotient = quotient / 2;

    }

    for(j = i-1; j> 0; j--)
         if (binaryNumber[j]!=0 && (j-1)!=0)
         printf("x^%d+",j-1);
          //binaryNumber[j]);
         printf("1\n");

}

 


