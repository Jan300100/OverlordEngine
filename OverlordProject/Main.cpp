#include "stdafx.h"
//#include <vld.h>
#include "MainGame.h"
#include "CmdOptions.h"
#include <dxgidebug.h>

int wmain(int argc, wchar_t* argv[])
{
	std::wstring cmdLine{};
	for (int i = 0; i < argc; i++)
	{
		cmdLine += argv[i];
	}
#pragma warning(push)
#pragma warning(disable: 6387)
	wWinMain(GetModuleHandle(0), 0, (PWSTR)cmdLine.c_str(), SW_SHOW);
#pragma warning(pop)
}

#pragma warning(push)
#pragma warning(disable: 28251 6387)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	CmdOptions::Create(pCmdLine);

	UNREFERENCED_PARAMETER(nCmdShow);
	UNREFERENCED_PARAMETER(pCmdLine);
	//notify user if heap is corrupt
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL,0);

	// Enable run-time memory leak check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

		typedef HRESULT(__stdcall *fPtr)(const IID&, void**); 
		HMODULE hDll = LoadLibrary("dxgidebug.dll"); 
		fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface"); 

		IDXGIDebug* pDXGIDebug;
		DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDXGIDebug);
		//_CrtSetBreakAlloc(10725);
	#endif

		//init rand
		//srand(unsigned int(time(0)));


	auto pGame = new MainGame();
	auto result = pGame->Run(hInstance);
	UNREFERENCED_PARAMETER(result);
	delete pGame;
	
	return 0;
}
#pragma warning(pop)
