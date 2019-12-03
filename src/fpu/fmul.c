#include <stdio.h>
#include "default.h"
 
long long int fmul(long long int i1,long long int i2){
   wire x1 = init(32,i1);
   wire x2 = init(32,i2);
   wire e1 = extract(x1,30,23);
   wire e2 = extract(x2,30,23);
   // mantissaの掛け算の計算
   wire mye = mul(concat(init(25,1),extract(x1,22,0)),concat(init(25,1),extract(x2,22,0)),0);

   wire ey = (extract(mye,47,47).val || bitand(extract(mye,46,22))) ? sub(add(e1,e2,2),init(10,126),0) : sub(add(e1,e2,2),init(10,127),0);
   wire my = (extract(mye,47,47).val) ? ((bitand(extract(mye,46,23))) ? init(23,0) : (extract(mye,23,23).val && extract(mye,24,24).val || bitor(extract(mye,22,0))) ? add(extract(mye,46,24),init(23,1),0) : extract(mye,46,24)) :
             (bitand(extract(mye,45,22)) ? init(23,0) : (extract(mye,22,22).val && extract(mye,23,23).val || bitor(extract(mye,21,0))) ? add(extract(mye,45,23),init(23,1),0) : extract(mye,45,23));

   // 符号を求める
   wire sy = init(1,extract(x1,31,31).val ^ extract(x2,31,31).val);
   wire y = (1-bitand(e1) && 1-bitand(e2) && 1-bitor(ey) && bitand(extract(mye,46,23))) ? concat(concat(sy,init(8,1)),init(23,0)) : // 繰り上がりで指数部が1，仮数部が0になるとき
            (1-bitand(e1) && 1-bitand(e2) && extract(ey,9,9).val || 1-bitor(ey)) ? concat(sy,init(31,0)) :
            (1-bitand(e1) && 1-bitand(e2) && extract(ey,8,8).val) ? concat(concat(sy,init(8,255)),init(23,0)) : concat(concat(sy,extract(ey,7,0)),my); // overflowしたら符号を合わせて無限にする

   return y.val;
}