#include <stdio.h>

int check_user(char *user, char *fname);

int main(int argc, char *argv[])
{
  int result;

  printf("fail %d\n", check_user("usera", "./bogus"));
  printf("no %d\n", check_user("userb", "./testuser.lst"));
  printf("suc %d\n", check_user("usera", "./testuser.lst"));
}
