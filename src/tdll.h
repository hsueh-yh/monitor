#ifndef _TDLL_
#define _TDLL_

//#include <Windows.h>
#include <iostream>
extern "C"
{
#include <dlfcn.h>
}

using namespace std;

class Tdll
{
private:

	void *handl;
	void loadDll(const char *dllName);
public:
	bool ok;
	Tdll(const char *dllName1)
	{
		handl = dlopen( dllName1, RTLD_LAZY );

		if (!handl)
		{
			handl = NULL;
		}
		ok = (handl != NULL);
	};

	~Tdll()
	{
		if (handl)
			dlclose(handl);
	}

	void loadFunction(void **fnc, const char *name)
	{
		*fnc = dlsym(handl, name);
        //cout << *fnc << endl;
		ok &= (*fnc != NULL);
	}
};

#endif
