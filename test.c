#include <stdio.h>

/**
 * lhsとrhsが同じ文字列だったら 0, そうでなければ1, error code 2
 */
int strcmp(char* lhs, char* rhs) {
  int i, flag;
  if (lhs[0] == '\0' && rhs[0] == '\0') {
    // 空文字列同士は等しいとする。
    return 0;
  }
  flag = 1;
  for (i = 0; ; ++i) {
    if (lhs[i] == '\0' || rhs[i] == '\0') {
      // 先にどちらかがnull文字に達した。
      if (lhs[i] != '\0' || rhs[i] != '\0') {
        return 1;
      }
      if (flag) {
        return 0;
      }
    }

    // 等しければ1, 等しくなければ0
    flag = lhs[i] == rhs[i];
  }
  return 2; // error code
}

int main() {
  char *c1 = "HELLO", *c2 = "HELLO";
  char *c3 = "NO", *c4 = "";

  printf("%d %d %d\n", strcmp(c1, c2), strcmp(c1, c3), strcmp(c2, c4));
}