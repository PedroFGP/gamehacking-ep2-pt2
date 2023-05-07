#include "pch.h"

auto UObject::GetName() const noexcept -> std::string
{
	std::string name{ Name.GetName() };
	if (Name.Number > 0)
		name += '_' + std::to_string(Name.Number);

	const auto pos = name.rfind('/');
	if (pos == std::string::npos)
		return name;

	return name.substr(pos + 1);
}

auto UObject::GetFullName() const noexcept -> std::string
{
	std::string name;
	if (Class == nullptr)
		return name;

	std::string temp;
	for (auto p = Outer; p; p = p->Outer)
		temp = p->GetName() + "." + temp;

	name = Class->GetName();
	name += " ";
	name += temp;
	name += GetName();

	return name;
}

auto UObject::IsA(UClass* cmp) const noexcept -> bool
{
	for (auto super = Class; super; super = static_cast<UClass*>(super->SuperField))
		if (super == cmp)
			return true;
	return false;
}