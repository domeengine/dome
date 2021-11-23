internal void*
IO_API_readFile(DOME_Context ctx, const char* path, size_t* lengthPtr) {
  ENGINE* engine = (ENGINE*)ctx;
  return ENGINE_readFile(engine, path, lengthPtr);
}


IO_API_v0 io_v0 = {
  .readFile = IO_API_readFile,
};
