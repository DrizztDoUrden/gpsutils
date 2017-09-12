#pragma once

#ifdef WINDOWS
	#ifndef GPSU_STATIC
		#ifdef GPSU_EXPORT
			#define GPSU_API _declspec(dllexport) 
		#else
			#define GPSU_API _declspec(dllimport)
		#endif
	#else
		#define GPSU_API
	#endif
#else
	#define GPSU_API
#endif
