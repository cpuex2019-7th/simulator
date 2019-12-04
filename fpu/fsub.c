#include <stdio.h>
#include "default.h"
#include "fadd.h"
 
long long int fsub(long long int i1,long long int i2){
   wire x2 = init(32,i2);
   return fadd(i1,concat(bitnot(extract(x2,31,31)),extract(x2,30,0)).val);
}