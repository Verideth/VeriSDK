#pragma once

#include <Windows.h>
#include "Bits.h"

#define WIN32_LEAN_AND_MEAN

#define MAXSTUDIOSKINS 32

typedef unsigned int uint32;
typedef float vec_t;

#define TEXTURE_GROUP_MODEL	"Model textures"

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

typedef unsigned short MaterialHandle;

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

enum OverrideType
{
	OVERRIDE_NORMAL = 0,
	OVERRIDE_BUILD_SHADOWS,
	OVERRIDE_DEPTH_WRITE,
	OVERRIDE_SSAO_DEPTH_WRITE,
};

enum MaterialVarFlags
{
	MATERIAL_VAR_DEBUG = (1 << 0),
	MATERIAL_VAR_NO_DEBUG_OVERRIDE = (1 << 1),
	MATERIAL_VAR_NO_DRAW = (1 << 2),
	MATERIAL_VAR_USE_IN_FILLRATE_MODE = (1 << 3),
	MATERIAL_VAR_VERTEXCOLOR = (1 << 4),
	MATERIAL_VAR_VERTEXALPHA = (1 << 5),
	MATERIAL_VAR_SELFILLUM = (1 << 6),
	MATERIAL_VAR_ADDITIVE = (1 << 7),
	MATERIAL_VAR_ALPHATEST = (1 << 8),
	MATERIAL_VAR_ZNEARER = (1 << 10),
	MATERIAL_VAR_MODEL = (1 << 11),
	MATERIAL_VAR_FLAT = (1 << 12),
	MATERIAL_VAR_NOCULL = (1 << 13),
	MATERIAL_VAR_NOFOG = (1 << 14),
	MATERIAL_VAR_IGNOREZ = (1 << 15),
	MATERIAL_VAR_DECAL = (1 << 16),
	MATERIAL_VAR_ENVMAPSPHERE = (1 << 17), // OBSOLETE
	MATERIAL_VAR_ENVMAPCAMERASPACE = (1 << 19), // OBSOLETE
	MATERIAL_VAR_BASEALPHAENVMAPMASK = (1 << 20),
	MATERIAL_VAR_TRANSLUCENT = (1 << 21),
	MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
	MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = (1 << 23), // OBSOLETE
	MATERIAL_VAR_OPAQUETEXTURE = (1 << 24),
	MATERIAL_VAR_ENVMAPMODE = (1 << 25), // OBSOLETE
	MATERIAL_VAR_SUPPRESS_DECALS = (1 << 26),
	MATERIAL_VAR_HALFLAMBERT = (1 << 27),
	MATERIAL_VAR_WIREFRAME = (1 << 28),
	MATERIAL_VAR_ALLOWALPHATOCOVERAGE = (1 << 29),
	MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY = (1 << 30),
	MATERIAL_VAR_VERTEXFOG = (1 << 31),
};

typedef void* (*Interface)(char* name, int returnVal); // This is supposed to be CreateInterfaceFn but whatever

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

	float* Base()
	{
		float clr[3];

		clr[0] = color[0] / 255.0f;
		clr[1] = color[1] / 255.0f;
		clr[2] = color[2] / 255.0f;

		return &clr[0];
	}

};

#define FLOAT32_NAN BitsToFloat( FLOAT32_NAN_BITS )

extern class QAngleByValue;

namespace VeriSDK
{
	struct model_t;
	typedef unsigned short ModelInstanceHandle_t;
	typedef float matrix3x4_t[3][4];

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

	class Material
	{
	public:
		void SetMaterialVarFlag(MaterialVarFlags flag, bool on) // doesnt work
		{
			typedef void(__thiscall* fnMaterialVarFlag)(void*, MaterialVarFlags, bool);

			return getvfunc<fnMaterialVarFlag>(this, 29)(this, flag, on);
		}

		void AlphaModulate(float alpha)
		{
			typedef void(__thiscall* fnAlphaModulate)(void*, float);

			return getvfunc<fnAlphaModulate>(this, 27)(this, alpha);
		}
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

	struct studiohdr_t
	{
		int id;
		int version;
		int checksum;
		char name[64];
		int length;
		Vector eyeposition;
		Vector illumposition;
		Vector hull_min;
		Vector hull_max;
		Vector view_bbmin;
		Vector view_bbmax;
		int flags;
		int numbones;
		int boneindex;
		//	inline mstudiobone_t *pBone(int i) const { Assert(i >= 0 && i < numbones); return (mstudiobone_t *)(((byte *)this) + boneindex) + i; };
		int numbonecontrollers;
		int bonecontrollerindex;
		int numhitboxsets;
		int hitboxsetindex;
		int numlocalanim;
		int localanimindex;
		int numlocalseq;
		int localseqindex;
		mutable int activitylistversion;
		mutable int eventsindexed;
		int numtextures;
		int textureindex;
		int numcdtextures;
		int cdtextureindex;
		int numskinref;
		int numskinfamilies;
		int skinindex;
		int numbodyparts;
		int bodypartindex;
		int numlocalattachments;
		int localattachmentindex;
		int numlocalnodes;
		int localnodeindex;
		int localnodenameindex;
		int numflexdesc;
		int flexdescindex;
		int numflexcontrollers;
		int flexcontrollerindex;
		int numflexrules;
		int flexruleindex;
		int numikchains;
		int ikchainindex;
		int nummouths;
		int mouthindex;
		int numlocalposeparameters;
		int localposeparamindex;
		int surfacepropindex;
		int keyvalueindex;
		int keyvaluesize;
		int numlocalikautoplaylocks;
		int localikautoplaylockindex;
		float mass;
		int contents;
		int numincludemodels;
		int includemodelindex;
		mutable void* virtualModel;
		int szanimblocknameindex;
		int numanimblocks;
		int animblockindex;
		mutable void* animblockModel;
		int bonetablebynameindex;
		void* pVertexBase;
		void* pIndexBase;
		byte constdirectionallightdot;
		byte rootLOD;
		byte numAllowedRootLODs;
		byte unused[1];
		int unused4;
		int numflexcontrollerui;
		int flexcontrolleruiindex;
		float flVertAnimFixedPointScale;
		int unused3[1];
		int studiohdr2index;
		int unused2[1];
	};

	class ModelInfo
	{
	public:
		const char* GetModelName(const model_t* modelName)
		{
			typedef const char*(__thiscall* fnGetModelName)(void*, const model_t*);

			return getvfunc<fnGetModelName>(this, 3)(this, modelName);
		}

		studiohdr_t* GetStudiomodel(const model_t* mod) 
		{
			typedef studiohdr_t*(__thiscall* fnStudiomodel)(void*, const model_t*);

			return getvfunc<fnStudiomodel>(this, 28)(this, mod);
		}

		void GetModelMaterials(const model_t* model, int count, Material** mat)
		{
			typedef void(__thiscall* fnGetModelMaterials)(void*, const model_t*, int, Material**);

			return getvfunc<fnGetModelMaterials>(this, 18)(this, model, count, mat);
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

	class ViewSetup
	{
	public:
		int x;
		int x_old;
		int y;
		int y_old;
		int width;
		int	width_old;
		int height;
		int	height_old;
		float fov;
		float fovViewmodel;
		Vector origin;
		QAngle angles;
		float fNear;
		float fFar;
		float nearViewmodel;
		float farViewmodel;
		float aspectRatio;
		float nearBlurDepth;
		float nearFocusDepth;
		float farFocusDepth;
		float farBlurDepth;
		float nearBlurRadius;
		float farBlurRadius;
		float doFQuality;
		int motionBlurMode;
		int edgeBlur;
	};

	class BaseClientDLL
	{
	public:
		bool GetPlayerView(ViewSetup& playerView)
		{
			typedef bool(__thiscall* fnPlyView)(void*, ViewSetup&);

			return getvfunc<fnPlyView>(this, 58)(this, playerView);
		}
	};
	
	class RenderView
	{
	public:
		void SetColorModulation(float const* blend)
		{
			typedef void(__thiscall* fnColorModulation)(void*, float const*);

			return getvfunc<fnColorModulation>(this, 28)(this, blend);
		}
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

	struct mstudiobone_t;

	class BasePlayer
	{

	};

	class BaseEntity
	{
	public:
		typedef int(__thiscall* fnGetTeam)(void*);
		typedef void(__thiscall* fnRenderBounds)(void*, Vector&, Vector&);
		typedef bool(__thiscall* fnAlive)(void*);

		void GetRenderBounds(Vector& min, Vector& max)
		{
			void* renderable = (PVOID)(this + 0x4);

			return getvfunc<fnRenderBounds>(this, 20)(renderable, min, max);
		}

		bool IsAlive()
		{
			return getvfunc<fnAlive>(this, 200)(this);
		}

		bool IsPlayer()
		{
			typedef bool(__thiscall* fnPlayer)(void*);

			return getvfunc<fnPlayer>(this, 202)(this);
		}

		bool IsDormant()
		{
			return *(bool*)((DWORD)this + 0xE9);
		}

		model_t* GetModel()
		{
			return *(model_t**)((DWORD)this + 0x6C);
		}

		int GetFlags()
		{
			return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x100);
		}

		Vector& GetVecOrigin()
		{
			return *(Vector*)((DWORD)this + 0x134);
		}

		QAngle& GetAbsAngles()
		{
			return *(QAngle*)((DWORD)this + 0x4D0C);
		}

		int GetTeam()
		{
			return getvfunc<fnGetTeam>(this, 124)(this);
		}
	};

	class IClientRenderable
	{
	public:

	};

	class MatRenderContext
	{
	public:

	};

	struct ModelRenderInfo_t
	{
		Vector origin;
		QAngle angles;
		IClientRenderable* pRenderable;
		const model_t *pModel;
		const matrix3x4_t *pModelToWorld;
		const matrix3x4_t *pLightingOffset;
		const Vector *pLightingOrigin;
		int flags;
		int entity_index;
		int skin;
		int body;
		int hitboxset;
		ModelInstanceHandle_t instance;

		ModelRenderInfo_t()
		{
			pModelToWorld = NULL;
			pLightingOffset = NULL;
			pLightingOrigin = NULL;
		}
	};

	class ClientEntityList
	{
	public:
		int GetHighestEntityIndex(void)
		{
			typedef int(__thiscall* fnGetHighestIndex)(void*);

			return getvfunc<fnGetHighestIndex>(this, 6)(this);
		}

		BaseEntity* GetClientEntity(int index)
		{
			typedef BaseEntity*(__thiscall* fnClientEnt)(void*, int);

			return getvfunc<fnClientEnt>(this, 3)(this, index);
		}
	};

	class DebugOverlay
	{
	public:
		void AddBoxOverlay(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration)
		{
			typedef void(__thiscall* fnBoxOverlay)(void*, const Vector&, const Vector&, const Vector&, QAngle const&, int, int, int, int, float);

			getvfunc<fnBoxOverlay>(this, 1)(this, origin, mins, max, orientation, r, g, b, a, duration);
		}
	};

	class ClientMode
	{
	public:
		void __fastcall OverrideView(uintptr_t ecx, uintptr_t edx, VeriSDK::ViewSetup* setup)
		{
			typedef void(__thiscall* fnOverrideView)(uintptr_t, VeriSDK::ViewSetup*);

			getvfunc<fnOverrideView>(this, 18)(ecx, setup);
		}
	};

	struct StudioDecalHandle_t 
	{ 
		int unused; 
	};

	struct studiohwdata_t;

	struct DrawModelState_t
	{
		studiohdr_t* m_pStudioHdr;
		studiohwdata_t*	m_pStudioHWData;
		IClientRenderable* m_pRenderable;
		const matrix3x4_t* m_pModelToWorld;
		StudioDecalHandle_t	m_decals;
		int	m_drawFlags;
		int	m_lod;
	};

	class ModelRender
	{
	public:
		void DrawModelExecute(MatRenderContext* ctx, const DrawModelState_t &state, const ModelRenderInfo_t &info, matrix3x4_t *customBoneToWorld)
		{
			typedef void(__thiscall* fnDrawModelExecute)(void*, MatRenderContext*, DrawModelState_t, const ModelRenderInfo_t&, matrix3x4_t*);

			return getvfunc<fnDrawModelExecute>(this, 21)(this, ctx, state, info, customBoneToWorld);
		}

		void ForcedMaterialOverride(Material* newMat, OverrideType overrideType = OVERRIDE_NORMAL)
		{
			typedef void(__thiscall* fnMaterialOverride)(void*, Material*, OverrideType);

			return getvfunc<fnMaterialOverride>(this, 1)(this, newMat, overrideType);
		}
	};

	class MatSys
	{
	public:
		Material* FindMaterial(char const* materialName, const char* textureGroupName, bool complain = true, const char* complainPrefix = NULL)
		{
			typedef Material*(__thiscall* fnFindMat)(void*, char const*, const char*, bool, const char*);

			return getvfunc<fnFindMat>(this, 84)(this, materialName, textureGroupName, complain, complainPrefix);
		}
	};

	class ConVar
	{
	public:
		void* fnChangeCallback;

		void SetValue(const char* val)
		{
			typedef void(__thiscall* fnConvarSetValue)(void*, const char*);

			return getvfunc<fnConvarSetValue>(this, 11)(this, val);
		}
	};

	class CVar
	{
	public:
		ConVar* FindVar(const char* varName)
		{
			typedef ConVar*(__thiscall* fnCVarFindVar)(void*, const char*);

			return getvfunc<fnCVarFindVar>(this, 14)(this, varName);
		}
	};
}

VeriSDK::IFTools* tools = new VeriSDK::IFTools;

// typedefs
typedef void(__thiscall* fnPaintTraverse)(void*, unsigned int, bool, bool);
typedef void(__thiscall* fnCreateMove)(void*, float, VeriSDK::UserCmd*);
typedef void(__thiscall* fnOverrideView)(void*, VeriSDK::ViewSetup*); 
typedef void(__thiscall* fnDrawModelExecute)(void*, VeriSDK::MatRenderContext* ctx, const VeriSDK::DrawModelState_t &state, const VeriSDK::ModelRenderInfo_t &pInfo, VeriSDK::matrix3x4_t *pCustomBoneToWorld);

// had to put at the bottom for BHop and other cheats
VeriSDK::Panel* panel;
VeriSDK::Engine* engine;
VeriSDK::BaseClientDLL* baseClientDLL;
VeriSDK::ClientMode* clientMode;
VeriSDK::ModelInfo* modelInfo;
VeriSDK::Surface* surface;
VeriSDK::DebugOverlay* dbgOverlay;
VeriSDK::ModelRender* mdlRender;
VeriSDK::RenderView* renderView;
VeriSDK::MatSys* matSys;
VeriSDK::CVar* cVar;

void ForceMaterial(VeriSDK::Material* material, Color color)
{
	if (material != NULL)
	{
		renderView->SetColorModulation(color.Base());
		mdlRender->ForcedMaterialOverride(material);
	}
}
