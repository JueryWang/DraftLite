#ifndef WG_AUTHORIZION_DEF_H
#define WG_AUTHORIZION_DEF_H
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllimport) char* GetAuthChipId();
    __declspec(dllimport) char* GetAuthPCUUID();
    __declspec(dllimport) char* GetAuthVeriCode();
    __declspec(dllimport) char* GetAuthTime();

#ifdef __cplusplus
}
#endif

#endif