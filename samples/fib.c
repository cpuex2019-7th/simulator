int f(){
  // calc fib(nth)
  int nth = 30;
  
  int b0, b1 = 1;
  int result = 1; // first this value is fib(2)
  
  int old_b1, old_result;
  for(int i = 0; i<nth-2; i++){
    old_b1 = b1;
    old_result = result;
    
    b0 = b1;
    b1 = result;
    result = old_b1 + old_result;    
  }
  return result;
}
