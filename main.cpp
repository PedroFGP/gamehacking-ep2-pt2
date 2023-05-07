#include "pch.h"

DrawTransitionOriginalType DrawTransitionOriginal;

bool DataCompare(PBYTE pData, PBYTE bSig, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == 0;
}

DWORD_PTR FindPattern(DWORD_PTR dwAddress, DWORD dwSize, const char* pbSig, const char* szMask, long offset)
{
	size_t length = strlen(szMask);
	for (size_t i = NULL; i < dwSize - length; i++)
	{
		if (DataCompare((PBYTE)dwAddress + i, (PBYTE)pbSig, szMask))
			return dwAddress + i + offset;
	}
	return 0;
}

DWORD_PTR FindPointerPattern(DWORD_PTR dwAddress, DWORD dwSize, const char* pbSig, const char* szMask, long offset)
{
	auto address = FindPattern(dwAddress, dwSize, pbSig, szMask, 0);

	if (address == 0)
		return 0;

	auto ptr = *(DWORD32*)(address + offset);

	return ptr + address + offset + 4;
}

bool PatchMem(void* address, void* bytes, UINT64 size)
{
	DWORD oldProtection;
	if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtection)) { return false; }
	memcpy(address, bytes, size);
	return VirtualProtect(address, size, oldProtection, &oldProtection);
}

void HookDrawTransition();

void Initialize()
{
	DWORD_PTR baseAddress = (DWORD_PTR)GetModuleHandleA(nullptr);

	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)baseAddress, &moduleInfo, sizeof(moduleInfo));

	auto TNameEntryArrayAddress = FindPointerPattern
	(
		baseAddress,
		moduleInfo.SizeOfImage,
		"\x48\x83\xEC\x28\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x75\x00\xB9\x00\x00\x00\x00\x48\x89\x5C\x24\x20\xE8",
		"xxxxxxx????xxxx?x????xxxxxx",
		0x7
	);

	auto FChunkedFixedUObjectArrayAddress = FindPointerPattern
	(
		baseAddress,
		moduleInfo.SizeOfImage,
		"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x00\x00\x48\x8D\x00\x00",
		"xxx????xx??xx??",
		0x3
	);

	auto UEngineAddress = FindPointerPattern
	(
		baseAddress,
		moduleInfo.SizeOfImage,
		"\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x0F\x84\x00\x00\x00\x00\x48\x8B\x89",
		"xxx????xxxxx????xxx",
		0x3
	);

	UObject::GObjects = (FChunkedFixedUObjectArray*)(FChunkedFixedUObjectArrayAddress);
	FName::GNames = *(TNameEntryArray**)(TNameEntryArrayAddress);
	GEngine = *(UEngine**)(UEngineAddress);

	HookDrawTransition();
}

void HookDrawTransition()
{
	auto viewport = GEngine->GameViewport;
	if (!viewport) return;

	auto vtable = viewport->Vtable;
	if (!vtable) return;

	auto drawTransitionPtr = vtable + 0x63;
	DrawTransitionOriginal = reinterpret_cast<DrawTransitionOriginalType>(drawTransitionPtr[0]);

	auto hook = &DrawTransitionHook;
	PatchMem(drawTransitionPtr, &hook, 8);
}