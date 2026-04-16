#pragma once
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>
#include <Psapi.h>
#include <string>
#include <memory>
#include <stdlib.h>
#pragma comment(lib, "Version.lib")

class Memory
{
private:
	bool SetDebugPrivilege(BOOL bEnablePrivilege)
	{
		HANDLE hProc = NULL;
		HANDLE hToken = NULL;
		LUID luid;
		TOKEN_PRIVILEGES tp;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
			if (LookupPrivilegeValue(NULL, "SeDebugPrivilege", &luid))
			{
				tp.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : SE_PRIVILEGE_REMOVED;
				tp.Privileges[0].Luid = luid;
				tp.PrivilegeCount = 1;
				AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

				return true;
			}
		}
		return false;
	}
public:
	uint32_t proc_id = 0;
	HANDLE proc_handle = INVALID_HANDLE_VALUE;
public:
	uintptr_t module_base = 0, module_size = 0;

	Memory()
	{
		SetDebugPrivilege(TRUE);
		const auto snapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD, 0) };
		if (INVALID_HANDLE_VALUE == snapshot) {
			MessageBoxA(0, "Failed to create snapshot", 0, 0);
			exit(0);
		}

		bool first = true;

		PROCESSENTRY32 process_entry{ sizeof PROCESSENTRY32 };
		auto process{ Process32First(snapshot, &process_entry) };
		for (; process; process = Process32Next(snapshot, &process_entry)) {
			if (_stricmp(process_entry.szExeFile, "TslGame.exe") == 0) {
				if (first) {
					first = false;
					continue;
				}
				else
					break;
			}
		}

		CloseHandle(snapshot);

		if (!process) {
			MessageBoxA(0, "Run TslGame", 0, 0);
			exit(0);
		}

		proc_id = process_entry.th32ProcessID;

		if (!(proc_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_entry.th32ProcessID))) {
			MessageBoxA(0, "Run TslGame", 0, 0);
			exit(0);
		}

		DWORD needed{ 0 };
		if (K32EnumProcessModulesEx(proc_handle, (HMODULE*)&module_base, sizeof(module_base), &needed, LIST_MODULES_64BIT))
		{
			MODULEINFO mi;
			GetModuleInformation(proc_handle, (HMODULE)module_base, &mi, sizeof(mi));
			module_size = mi.SizeOfImage;
		}
	}

	std::string GetProcessFileVersion(HANDLE hProcess) {
		char exePath[MAX_PATH];
		DWORD pathSize = MAX_PATH;

		if (!QueryFullProcessImageNameA(hProcess, 0, exePath, &pathSize)) {
			return "Unknown_Path";
		}

		DWORD dummy;
		DWORD verSize = GetFileVersionInfoSizeA(exePath, &dummy);
		if (verSize == 0) return "Unknown_Size";

		std::vector<BYTE> verData(verSize);
		if (!GetFileVersionInfoA(exePath, 0, verSize, verData.data())) {
			return "Unknown_Info";
		}

		VS_FIXEDFILEINFO* pFileInfo = nullptr;
		UINT fileInfoLen = 0;
		if (VerQueryValueA(verData.data(), "\\", (LPVOID*)&pFileInfo, &fileInfoLen)) {
			int major = HIWORD(pFileInfo->dwFileVersionMS);
			int minor = LOWORD(pFileInfo->dwFileVersionMS);
			int build = HIWORD(pFileInfo->dwFileVersionLS);
			int revision = LOWORD(pFileInfo->dwFileVersionLS);

			char versionStr[256];
			sprintf_s(versionStr, "%d.%d.%d.%d", major, minor, build, revision);
			return std::string(versionStr);
		}

		return "Unknown_Version";
	}

	__forceinline bool read(uintptr_t va, void* out_buffer, uint64_t size)
	{
		SIZE_T read;
		return ReadProcessMemory(proc_handle, (LPCVOID)va, out_buffer, size, &read) && read == size;
	}

	template <typename T>
	__forceinline T read(uintptr_t va)
	{
		T t{};
		ReadProcessMemory(proc_handle, (LPCVOID)va, &t, sizeof(t), nullptr);
		return t;
	};
	__forceinline uintptr_t calc_relative(uintptr_t current, int32_t relative)
	{
		return current + read<int32_t>(current) + relative;
	}
	__forceinline std::vector<uintptr_t> aob_scan(const uintptr_t start, const uintptr_t end, const std::basic_string_view<char> pattern, const std::basic_string_view<char> mask)
	{
		std::vector<uintptr_t> result;
		uint8_t chunk[0x1000];
		for (auto p = start; p < end; p += 0x1000)
		{
			read(p, chunk, 0x1000);
			int n = 0;
			for (auto c = 0u; c < 0x1000; c++)
			{
				(mask[n] != '?') && (chunk[c] == (uint8_t)pattern[n]) ? n++ : ((mask[n] == '?') ? n++ : n = 0);
				if (n >= mask.size())
					result.push_back(p + c - n + 1);
			}
		}
		return result;
	}

	__forceinline int FormatPattern(const char* src, unsigned char* out)
	{
		while (*src == ' ') src++;
		int cur = 0;
		bool h = false;
		bool l = false;
		while (*src)
		{
			const char c = *(src++);
			int ic = 0;
			if (c >= '0' && c <= '9')
			{
				if (!h)
				{
					ic = c - '0';
					out[cur] = ic * 16;
					h = true;
				}
				else if (!l)
				{
					ic = c - '0';
					out[cur] += ic;
					h = false;
					l = false;
				}

			}
			if (c >= 'a' && c <= 'f')
			{
				if (!h)
				{
					ic = c - 'a' + 10;
					out[cur] = ic * 16;
					h = true;
				}
				else if (!l)
				{
					ic = c - 'a' + 10;
					out[cur] += ic;
					h = false;
					l = false;
				}
			}
			if (c >= 'A' && c <= 'Z')
			{
				if (!h)
				{
					ic = c - 'A' + 10;
					out[cur] = ic * 16;
					h = true;
				}
				else if (!l)
				{
					ic = c - 'A' + 10;
					out[cur] += ic;
					h = false;
					l = false;
				}
			}
			if (c == ' ')
			{
				if (h && !l)
				{
					out[cur] = out[cur] / 16;
				}
				h = false;
				l = false;
			}
			if (c == '?')
			{
				if (!h)
				{
					out[cur] = 0;
					h = true;
				}
				else if (!l)
				{
					out[cur] += 0;
					h = false;
					l = false;
				}
			}
			if (*src && !h && !l && *src != ' ')
			{
				cur++;
			}
		}
		return cur;
	}

	__forceinline long long FindPattern(unsigned char* data, int searchLength, const char* signature, unsigned long long* result)
	{
		int ret = 0;
		unsigned char* sig = 0;
		sig = (unsigned char*)malloc(1024);
		if (!sig) throw "variable initialization memory failed in function [FindPattern]";
		int signatureLength = FormatPattern(signature, sig) + 1;
		if (!signatureLength) {
			free(sig);
			sig = nullptr;
			return ret;
		}
		unsigned char* headerCursor = data;
		unsigned char* buffer = data;
		unsigned char* max = buffer + searchLength - signatureLength;
		unsigned char first = sig[0];
		int last = signatureLength - 1;
		for (; buffer < max; ++buffer)
		{
			if (*buffer != first) continue;
			for (int i = 0; i < signatureLength; i++)
			{
				if (i == last)
				{
					if (sig[last] == '\x00' || sig[last] == buffer[i])
					{
						if (buffer - headerCursor < 0x0fffffff)
						{
							result[ret++] = buffer - headerCursor;
							if (ret > 0x1FFFF)
							{
								free(sig);
								sig = 0;
								return ret;
							}
						}


					}
				}
				if (sig[i] == '\x00') continue;
				if (buffer[i] != sig[i]) break;
			}
		}

		free(sig);
		sig = nullptr;
		return ret;
	}

	__forceinline unsigned long long FindPattern(unsigned char* data, int searchLength, const char* signature)
	{
		return FindPattern(data, searchLength, signature, 1);
	}

	__forceinline unsigned long long FindPattern(unsigned char* data, int searchLength, const char* signature, int findcount)
	{
		int iFoundCount = 1;
		unsigned long long ret = 0;
		unsigned char* sig = 0;
		sig = (unsigned char*)malloc(1024);
		if (!sig) throw "variable initialization memory failed in function [FindPattern]";
		int signatureLength = FormatPattern(signature, sig) + 1;
		if (!signatureLength) {
			free(sig);
			sig = 0;
			return ret;
		}
		unsigned char* headerCursor = data;
		unsigned char* buffer = data;
		unsigned char* max = buffer + searchLength - signatureLength;
		unsigned char first = sig[0];
		int last = signatureLength - 1;
		for (; buffer < max; ++buffer)
		{
			if (*buffer != first) continue;
			for (int i = 0; i < signatureLength; i++)
			{
				if (i == last)
				{
					if (sig[last] == '\x00' || sig[last] == buffer[i])
					{
						if (buffer - headerCursor < 0x0fffffff && iFoundCount++ == findcount)
						{
							ret = buffer - headerCursor;
							free(sig);
							sig = 0;
							return ret;
						}

					}
				}
				if (sig[i] == '\x00') continue;
				if (buffer[i] != sig[i]) break;
			}
		}

		free(sig);
		sig = 0;
		return ret;
	}
	__forceinline unsigned long long FindLastPattern(unsigned char* data, int searchLength, const char* signature)
	{
		unsigned long long lastFoundOffset = 0;
		unsigned char* sig = (unsigned char*)malloc(1024);
		if (!sig) return 0;

		int signatureLength = FormatPattern(signature, sig) + 1;
		if (signatureLength <= 0) {
			free(sig);
			return 0;
		}

		unsigned char* headerCursor = data;
		unsigned char* buffer = data;
		unsigned char* max = buffer + searchLength - signatureLength;
		unsigned char first = sig[0];
		int last = signatureLength - 1;

		for (; buffer < max; ++buffer)
		{
			if (*buffer != first) continue;

			bool match = true;
			for (int i = 0; i < signatureLength; i++)
			{
				if (sig[i] == '\x00') continue;
				if (buffer[i] != sig[i]) {
					match = false;
					break;
				}
			}

			if (match) {
				lastFoundOffset = (unsigned long long)(buffer - headerCursor);
			}
		}

		free(sig);
		return lastFoundOffset;
	}
};

inline std::unique_ptr<Memory>memory;