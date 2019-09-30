// the sum of even numbers between 0 and 20
int f(){
  int result = 0;
  for(int i = 0; i <= 20; i++){
    if (i % 2 == 0)
      result += i;
  }
  return result;
}
