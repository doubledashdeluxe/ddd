#ifdef __CWCC__
void operator delete(void *) {
    while (true) {}
}
#endif
