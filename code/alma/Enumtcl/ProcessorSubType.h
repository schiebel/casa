/*
 *
 * /////////////////////////////////////////////////////////////////
 * // WARNING!  DO NOT MODIFY THIS FILE!                          //
 * //  ---------------------------------------------------------  //
 * // | This is generated code using a C++ template function!   | //
 * // ! Do not modify this file.                                | //
 * // | Any changes will be lost when the file is re-generated. | //
 * //  ---------------------------------------------------------  //
 * /////////////////////////////////////////////////////////////////
 *
 */


#if     !defined(_PROCESSORSUBTYPE_H)

#include <CProcessorSubType.h>
#define _PROCESSORSUBTYPE_H
#endif 

#if     !defined(_PROCESSORSUBTYPE_HH)

#include "Enum.hpp"

using namespace ProcessorSubTypeMod;

template<>
 struct enum_set_traits<ProcessorSubTypeMod::ProcessorSubType> : public enum_set_traiter<ProcessorSubTypeMod::ProcessorSubType,4,ProcessorSubTypeMod::ALMA_RADIOMETER> {};

template<>
class enum_map_traits<ProcessorSubTypeMod::ProcessorSubType,void> : public enum_map_traiter<ProcessorSubTypeMod::ProcessorSubType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<ProcessorSubTypeMod::ProcessorSubType,EnumPar<void> >
     (ProcessorSubTypeMod::ALMA_CORRELATOR_MODE,ep((int)ProcessorSubTypeMod::ALMA_CORRELATOR_MODE,"ALMA_CORRELATOR_MODE","un-documented")));
    m_.insert(pair<ProcessorSubTypeMod::ProcessorSubType,EnumPar<void> >
     (ProcessorSubTypeMod::SQUARE_LAW_DETECTOR,ep((int)ProcessorSubTypeMod::SQUARE_LAW_DETECTOR,"SQUARE_LAW_DETECTOR","un-documented")));
    m_.insert(pair<ProcessorSubTypeMod::ProcessorSubType,EnumPar<void> >
     (ProcessorSubTypeMod::HOLOGRAPHY,ep((int)ProcessorSubTypeMod::HOLOGRAPHY,"HOLOGRAPHY","un-documented")));
    m_.insert(pair<ProcessorSubTypeMod::ProcessorSubType,EnumPar<void> >
     (ProcessorSubTypeMod::ALMA_RADIOMETER,ep((int)ProcessorSubTypeMod::ALMA_RADIOMETER,"ALMA_RADIOMETER","un-documented")));
    return true;
  }
  static map<ProcessorSubTypeMod::ProcessorSubType,EnumPar<void> > m_;
};
#define _PROCESSORSUBTYPE_HH
#endif
