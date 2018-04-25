#pragma once

#include "BSShader.h"
#include "BSShaderAccumulator.h"

#define BAD_SHADER 0x11223344

#define BSSM_AMBIENT_OCCLUSION 0x2

#define BSSM_GRASS_SHADOW_L 0x5C000058
#define BSSM_GRASS_SHADOW_LS 0x5C000059
#define BSSM_GRASS_SHADOW_LB 0x5C00005A
#define BSSM_GRASS_SHADOW_LSB 0x5C00005B
#define BSSM_WATER_STENCIL 0x5C00006D
#define BSSM_WATER_DISPLACEMENT_STENCIL_Vc 0x5C000070

#define BSSM_BLOOD_SPLATTER_FLARE 0x5C006073
#define BSSM_BLOOD_SPLATTER 0x5C006074

#define BSSM_DISTANTTREE 0x5C00002E
#define BSSM_DISTANTTREE_DEPTH 0x5C00002F

#define BSSM_GRASS_DIRONLY_LF 0x5C000030
#define BSSM_GRASS_DIRONLY_LV 0x5C000031
#define BSSM_GRASS_DIRONLY_LFS 0x5C000032
#define BSSM_GRASS_DIRONLY_LVS 0x5C000033
#define BSSM_GRASS_DIRONLY_LFB 0x5C000040
#define BSSM_GRASS_DIRONLY_LFSB 0x5C000041
#define BSSM_GRASS_NOALPHA_DIRONLY_LF 0x3
#define BSSM_GRASS_NOALPHA_DIRONLY_LFS 0x5
#define BSSM_GRASS_NOALPHA_DIRONLY_LVS 0x6
#define BSSM_GRASS_NOALPHA_DIRONLY_LFB 0x13
#define BSSM_GRASS_NOALPHA_DIRONLY_LFSB 0x14

#define BSSM_SKYBASEPRE 0x5C00005D
#define BSSM_SKY 0x5C00005E
#define BSSM_SKY_MOON_STARS_MASK 0x5C00005F
#define BSSM_SKY_STARS 0x5C000060
#define BSSM_SKY_TEXTURE 0x5C000061
#define BSSM_SKY_CLOUDS 0x5C000062
#define BSSM_SKY_CLOUDSLERP 0x5C000063
#define BSSM_SKY_CLOUDSFADE 0x5C000064
#define BSSM_SKY_SUNGLARE 0x5C006072

enum ShaderEnum
{
	BSSM_SHADER_INVALID = 0,
	BSSM_SHADER_RUNGRASS = 1,
	BSSM_SHADER_SKY = 2,
	BSSM_SHADER_WATER = 3,
	BSSM_SHADER_BLOODSPLATTER = 4,
	BSSM_SHADER_IMAGESPACE = 5,
	BSSM_SHADER_LIGHTING = 6,
	BSSM_SHADER_EFFECT = 7,
	BSSM_SHADER_UTILITY = 8,
	BSSM_SHADER_DISTANTTREE = 9,
	BSSM_SHADER_PARTICLE = 10,
	BSSM_SHADER_COUNT = 11,
};

enum class BSSM_SHADER_TYPE
{
	VERTEX,		// BSVertexShader
	PIXEL,		// BSPixelShader
	COMPUTE,	// BSComputeShader
};

enum class BSSM_GROUP_TYPE
{
	PER_GEO,	// PerGeometry
	PER_MAT,	// PerMaterial
	PER_TEC,	// PerTechnique
};

struct BSConstantGroup
{
	ID3D11Buffer *m_Buffer;	// Selected from pool in Load*ShaderFromFile()
	void *m_Data;			// m_ConstantBufferData = DeviceContext->Map(m_ConstantBuffer)

	// Based on shader load flags, these **CAN BE NULL**. At least one of the
	// pointers is guaranteed to be non-null.
};

void /*BSShaderManager::*/SetCurrentAccumulator(BSShaderAccumulator *Accumulator);
void ClearShaderAndTechnique();
bool SetupShaderAndTechnique(BSShader *Shader, uint32_t Technique);

#include "../BSRenderPass.h"
#include "BSShaderAccumulator.h"

namespace BSShaderMappings { struct Entry; }

class ShaderDecoder
{
protected:
	struct ParamIndexPair
	{
		int Index;
		const char *Name;
		const struct BSShaderMappings::Entry *Remap;
	};

	void *m_HlslData;
	size_t m_HlslDataLen;

	char m_Type[256];
	BSSM_SHADER_TYPE m_CodeType;

public:
	ShaderDecoder(const char *Type, BSSM_SHADER_TYPE CodeType);
	~ShaderDecoder();

	void SetShaderData(void *Buffer, size_t BufferSize);
	void DumpShader();

protected:
	void DumpShaderInfo();
	void DumpCBuffer(FILE *File, BSConstantGroup *Buffer, std::vector<ParamIndexPair> Params, int GroupIndex);

	virtual uint32_t GetTechnique() = 0;
	virtual const uint8_t *GetConstantArray() = 0;
	virtual size_t GetConstantArraySize() = 0;
	virtual void DumpShaderSpecific(const char *TechName, std::vector<ParamIndexPair>& PerGeo, std::vector<ParamIndexPair>& PerMat, std::vector<ParamIndexPair>& PerTec, std::vector<ParamIndexPair>& Undefined) = 0;

	const char *GetGroupName(int Index);
	const char *GetGroupRegister(int Index);
	const char *GetConstantName(int Index);
	void GetTechniqueName(char *Buffer, size_t BufferSize, uint32_t Technique);
	const char *GetSamplerName(int Index, uint32_t Technique);
	std::vector<std::pair<const char *, const char *>> GetDefineArray(uint32_t Technique);
};

//
// *** SHADER CONSTANTS ***
//
// Everything ripped directly from CreationKit.exe.
//
// GetString() returns the name used in the HLSL source code.
// GetSize() is multiplied by sizeof(float) to get the real CBuffer size.
//
// NOTE: In the creation kit, there are copy-paste errors with the placeholder
// saying "BSLightingShaderX" instead of the real function name.
//
// NOTE: BSXShader/BSLightingShader/BSUtilityShader have variable-size constant types
//
#pragma push_macro("TEST_BIT")
#undef TEST_BIT
#define TEST_BIT(index) (Technique & (1u << (index)))

#define DO_ALPHA_TEST_FLAG 0x10000
#define BSSM_PLACEHOLDER "Add-your-constant-to-" __FUNCTION__

namespace BSShaderMappings
{
	struct Entry
	{
		const char *Type;
		BSSM_GROUP_TYPE Group;
		int Index;
		const char *ParamTypeOverride;
	};

	const Entry Vertex[] =
	{
#define REMAP_VERTEX_UNUSED(ShaderType, GroupType)
#define REMAP_VERTEX(ShaderType, GroupType, ParameterIndex, ParamType) { ShaderType, (BSSM_GROUP_TYPE)GroupType, ParameterIndex, ParamType },
#include "BSShaderConstants.inl"
	};

	const Entry Pixel[] =
	{
#define REMAP_PIXEL_UNUSED(ShaderType, GroupType)
#define REMAP_PIXEL(ShaderType, GroupType, ParameterIndex, ParamType) { ShaderType, (BSSM_GROUP_TYPE)GroupType, ParameterIndex, ParamType },
#include "BSShaderConstants.inl"
	};

	static const Entry *GetEntryForVertexShader(const char *Type, int ParamIndex)
	{
		for (int i = 0; i < ARRAYSIZE(Vertex); i++)
		{
			if (_stricmp(Vertex[i].Type, Type) != 0)
				continue;

			if (Vertex[i].Index != ParamIndex)
				continue;

			return &Vertex[i];
		}

		return nullptr;
	}

	static const Entry *GetEntryForPixelShader(const char *Type, int ParamIndex)
	{
		for (int i = 0; i < ARRAYSIZE(Pixel); i++)
		{
			if (_stricmp(Pixel[i].Type, Type) != 0)
				continue;

			if (Pixel[i].Index != ParamIndex)
				continue;

			return &Pixel[i];
		}

		return nullptr;
	}
}

namespace BSShaderInfo
{
#include "BSShaderManagerInfo.inl"
}

#undef TEST_BIT
#pragma pop_macro("TEST_BIT")