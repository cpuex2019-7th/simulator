#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "default.h"
#include "fsqrt.h"

int main(){
   // long long int i1,i2;
   // scanf("%lld",&i1);
   // scanf("%lld",&i2);
   // long long int y = fsqrt(i1,i2);
   // print_wire(init(32,i1),"\n");
   // print_wire(init(32,i2),"\n");
   // print_wire(init(32,y),"\n");
   int i,s1,it;
   wire m1;
   for (i=1; i<255; i++) {
      for (s1=0; s1<2; s1++) {
         for (it=0; it<10; it++) {
            switch(it) {
               case 0:
                  m1 = init(23,0);
                  break;
               case 1:
                  m1 = init(23,1);
                  break;
               case 2:
                  m1 = init(23,2);
                  break;
               case 3:
                  m1 = concat(init(4,7),init(19,0));
                  break;
               case 4:
                  m1 = concat(init(1,1),init(22,0));
                  break;
               case 5:
                  m1 = bitnot(concat(init(2,1),init(21,0)));
                  break;
               case 6:
                  m1 = bitnot(init(23,0));
                  break;
               default:
                  if(i==256){
                     m1 = init(23,0);
                  }else{
                     m1 = init(23,rand() >> 9);
                  }
            }
                     
            wire ax = concat3(init(1,s1),init(8,i),m1);
            wire y = init(32,fsqrt(ax.val));
            double a = bitstoreal(ax);
            double fpu = bitstoreal(y);
            double ans = sqrt(a);
            if(fpu-ans>=max(fabs(ans)*pow(2,-20),pow(2,-126)) && fabs(ans)<pow(2,128)){
               print_wire(ax,"\n");
               print_wire(y,"\n");
               printf("fpu %le\n",fpu);
               printf("sqrt(%le) = %le\n",a,ans);
            }
         }
      }
   }
   return 0;
}