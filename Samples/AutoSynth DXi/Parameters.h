// Declarations of automated parameters used by the plug-in

////////////////////////////////////////////////////////////////////////////////
#ifndef DEFINE_PARAM_INFO
////////////////////////////////////////////////////////////////////////////////

enum
{
	PARAM_ENABLE,
	PARAM_FILTER_CUTOFF,
	PARAM_FILTER_RESONANCE,
	NUM_AUTOMATED_PARAMS,

	// Internal parameters
	_PARAM_MODE						= NUM_AUTOMATED_PARAMS,
	_PARAM_MULTI_OUT,

	NUM_PARAMS
};

////////////////////////////////////////////////////////////////////////////////
#else
////////////////////////////////////////////////////////////////////////////////

#define MP_NONE	(0)
#define MP_JUMP	(MP_CURVE_JUMP)
#define MP_LINES	(MP_CURVE_JUMP|MP_CURVE_LINEAR)
#define MP_QUADS	(MP_CURVE_JUMP|MP_CURVE_LINEAR|MP_CURVE_SQUARE|MP_CURVE_INVSQUARE)
#define MP_ALL		(MP_CURVE_JUMP|MP_CURVE_LINEAR|MP_CURVE_SQUARE|MP_CURVE_INVSQUARE|MP_CURVE_SINE)

const ParamInfo CMediaParams::m_aParamInfo[ NUM_PARAMS ] =
{
//	MP_TYPE		MP_CAPS		min	max		def	units		label				int.min	int.max	"Enum1,Enum2,.."
//	-------		-------		---	---		---	-----		-----				-------	-------	---------------
{	MPT_BOOL,	MP_QUADS,	0,		1,			1,		L"",		L"Enabled",		0,			1,			NULL	},
{	MPT_FLOAT,	MP_QUADS,	0,		1,			0.5,	L"Fs/2",	L"Cutoff",		0,			1,			NULL	},
{	MPT_FLOAT,	MP_QUADS,	0,		4,			1,		L"Q",		L"Resonance",	0,			4,			NULL	},

{	MPT_ENUM,	MP_NONE,		0,		1,			0,		L"",		L"Mode",			0,			1,			L"Synth,Xform"	},
{	MPT_BOOL,	MP_NONE,		0,		1,			1,		L"",		L"MultiOut",	0,			1,			NULL	},
};

////////////////////////////////////////////////////////////////////////////////
#endif // DEFINE_PARAM_INFO
////////////////////////////////////////////////////////////////////////////////
