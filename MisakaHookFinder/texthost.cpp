#include "pch.h"
#include "host.h"
#include "hookcode.h"
#include "texthost.h"
#include "extension.h"
#include <fstream>

//const wchar_t* ALREADY_INJECTED = L"Textractor: already injected";
//const wchar_t* NEED_32_BIT = L"Textractor: architecture mismatch: only Textractor x86 can inject this process";
//const wchar_t* NEED_64_BIT = L"Textractor: architecture mismatch: only Textractor x64 can inject this process";
//const wchar_t* INVALID_CODEPAGE = L"Textractor: couldn't convert text (invalid codepage?)";
//const wchar_t* INJECT_FAILED = L"Textractor: couldn't inject";
//const wchar_t* INVALID_CODE = L"Textractor: invalid code";
//const wchar_t* INVALID_PROCESS = L"Textractor: invalid process";
//const wchar_t* INITIALIZED = L"Textractor: initialization completed";
//const wchar_t* CONSOLE = L"Console";
//const wchar_t* CLIPBOARD = L"Clipboard";

const wchar_t* ALREADY_INJECTED = L"Textractor: 已经注入";
const wchar_t* NEED_32_BIT = L"Textractor: 架构不匹配: 请尝试使用32位版本的Textractor";
const wchar_t* NEED_64_BIT = L"Textractor: 架构不匹配: 请尝试使用64位版本的Textractor";
const wchar_t* INVALID_CODEPAGE = L"Textractor: 无法转换文本 (无效的代码页?)";
const wchar_t* INJECT_FAILED = L"Textractor: 无法注入";
const wchar_t* INVALID_CODE = L"Textractor: 无效特殊码";
const wchar_t* INVALID_PROCESS = L"Textractor: 无效进程";
const wchar_t* INITIALIZED = L"Textractor: 初始化完成";
const wchar_t* CONSOLE = L"控制台";
const wchar_t* CLIPBOARD = L"剪贴板";

namespace TextHost
{
	DWORD TextHostInit( ProcessEvent connect,
		                                 ProcessEvent disconnect,
		                                 OnCreateThread create,
		                                 OnRemoveThread remove,
		                                 OutputText output
	                                    )
	{
		auto createThread = [create](TextThread& thread)
		{
			create(thread.handle, 
				thread.tp.processId, 
				thread.tp.addr, 
				thread.tp.ctx, 
				thread.tp.ctx2, 
				thread.name.c_str(), 
				HookCode::Generate(thread.hp, thread.tp.processId).c_str());
		};
		auto removeThread = [remove](TextThread& thread)
		{
			remove(thread.handle);
		};
		auto outputText = [output](TextThread& thread, std::wstring& text)
		{
			if (!text.empty()) 
			{
				Extension::RemoveRepeatChar(thread.handle, text);
				Extension::RemoveRepeatPhrase(thread.handle, text);
				output(thread.handle, text.c_str());
			}
			return false;
		};

		Host::Start(connect, disconnect, createThread, removeThread, outputText);
		Host::AddConsoleOutput(INITIALIZED);
		return 0;
	}

	DWORD InjectProcess(DWORD processId)
	{
		Host::InjectProcess(processId); 	
		return 0;
	}

	DWORD DetachProcess(DWORD processId)
	{
		try { Host::DetachProcess(processId); }
		catch (std::out_of_range)
		{ Host::AddConsoleOutput(INVALID_PROCESS); }
		return 0;
	}

	DWORD InsertHook(DWORD processId, LPCWSTR command)
	{
		if(auto hp = HookCode::Parse(command))
		try {Host::InsertHook(processId, hp.value());}catch(std::out_of_range){}
		else { Host::AddConsoleOutput(INVALID_CODE); }
		return 0;
	}

	DWORD RemoveHook(DWORD processId, uint64_t address)
	{
		try { Host::RemoveHook(processId, address); }
		catch (std::out_of_range) {}
		return 0;
	}

	DWORD SearchForText(DWORD processId, LPCWSTR text, int codepage)
	{
		SearchParam sp = {};
		wcsncpy_s(sp.text, text, PATTERN_SIZE - 1);
		sp.codepage = codepage;
		try { Host::FindHooks(processId, sp); }
		catch (std::exception) {}
		return 0;
	}

	VOID SearchForHooks(DWORD processId, SearchParam* sp, FindHooks findHooks)
	{
		auto hooks = std::make_shared<std::vector<std::wstring>>();
		auto timeout = GetTickCount64() + sp->searchTime + 100'00;

		try
		{
			Host::FindHooks(processId, *sp,
				[hooks](HookParam hp, std::wstring text)
				{
					hooks->push_back(HookCode::Generate(hp) + L" => " + text);
				});
		}
		catch (std::out_of_range) { return; }

		std::thread([hooks,timeout,findHooks]
			{
				for (int lastSize = 0; hooks->size() == 0 || hooks->size() != lastSize; Sleep(2000))
				{
						lastSize = hooks->size();
						if (GetTickCount64() > timeout) break; //如果没有找到结果，size始终为0，不能跳出循环，所以设定超时时间
				}
				static std::string location = std::filesystem::current_path().string() + "\\";
				std::ofstream saveFile(location + "result.txt");
				if (saveFile.is_open()) 
				{
					for (std::vector<std::wstring>::const_iterator it = hooks->begin(); it != hooks->end(); ++it)
					{
						saveFile << WideStringToString(*it) << std::endl; //utf-8
						saveFile << "<=====>" << std::endl; //utf-8
					}
					saveFile.close();
					if (hooks->size() != 0) {
						findHooks();
					}
				}
				hooks->clear();
			}).detach();
	}

	DWORD AddClipboardThread(HWND handle)
	{
		if (AddClipboardFormatListener(handle) == TRUE)
			Host::AddClipboardThread(GetWindowThreadProcessId(handle, NULL));
		return 0;
	}
}

