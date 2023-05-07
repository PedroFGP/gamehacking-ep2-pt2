#pragma once
#include "pch.h"

class FChunkedFixedUObjectArray;
class UClass;

template<class T>
class TArray
{
	friend class FString;

public:
	constexpr TArray() noexcept
	{
		Data = nullptr;
		Count = Max = 0;
	};

	[[nodiscard]] constexpr auto Num() const noexcept
	{
		return Count;
	};

	[[nodiscard]] constexpr auto& operator[](INT32 i) noexcept
	{
		return Data[i];
	};

	[[nodiscard]] constexpr const auto& operator[](INT32 i) const noexcept
	{
		return Data[i];
	};

	[[nodiscard]] constexpr auto IsValidIndex(INT32 i) const noexcept
	{
		return i < Num();
	}

private:
	T* Data;
	INT32 Count;
	INT32 Max;
};

class FString : public TArray<wchar_t>
{
public:
	constexpr FString() noexcept
	{
	};

	constexpr FString(const wchar_t* other) noexcept
	{
		Max = Count = *other ? static_cast<INT32>(std::wcslen(other)) + 1 : 0;

		if (Count)
			Data = const_cast<wchar_t*>(other);
	};

	[[nodiscard]] constexpr auto IsValid() const noexcept
	{
		return Data != nullptr;
	}

	[[nodiscard]] constexpr auto c_str() const noexcept
	{
		return Data;
	}

	[[nodiscard]] auto ToString() const noexcept
	{
		const auto length = std::wcslen(Data);
		std::string str(length, '\0');
		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);
		return str;
	}
};


class FNameEntry
{
public:
	[[nodiscard]] constexpr auto GetAnsiName() const noexcept
	{
		return AnsiName;
	}

	[[nodiscard]] constexpr auto GetWideName() const noexcept
	{
		return WideName;
	}

private:
	INT32 Index; //0x0000
	char pad_0004[4]; //0x0004
	class FNameEntry* HashNext; //0x0008

	union //0x0010
	{
		char AnsiName[1024];
		wchar_t WideName[1024];
	};
};

class TNameEntryArray
{
	enum
	{
		ElementsPerChunk = 16 * 1024,
		ChunkTableSize = (2 * 1024 * 1024 + ElementsPerChunk - 1) / ElementsPerChunk
	};

	[[nodiscard]] constexpr auto GetItemPtr(std::size_t Index) const noexcept
	{
		const auto ChunkIndex = Index / ElementsPerChunk;
		const auto WithinChunkIndex = Index % ElementsPerChunk;
		return Chunks[ChunkIndex] + WithinChunkIndex;
	}

public:
	[[nodiscard]] constexpr auto Num() const noexcept
	{
		return NumElements;

	}

	[[nodiscard]] constexpr auto IsValidIndex(INT32 Index) const noexcept
	{
		return Index < Num() && Index >= 0;
	}

	[[nodiscard]] constexpr auto& operator[](INT32 Index) const noexcept
	{
		return *GetItemPtr(Index);
	}

private:
	FNameEntry* const* Chunks[ChunkTableSize];
	INT32 NumElements;
	INT32 NumChunks;
};

class FName
{
public:
	static inline TNameEntryArray* GNames = nullptr;

	[[nodiscard]] static constexpr auto& GetGlobalNames() noexcept
	{
		return *GNames;
	};

	[[nodiscard]] constexpr auto GetName() const noexcept
	{
		return GetGlobalNames()[ComparisonIndex]->GetAnsiName();
	};

	[[nodiscard]] constexpr auto operator==(const FName& other) const noexcept
	{
		return ComparisonIndex == other.ComparisonIndex;
	};

	INT32 ComparisonIndex;
	INT32 Number;

	constexpr FName() noexcept :
		ComparisonIndex(0),
		Number(0)
	{
	};

	constexpr FName(INT32 i) noexcept :
		ComparisonIndex(i),
		Number(0)
	{
	};

	FName(const char* nameToFind) noexcept :
		ComparisonIndex(0),
		Number(0)
	{
		static std::unordered_set<INT32> cache;

		for (auto i : cache) {
			if (!std::strcmp(GetGlobalNames()[i]->GetAnsiName(), nameToFind)) {
				ComparisonIndex = i;
				return;
			}
		}

		for (decltype(GetGlobalNames().Num()) i = 0, max = GetGlobalNames().Num(); i < max; ++i) {
			if (GetGlobalNames()[i] != nullptr) {
				if (!std::strcmp(GetGlobalNames()[i]->GetAnsiName(), nameToFind)) {
					cache.insert(i);
					ComparisonIndex = i;
					return;
				}
			}
		}
	};
};

// Class CoreUObject.Object
// 0x0028
class UObject
{
public:
	static inline FChunkedFixedUObjectArray* GObjects = nullptr;	// 0x0000(0x0000)
	void** Vtable;                                                   // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	INT32 ObjectFlags;                                              // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	INT32 InternalIndex;                                            // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	class UClass* Class;                                                    // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	FName Name;                                                     // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	class UObject* Outer;                                                    // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY

	[[nodiscard]] static constexpr auto& GetGlobalObjects() noexcept
	{
		return GObjects;
	}

	template <typename T>
	[[nodiscard]] static constexpr auto FindObject(std::string_view name) noexcept -> T*
	{
		for (decltype(GetGlobalObjects()->Num()) i = 0, max = GetGlobalObjects()->Num(); i < max; ++i) {

			auto object = GetGlobalObjects()->GetByIndex(i).Object;
			if (object == nullptr)
				continue;

			if (object->GetFullName() == name)
				return static_cast<T*>(object);
		}
		return nullptr;
	}

	template <typename T>
	[[nodiscard]] static constexpr auto FindObjects(std::string_view name) noexcept -> std::vector<T*>
	{
		std::vector<T*> objects;
		for (decltype(GetGlobalObjects()->Num()) i = 0, max = GetGlobalObjects()->Num(); i < max; ++i) {

			auto object = GetGlobalObjects()->GetByIndex(i).Object;
			if (object == nullptr)
				continue;

			if (object->GetFullName() == name)
				objects.emplace_back(static_cast<T*>(object));
		}
		return objects;
	}

	template <typename T>
	[[nodiscard]] static constexpr auto FindObject() noexcept -> T*
	{
		auto StaticClass = T::StaticClass();
		for (decltype(GetGlobalObjects()->Num()) i = 0, max = GetGlobalObjects()->Num(); i < max; ++i) {

			auto object = GetGlobalObjects()->GetByIndex(i).Object;
			if (object == nullptr)
				continue;

			if (object->IsA(StaticClass))
				return static_cast<T*>(object);
		}
		return nullptr;
	}

	template <typename T>
	[[nodiscard]] static constexpr auto FindObjects() noexcept -> std::vector<T*>
	{
		std::vector<T*> objects;
		auto StaticClass = T::StaticClass();
		for (decltype(GetGlobalObjects()->Num()) i = 0, max = GetGlobalObjects()->Num(); i < max; ++i) {

			auto object = GetGlobalObjects()->GetByIndex(i).Object;
			if (object == nullptr)
				continue;

			if (object->IsA(StaticClass))
				objects.emplace_back(static_cast<T*>(object));
		}
		return objects;
	}

	[[nodiscard]] static auto FindClass(std::string_view name) noexcept
	{
		return FindObject<UClass>(name);
	}

	template <typename T>
	[[nodiscard]] static auto GetObjectCasted(std::size_t index) noexcept
	{
		return static_cast<T*>(GetGlobalObjects()->GetByIndex(index).Object);
	}

	[[nodiscard]] auto GetName() const noexcept -> std::string;

	[[nodiscard]] auto GetFullName() const noexcept -> std::string;

	auto IsA(UClass* cmp) const noexcept -> bool;

	template <typename T>
	[[nodiscard]] constexpr auto IsA() const noexcept
	{
		return IsA(T::StaticClass());
	}

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class CoreUObject.Object");
		return ptr;
	}

	void ExecuteUbergraph(int EntryPoint);
};

// Class CoreUObject.Field
// 0x0008 (0x0030 - 0x0028)
class UField : public UObject
{
public:
	class UField* Next;                                                     // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class CoreUObject.Field");
		return ptr;
	}

};


// Class CoreUObject.Struct
// 0x0058 (0x0088 - 0x0030)
class UStruct : public UField
{
public:
	class UStruct* SuperField;                                               // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	class UField* Children;                                                 // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	INT32 PropertySize;                                             // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	INT32 MinAlignment;                                             // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY
	char pad_0048[64];                                             // 0x0000(0x0000) NOT AUTO-GENERATED PROPERTY

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class CoreUObject.Struct");
		return ptr;
	}

};

// Class CoreUObject.Class
// 0x0178 (0x0200 - 0x0088)
class UClass : public UStruct
{
public:
	unsigned char                                      UnknownData00[0x178];                                     // 0x0088(0x0178) MISSED OFFSET

	template <typename T>
	constexpr auto CreateDefaultObject() noexcept -> T*
	{
		return static_cast<T*>(CreateDefaultObject());
	}

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class CoreUObject.Class");
		return ptr;
	}

};



class FUObjectItem
{
	enum class EInternalObjectFlags : INT32
	{
		None = 0,
		ReachableInCluster = 1 << 23,
		ClusterRoot = 1 << 24,
		Native = 1 << 25,
		Async = 1 << 26,
		AsyncLoading = 1 << 27,
		Unreachable = 1 << 28,
		PendingKill = 1 << 29,
		RootSet = 1 << 30,
		GarbageCollectionKeepFlags = Native | Async | AsyncLoading,
		AllFlags = ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet,
	};

public:
	class UObject* Object;
	INT32 Flags;
	INT32 ClusterRootIndex;
	INT32 SerialNumber;
	char pad_0014[4];

	[[nodiscard]] constexpr auto IsUnreachable() const noexcept
	{
		return !!(Flags & static_cast<std::underlying_type_t<EInternalObjectFlags>>(EInternalObjectFlags::Unreachable));
	}

	[[nodiscard]] constexpr auto IsPendingKill() const noexcept
	{
		return !!(Flags & static_cast<std::underlying_type_t<EInternalObjectFlags>>(EInternalObjectFlags::PendingKill));
	}
};


class FChunkedFixedUObjectArray
{
public:
	enum
	{
		NumElementsPerChunk = 64 * 1024
	};

	[[nodiscard]] constexpr auto Num() const noexcept
	{
		return NumElements;
	}

	[[nodiscard]] constexpr auto GetObjectPtr(INT32 Index) const noexcept
	{
		const auto ChunkIndex = Index / NumElementsPerChunk;
		const auto WithinChunkIndex = Index % NumElementsPerChunk;
		return Objects[ChunkIndex] + WithinChunkIndex;
	}

	[[nodiscard]] constexpr auto& GetByIndex(INT32 Index) const noexcept
	{
		return *GetObjectPtr(Index);
	}

private:
	class FUObjectItem** Objects;
	class FUObjectItem* PreAllocatedObjects;
	INT32 MaxElements;
	INT32 NumElements;
	INT32 MaxChunks;
	INT32 NumChunks;
};

// Class Engine.Actor
// 0x0430 (0x0458 - 0x0028)
class AActor : public UObject
{
public:

};

// Class Engine.Level
// 0x02F8 (0x0320 - 0x0028)
class ULevel : public UObject
{
public:
	unsigned char UnknownData00[0x78];	// 0x0028(0x0098) MISSED OFFSET
	TArray<class AActor*> Actors;		// 0x00A0(0x0010)
};

// Class Engine.World
// 0x0960 (0x0988 - 0x0028)
class UWorld : public UObject
{
public:
	unsigned char UnknownData00[0x8];	// 0x0028(0x0008) MISSED OFFSET
	class ULevel* PersistentLevel;		// 0x0030(0x0008) (ZeroConstructor, Transient, IsPlainOldData)
};

// Class Engine.ScriptViewportClient
// 0x0020 (0x0048 - 0x0028)
class UScriptViewportClient : public UObject
{
public:
	unsigned char UnknownData00[0x20]; // 0x0028(0x0020) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.ScriptViewportClient");
		return ptr;
	}

};

// Class Engine.GameViewportClient
// 0x02D0 (0x0318 - 0x0048)
class UGameViewportClient : public UScriptViewportClient
{
public:
	unsigned char UnknownData01[0x40];	// 0x0048(0x0040) MISSED OFFSET
	class UWorld* World;				// 0x0088(0x0008) (ZeroConstructor, IsPlainOldData)
	void* GameInstance;	// 0x0090(0x0008) (ZeroConstructor, IsPlainOldData)
	unsigned char UnknownData02[0x280];	// 0x0098(0x0280) MISSED OFFSET

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindClass("Class Engine.GameViewportClient");
		return ptr;
	}
};


// Class Engine.Engine
// 0x11A8 (0x11D0 - 0x0028)
class UEngine : public UObject
{
public:
	unsigned char UnknownData00[0x788]; // 0x28(0x788)
	class UGameViewportClient* GameViewport; // 0x7B0(0x08)
};

static UEngine* GEngine;