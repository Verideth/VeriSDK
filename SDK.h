#pragma once

#define WIN32_LEAN_AND_MEAN

enum VeriFontFlags
{
	FONTFLAG_NONE,
	FONTFLAG_ITALIC = 0x001,
	FONTFLAG_UNDERLINE = 0x002,
	FONTFLAG_STRIKEOUT = 0x004,
	FONTFLAG_SYMBOL = 0x008,
	FONTFLAG_ANTIALIAS = 0x010,
	FONTFLAG_GAUSSIANBLUR = 0x020,
	FONTFLAG_ROTARY = 0x040,
	FONTFLAG_DROPSHADOW = 0x080,
	FONTFLAG_ADDITIVE = 0x100,
	FONTFLAG_OUTLINE = 0x200,
	FONTFLAG_CUSTOM = 0x400,
	FONTFLAG_BITMAP = 0x800,
};

#include <Windows.h>

// hooks
typedef void(__thiscall* hkPaintTraverse)(void*, unsigned int, bool, bool);

typedef void* (*Interface)(char* name, int returnVal);

// credits @uc
template<typename Function>Function getvfunc(void*_VMT, int Index)
{
	void***_TVMT = (void***)_VMT;
	void**VMT = *_TVMT;
	void*_Address = VMT[Index];
	return(Function)(_Address);
}

// credits @uc
class CVMTHookManager
{
private:
	PDWORD*	m_ppdwClassBase;
	PDWORD	m_pdwNewVMT, m_pdwOldVMT;
	DWORD	m_dwVMTSize;

public:
	CVMTHookManager(void)
	{
		memset(this, 0, sizeof(CVMTHookManager));
	}

	CVMTHookManager(PDWORD* ppdwClassBase)
	{
		Initialize(ppdwClassBase);
	}

	~CVMTHookManager(void)
	{
		UnHook();
	}

	bool Initialize(PDWORD* ppdwClassBase)
	{
		m_ppdwClassBase = ppdwClassBase;
		m_pdwOldVMT = *ppdwClassBase;
		m_dwVMTSize = GetVMTCount(*ppdwClassBase);
		m_pdwNewVMT = new DWORD[m_dwVMTSize];
		memcpy(m_pdwNewVMT, m_pdwOldVMT, sizeof(DWORD) * m_dwVMTSize);
		*ppdwClassBase = m_pdwNewVMT;
		return true;
	}

	bool Initialize(PDWORD** pppdwClassBase)
	{
		return Initialize(*pppdwClassBase);
	}

	void UnHook(void)
	{
		if (m_ppdwClassBase)
		{
			*m_ppdwClassBase = m_pdwOldVMT;
		}
	}

	void ReHook(void)
	{
		if (m_ppdwClassBase)
		{
			*m_ppdwClassBase = m_pdwNewVMT;
		}
	}

	int GetFuncCount(void)
	{
		return (int)m_dwVMTSize;
	}

	DWORD GetMethodAddress(int Index)
	{
		if (Index >= 0 && Index <= (int)m_dwVMTSize && m_pdwOldVMT != NULL)
		{
			return m_pdwOldVMT[Index];
		}
		return NULL;
	}

	PDWORD GetOldVMT(void)
	{
		return m_pdwOldVMT;
	}

	DWORD HookMethod(DWORD dwNewFunc, unsigned int iIndex)
	{
		if (m_pdwNewVMT && m_pdwOldVMT && iIndex <= m_dwVMTSize && iIndex >= 0)
		{
			m_pdwNewVMT[iIndex] = dwNewFunc;
			return m_pdwOldVMT[iIndex];
		}
		return NULL;
	}

	template< class T > T GetMethod(unsigned int nIndex) const
	{
		return (T)m_pdwOldVMT[nIndex];
	}

private:
	DWORD GetVMTCount(PDWORD pdwVMT)
	{
		DWORD dwIndex = 0;

		for (dwIndex = 0; pdwVMT[dwIndex]; dwIndex++)
		{
			if (IsBadCodePtr((FARPROC)pdwVMT[dwIndex]))
			{
				break;
			}
		}
		return dwIndex;
	}
};

class Color
{
public:

	unsigned char color[4];

	Color(float r, float g, float b, float a)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		color[3] = a;
	}
};

namespace VeriSDK
{
	class Surface
	{
	public:
		// Create a font
		long CreateFont()
		{
			typedef	int(__thiscall* fnCreateFont)(void*);

			return getvfunc<fnCreateFont>(this, 71)(this);
		}

		// Set font glyph set
		void SetFontGlyphSet(unsigned long& font, const char* fontName, int h, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0)
		{
			typedef void(__thiscall* fnSFGS)(void*, unsigned long, const char*, int, int, int, int, int, int, int);

			return getvfunc<fnSFGS>(this, 72)(this, font, fontName, h, weight, blur, scanlines, flags, nRangeMin, nRangeMax);
		}

		// Draw color
		void DrawSetColor(Color col)
		{
			typedef void(__thiscall* fnDrawSetColor)(void*, Color);

			return getvfunc<fnDrawSetColor>(this, 14)(this, col);
		}

		void DrawOutlinedRect(int x, int y, int w, int h)
		{
			typedef void(__thiscall* fnOutlinedRect)(void*, int, int, int, int);

			return getvfunc<fnOutlinedRect>(this, 18)(this, x, y, w, h);
		}

		// Text to draw, length, length of text, and type, type of text. Nor
		void DrawPrintText(wchar_t* text, int length, int type)
		{

			typedef void(__thiscall* fnTTP) (void*, wchar_t*, int, int);

			return getvfunc<fnTTP>(this, 28) (this, text, length, type);
		}

		void DrawSetTextPos(int x, int y)
		{
			typedef void(__thiscall* fnSetTextPos)(void*, int, int);

			return getvfunc<fnSetTextPos>(this, 26)(this, x, y);
		}

		void DrawSetTextFont(int font)
		{
			typedef void(__thiscall* fnFont)(void*, int);

			return getvfunc<fnFont>(this, 23)(this, font);
		}

		void DrawSetTextColor(Color col)
		{
			typedef void(__thiscall* fnCol) (void*, Color);

			return getvfunc<fnCol>(this, 24)(this, col);
		}
	};

	class Panel
	{
	public:
		const char* GetName(int panelID)
		{
			typedef const char* (__thiscall* fnPanel) (void*, int);

			return getvfunc<fnPanel>(this, 36)(this, panelID);
		}
	};

	class IFTools
	{
	public:
		virtual void* IFInfo(char* ModuleName, char* InterfaceName)
		{
			Interface TMP = (Interface)GetProcAddress(GetModuleHandleA(ModuleName), "CreateInterface");

			return (void*)TMP(InterfaceName, 0);
		}
	};

	class Engine
	{
	public:
		void ScreenSize(int& w, int& h)
		{
			typedef void(__thiscall* fnScreenSize)(void*, int&, int&);

			return getvfunc<fnScreenSize>(this, 5)(this, w, h);
		}
	};
}
