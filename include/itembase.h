#pragma once

#include <cstdlib>

class ItemBase
{
private:
	char* name;                 // The name of the item

public:
	ItemBase() : name(nullptr) {}
	virtual ~ItemBase() { if (name) delete[] name; }

	char const* GetName() const { return name ? name : ""; }
	void SetName(char const* newName)
	{
		if (name) delete[] name;
		if (newName) {
			int n = strlen(newName);
			name = new char[n + 1];
			for (int i = 0; i < n; i++) name[i] = newName[i];
			name[n] = '\0';
		}
		else { name = nullptr; }
	}
};
