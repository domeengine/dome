char* strToLower(char* str) {
  size_t length = strlen(str) + 1;
  char* result = malloc(length);
  for (size_t i = 0; i < length; i++) {
    result[i] = tolower(str[i]);
  }
  return result;
}
