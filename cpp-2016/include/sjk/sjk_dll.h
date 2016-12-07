#pragma once
#ifndef SJK_DLL_H_INCLUDED
#define SJK_DLL_H_INCLUDED

#ifdef _WIN32
	#include <windows.h>
#else

#endif

#include "sjk_exception.h"
#include "sjk_assert.h"

namespace sjk
{
	namespace dll
	{
		// dll::loader handles lifetime of loading a dll
		struct loader
		{
			loader(const loader& other) = delete;
			loader& operator=(const loader& other) = delete;
			loader(const std::string& path) : m_handle(NULL)
			{
				load(path);
			}

			loader() : m_handle(NULL){}
			
			~loader(){
				clear();
			}
			
			#ifndef _WIN32
				#define FARPROC 
				#define WINAPI
			#endif
			
			FARPROC WINAPI get_func_address(const std::string& function_name){

				if (!m_handle) {
					SJK_EXCEPTION("get_func_address, with name: ", function_name, "failed - no dll handle");
					return NULL;
				}
				auto ptr = GetProcAddress(m_handle, function_name.c_str());
				if (!ptr) {
					SJK_EXCEPTION("No exported function found, with name: ", function_name);
				}
				return ptr;
			}

			const std::string& path() const {
				return m_spath;
			}

			void load(const std::string path) {
				if (m_handle) {
					clear();
				}
				m_handle = LoadLibraryA(path.c_str());
				if (m_handle == NULL) {
					SJK_EXCEPTION("Unable to load dll: ", path, "GetLastError() said: ", sjk::exception::error_string(GetLastError()));
				}
				m_spath = path;
			}
		private:
			std::string m_spath;
			HMODULE m_handle;

			void clear() {
				if (m_handle)
				{
					FreeLibrary(m_handle);
					m_handle = NULL;
				}
			}
		};
	
	}

}


#endif