/**
 * lhsとrhsが同じ文字列だったら 0, そうでなければ1, error code 2
 */
int strcmp(char* lhs, char* rhs) {
  int i;
  // 空文字列同士は等しいとする。
  if (lhs[0] == '\0' && rhs[0] == '\0') { return 0; }
  for (i = 0;; ++i) {
    if (lhs[i] == '\0' || rhs[i] == '\0') { return !(lhs[i] == rhs[i]); }
    if (lhs[i] != rhs[i]) { return 1; }
  }
  return 2; // error code
}

/**
 * lhsとrhsが同じ文字列だったら 0, そうでなければ1, error code 2
 */
int strncmp(char* lhs, char* rhs, unsigned int len) {
  int i;
  for (i = 0; i < len; ++i) {
    if (lhs[i] != rhs[i]) { return 1; }
  }
  return 0;
}