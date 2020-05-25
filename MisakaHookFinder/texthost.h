#pragma once

#include "pch.h"
#include "types.h"

typedef void (FindHooks)();
typedef void (ProcessEvent)(DWORD processId);
typedef void (OnCreateThread)(int64_t thread_id, DWORD processId, uint64_t addr, uint64_t context, uint64_t subcontext, LPCWSTR name, LPCWSTR hookcode);
typedef void (OnRemoveThread)(int64_t thread_id);
typedef void (OutputText)(int64_t thread_id, LPCWSTR output);

namespace TextHost
{
	DWORD  TextHostInit(ProcessEvent connect, ProcessEvent disconnect, OnCreateThread create, OnRemoveThread remove, OutputText output);
	DWORD  InjectProcess(DWORD processId);
	DWORD  DetachProcess(DWORD processId);
	DWORD  InsertHook(DWORD processId, LPCWSTR command);
	DWORD  RemoveHook(DWORD processId, uint64_t address);
	DWORD  SearchForText(DWORD processId, LPCWSTR text, int codepage);
	VOID   SearchForHooks(DWORD processId, SearchParam* sp, FindHooks findhooks);
	DWORD  AddClipboardThread(HWND handle);
}