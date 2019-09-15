The Cakewalk DirectX Plug-In Wizard has created
the following files for you:

SynthPlugIn.h, SynthPlugIn.cpp:
    CSynthPlugIn, the DirectX plug-in object.

SynthPlugInPropPage.h, SynthPlugInPropPage.cpp:
    CSynthPlugInPropPage, an class that implements the plug-in's
    property page (IPropertyPage).

SynthPlugIn.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Developer Studio.

res\SynthPlugIn.rc2
    This file contains resources that are not edited by Microsoft 
    Developer Studio.  You should place all resources not
    editable by the resource editor in this file.

SynthPlugIn.def
    This file contains information about the DLL that must be
    provided to run with Microsoft Windows.  It defines parameters
    such as the name and description of the DLL.  It also exports
	functions from the DLL.

SynthPlugIn.clw
    This file contains information used by ClassWizard to edit existing
    classes or add new classes.  ClassWizard also uses this file to store
    information needed to create and edit message maps and dialog data
    maps and to create prototype member functions.

///////////////////////////////////////////////////////////
Support files:

SynthPlugInApp.h
SynthPlugInApp.cpp:
    Entry points for component registration and deregistration.

MediaParams.h
MediaParams.cpp:
    CMediaParams, a helper class to implement all pertinent DirectX automation
    intefaces, such as IMediaParams and IMediaParamsInfo.

ParamEnvelope.h
ParamEnvelope.cpp:
    CParamEnvelope, a container for a single parameter's envelope, i.e., its
    evolving shape over time.  CMediaParams keeps a collection of these.

MfxTime.h
MidiFilter.h
    Declarations of the Cakewalk MIDI Effects (MFX) COM classes.

DeferZeroFill.h
	Declaration of IDeferZeroFill, a custom interface provided by
	media samples delivered in SONAR.

SoftSynth.h
SoftSynth.cpp
	Wrapper class for MFX soft synth COM object.

StringMap.h
StringMap.cpp
	Helper class for IMfxStringMap, reference counted string maps.

Instrument.h
Instrument.cpp
	Helper class for IMfxInstrument, instrument definitions.

///////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named SynthPlugIn.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Developer Studio reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
