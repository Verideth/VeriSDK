#pragma once

#include <Windows.h>
#include "Bits.h"

#define WIN32_LEAN_AND_MEAN

typedef unsigned int uint32;
typedef float vec_t;

vec_t BitsToFloat(uint32 i)
{
	union Convertor_t
	{
		vec_t f;
		unsigned long ul;
	}tmp;
	tmp.ul = i;
	return tmp.f;
}

uint32 const FloatBits(const vec_t &f)
{
	union Convertor_t
	{
		vec_t f;
		uint32 ul;
	}tmp;
	tmp.f = f;
	return tmp.ul;
}

inline bool IsFinite(const vec_t &f)
{
#if _X360
	return f == f && fabs(f) <= FLT_MAX;
#else
	return ((FloatBits(f) & 0x7F800000) != 0x7F800000);
#endif
}

#define IN_JUMP (1 << 1)
#define	FL_ONGROUND (1 << 0)
#define VALVE_RAND_MAX 0x7FFF
#define  Assert( _exp )										((void)0)
#define FLOAT32_NAN_BITS     (uint32)0x7FC00000	// not a number!
#define VEC_T_NAN FLOAT32_NAN

#ifdef VECTOR_PARANOIA
#define CHECK_VALID( _v)	Assert( (_v).IsValid() )
#else
#ifdef GNUC
#define CHECK_VALID( _v)
#else
#define CHECK_VALID( _v)	0
#endif
#endif

typedef unsigned short MaterialHandle_t;

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

#define FLOAT32_NAN BitsToFloat( FLOAT32_NAN_BITS )

extern class QAngleByValue;

namespace VeriSDK
{
	class QAngle
	{
	public:
		// Members
		vec_t x, y, z;

		// Construction/destruction
		QAngle(void)
		{
		#ifdef _DEBUG
		#ifdef VECTOR_PARANOIA
			// Initialize to NAN to catch errors
			x = y = z = VEC_T_NAN;
		#endif
		#endif
		}

		QAngle(vec_t X, vec_t Y, vec_t Z)
		{
			x = X; y = Y; z = Z;
		}
		//	QAngle(RadianEuler const &angles);	// evil auto type promotion!!!

		// Allow pass-by-value
		operator QAngleByValue &() { return *((QAngleByValue *)(this)); }
		operator const QAngleByValue &() const { return *((const QAngleByValue *)(this)); }

		// Initialization
		void Init(vec_t ix = 0.0f, vec_t iy = 0.0f, vec_t iz = 0.0f)
		{
			x = ix; y = iy; z = iz;
		}

		void Random(vec_t minVal, vec_t maxVal)
		{
			x = minVal + ((float)rand() / VALVE_RAND_MAX) * (maxVal - minVal);
			y = minVal + ((float)rand() / VALVE_RAND_MAX) * (maxVal - minVal);
			z = minVal + ((float)rand() / VALVE_RAND_MAX) * (maxVal - minVal);
		}	

		// Got any nasty NAN's?
		bool IsValid()
		{
			return IsFinite(x) && IsFinite(y) && IsFinite(z);
		}

		void Invalidate()
		{
			//#ifdef _DEBUG
			//#ifdef VECTOR_PARANOIA
			x = y = z = VEC_T_NAN;
			//#endif
			//#endif
		}

		// array access...
		vec_t operator[](int i) const;

		vec_t& operator[](int i)
		{
			Assert((i >= 0) && (i < 3));
			return ((vec_t*)this)[i];
		}

		// Base address...
		vec_t* Base();
		vec_t const* Base() const;

		// equality
		//bool operator==(const QAngle& v) const;
		//bool operator!=(const QAngle& v) const;

		// arithmetic operations
		//QAngle&	operator+=(const QAngle &v);
		//QAngle&	operator-=(const QAngle &v);
		//QAngle&	operator*=(float s);
		//QAngle&	operator/=(float s);

		// Get the vector's magnitude.
		vec_t	Length() const;
		vec_t	LengthSqr() const;

		// negate the QAngle components
		//void	Negate(); 

		// Dump QAngle output
		__inline void dump(const char* name)
		{
			printf("%s: %.2f %.2f %.2f\n", name, x, y, z);
		}

		// No assignment operators either...
		//QAngle& operator=(const QAngle& src);

#ifndef VECTOR_NO_SLOW_OPERATIONSw
		// copy constructors

		// arithmetic operations
		QAngle	operator-(void) const;

		QAngle	operator+(const QAngle& v) const;
		QAngle	operator-(const QAngle& v) const;
		QAngle	operator*(float fl) const;
		QAngle	operator/(float fl) const;
#else

	private:
		// No copy constructors allowed if we're in optimal mode
		QAngle(const QAngle& vOther);

#endif
	};

	class QAngleByValue : public QAngle
	{
	public:
		// Construction/destruction:
		QAngleByValue(void) : QAngle() {}
		QAngleByValue(vec_t X, vec_t Y, vec_t Z) : QAngle(X, Y, Z) {}
		QAngleByValue(const QAngleByValue& vOther) { *this = vOther; }
	};

	class Vector
	{
	public:
		float x;
		float y;
		float z;

		inline void Init(float ix, float iy, float iz)
		{
			x = ix; y = iy; z = iz;
		}

		Vector() { x = 0; y = 0; z = 0; };
		Vector(float X, float Y, float Z) { x = X; y = Y; z = Z; };

		float operator[](int i) const { if (i == 1) return x; if (i == 2) return y; return z; };
		float& operator[](int i) { if (i == 1) return x; if (i == 2) return y; return z; };

		bool operator==(const Vector& v) { return true; }
		bool operator!=(const Vector& v) { return true; }

		inline Vector operator-(const Vector& v) { return Vector(x - v.x, y - v.y, z - v.z); }
		inline Vector operator+(const Vector& v) { return Vector(x + v.x, y + v.y, z + v.z); }
		inline Vector operator*(const int n) { return Vector(x*n, y*n, z*n); }
		inline Vector operator-() { return Vector(-x, -y, -z); }
		inline Vector& operator+=(const Vector &v) { x += v.x; y += v.y; z += v.z; return *this; }
		float LengthSqr(void) { return (x*x + y*y + z*z); }
	};

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

		int GetLocalPlayer()
		{
			typedef int(__thiscall* fnLocal)(PVOID);

			return getvfunc<fnLocal>(this, 12)(this);
		}

		void SetViewAngles(Vector& vAngles)
		{
			typedef void(__thiscall* fnVA)(PVOID, Vector&);

			return getvfunc<fnVA>(this, 19)(this, vAngles);
		}
	};

	class BaseClientDLL
	{
	public:
		void CreateMove(int sequence_number, float frametime, bool active)
		{
			typedef void(__thiscall* fnMove)(void*, int, float, bool);

			return getvfunc<fnMove>(this, 21)(this, sequence_number, frametime, active);
		}
	};

	class UserCmd
	{
	public:
		int	commandNumber;
		int	tickCount;
		Vector viewAngles;
		Vector aimDirection;
		Vector move;
		int	buttons;
		char impulse;
		int weaponSelect;
		int	weaponSubtype;
		int	randomSeed;
		short mousedx;
		short mousedy;
		bool hasBeenPredicted;
	};

	class VerifiedUserCmd
	{
	public:
		UserCmd	verifyCmd;
		unsigned long verifyCrc;
	};

	class Input
	{
	public:
		virtual ~Input()
		{

		}

		UserCmd* commands;
		VerifiedUserCmd* verifiedCommands;
	};

	class ClientEntity
	{
	public:
		Vector GetOrigin()
		{
			return *reinterpret_cast<Vector*>((DWORD)this + (DWORD)0x134);
		}

		int GetTeamNum()
		{
			return *reinterpret_cast<int*>((DWORD)this + (DWORD)0xF0);
		}

		int GetFlags()
		{
			return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x100);
		}
	};

	class BasePlayer : public ClientEntity 
	{
	
	};

	class ClientEntityList
	{
	public:
		void GetHighestEntityIndex(void)
		{
			typedef void(__thiscall* fnGetHighestIndex)(void*);

			return getvfunc<fnGetHighestIndex>(this, 6)(this);
		}

		inline ClientEntity* GetClientEntity(int Index) 
		{
			return getvfunc<ClientEntity*(__thiscall *)(void *, int)>(this, 3)(this, Index);
		}
	};

	class IMaterial
	{

	};
}

VeriSDK::IFTools* tools = new VeriSDK::IFTools;

// typedefs
typedef void(__thiscall* fnPaintTraverse)(void*, unsigned int, bool, bool);
typedef void(__thiscall* fnCreateMove)(void*, int, float, bool);

// had to put at the bottom for BHop and other cheats
VeriSDK::Engine* engine;
VeriSDK::BaseClientDLL* clientMode;
