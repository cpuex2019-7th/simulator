#include <stdio.h>
#include "default.h"
#include <math.h>
// typedef struct {
//     long long int val;
//     int len;
// } wire;

void dec2bin(wire w,int* binary){
    int i;
    long long int decimal = w.val;
    for(i=0;i<64;i++){
        binary[i] = 0;
    }
    for(i=0;decimal>0;i++){
        binary[i] = decimal % 2;
        decimal = decimal / 2;
    }
}

wire init(int l,long long int v) {
    wire w;
    w.len = l;
    w.val = v;
    return w;
}

wire concat(wire a, wire b){
    a.val = a.val << b.len;
    return init(a.len+b.len,a.val+b.val);
}

wire concat3(wire a, wire b, wire c){
    return concat(concat(a,b),c);
}

wire extract(wire w, int s, int t){
    int binary[64];
    dec2bin(w,binary);
    w.val = binary[t];
    int p = 1;
    for(int i=t+1;i<=s;i++){
        p*=2;
        w.val += p*binary[i];
    }
    w.len = s-t+1;
    return w;
}

void print_wire(wire w,char* s){
    // printf("%3d ", w.len);
    int binary[64];
    dec2bin(w,binary);
    if(w.len==32){
        for(int i=w.len-1;i>=0;i--){
            if(i==w.len-2 || i == w.len-10) printf(" ");
            printf("%d", binary[i]);
        }
    }else{
        for(int i=w.len-1;i>=0;i--){
            printf("%d", binary[i]);
        }
    }
    printf("%s",s);
}

int check_len(wire a,wire b){
    if(a.len != b.len){
        printf("len of ");
        int binary[64];
        dec2bin(a,binary);
        for(int i=a.len-1;i>=0;i--){
            printf("%d", binary[i]);
        }
        printf(" and ");
        dec2bin(b,binary);
        for(int i=b.len-1;i>=0;i--){
            printf("%d", binary[i]);
        }
        printf(" is different.\n");
        return 0;
    }else{
        return 1;
    }
}

int eq(wire a,wire b){
    if(check_len(a,b)){
        if(a.val == b.val){
            return 1;
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}
int gt(wire a,wire b){
    if(check_len(a,b)){
        if(a.val > b.val){
            return 1;
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}
int ge(wire a,wire b){
    if(check_len(a,b)){
        if(a.val >= b.val){
            return 1;
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}
int lt(wire a,wire b){
    if(check_len(a,b)){
        if(a.val < b.val){
            return 1;
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}
int le(wire a,wire b){
    if(check_len(a,b)){
        if(a.val <= b.val){
            return 1;
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}

wire bitnot(wire w){
    int binary[64];
    dec2bin(w,binary);
    for(int i=0;i<w.len;i++){
        binary[i] = 1 - binary[i];
    }
    w.val = binary[0];
    int p = 1;
    for(int i=1;i<w.len;i++){
        p*=2;
        w.val += p*binary[i];
    }
    return w;
}

int bitor(wire w){
    int binary[64];
    dec2bin(w,binary);
    for(int i=0;i<w.len;i++){
        if(binary[i]) return 1;
    }
    return 0;
}

int bitand(wire w){
    int binary[64];
    dec2bin(w,binary);
    for(int i=0;i<w.len;i++){
        if(~binary[i]) return 0;
    }
    return 1;
}

wire add(wire a, wire b, int l){
    check_len(a,b);
    return init(a.len+l,a.val+b.val);
}

wire sub(wire a, wire b, int l){
    check_len(a,b);
    return init(a.len+l,a.val-b.val);
}

wire mul(wire a, wire b, int l){
    check_len(a,b);
    return init(a.len+l,a.val*b.val);
}

double bitstoreal(wire w){
    int s = extract(w,31,31).val;
    int e = extract(w,30,23).val;
    int m = extract(w,22,0).val;
    // printf("%d %d %d\n",s,e,m);
    double mm = 0;
    for(int i=-23;i<=-1;i++){
        mm += (m%2)*pow(2,i);
        m /= 2;
    }
    return pow(-1,s)*pow(2,e-127)*(1+mm);
}

double max(double a,double b){
    if(a>b){
        return a;
    }else{
        return b;
    }
}

double max3(double a,double b,double c){
    return max(max(a,b),c);
}

// int main(){
//     // wire a = init(7,5);
//     // wire b = init(7,4);
//     // wire c = concat(a,b);
//     // print_wire(c);
//     // wire d = extract(c,8,2);
//     // print_wire(d);
//     printf("%le",bitstoreal(concat3(init(1,1),init(8,164),init(23,3670016))));
//     return 0;
// }