#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "default.h"
#include "fmul.h"

int main(){
   // long long int i1,i2;
   // scanf("%lld",&i1);
   // scanf("%lld",&i2);
   // long long int y = fmul(i1,i2);
   // print_wire(init(32,i1),"\n");
   // print_wire(init(32,i2),"\n");
   // print_wire(init(32,y),"\n");
   int i,j,s1,s2,it,jt;
   wire m1,m2;
   for (i=1; i<255; i++) {
      for (j=1; j<255; j++) {
         for (s1=0; s1<2; s1++) {
            for (s2=0; s2<2; s2++) {
               for (it=0; it<10; it++) {
                  for (jt=0; jt<10; jt++) {
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
                     switch(jt) {
                        case 0:
                           m2 = init(23,0);
                           break;
                        case 1:
                           m2 = init(23,1);
                           break;
                        case 2:
                           m2 = init(23,2);
                           break;
                        case 3:
                           m2 = concat(init(4,7),init(19,0));
                           break;
                        case 4:
                           m2 = concat(init(1,1),init(22,0));
                           break;
                        case 5:
                           m2 = bitnot(concat(init(2,1),init(21,0)));
                           break;
                        case 6:
                           m2 = bitnot(init(23,0));
                           break;
                        default:
                           if(i==256){
                              m2 = init(23,0);
                           }else{
                              m2 = init(23,rand() >> 9);
                           }
                     }
                     wire ax = concat3(init(1,s1),init(8,i),m1);
                     wire bx = concat3(init(1,s2),init(8,j),m2);
                     wire y = init(32,fmul(ax.val,bx.val));
                     double a = bitstoreal(ax);
                     double b = bitstoreal(bx);
                     double fpu = bitstoreal(y);
                     double ans = a*b;
                     if(fpu-ans>=max(fabs(ans)*pow(2,-22),pow(2,-126)) && fabs(ans)<pow(2,128)){
                        print_wire(ax,"\n");
                        print_wire(bx,"\n");
                        print_wire(y,"\n");
                        printf("fpu %le\n",fpu);
                        printf("%le * %le = %le\n",a,b,ans);
                     }
                  }
               }
            }
         }
      }
   }
   return 0;
}