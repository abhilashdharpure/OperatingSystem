#pragma once
#ifdef __cplusplus
extern "C" {
#endif
int  builtin_client_start(void);  /* returns the fd to watch, or -1 on failure */
void builtin_client_pump(void);   /* call when that fd is readable */
extern "C" int builtin_client_get_error();
extern "C" void builtin_client_disconnect();
#ifdef __cplusplus
}
#endif