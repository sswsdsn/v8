// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_SHARED_FUNCTION_INFO_INL_H_
#define V8_OBJECTS_SHARED_FUNCTION_INFO_INL_H_

#include "src/heap/heap-inl.h"
#include "src/objects/debug-objects-inl.h"
#include "src/objects/scope-info.h"
#include "src/objects/shared-function-info.h"
#include "src/objects/templates.h"
#include "src/wasm/wasm-objects-inl.h"

// Has to be the last include (doesn't have include guards):
#include "src/objects/object-macros.h"

namespace v8 {
namespace internal {

CAST_ACCESSOR(PreParsedScopeData)
ACCESSORS(PreParsedScopeData, scope_data, PodArray<uint8_t>, kScopeDataOffset)
INT_ACCESSORS(PreParsedScopeData, length, kLengthOffset)

Object* PreParsedScopeData::child_data(int index) const {
  DCHECK_GE(index, 0);
  DCHECK_LT(index, this->length());
  int offset = kChildDataStartOffset + index * kPointerSize;
  return RELAXED_READ_FIELD(this, offset);
}

void PreParsedScopeData::set_child_data(int index, Object* value,
                                        WriteBarrierMode mode) {
  DCHECK_GE(index, 0);
  DCHECK_LT(index, this->length());
  int offset = kChildDataStartOffset + index * kPointerSize;
  RELAXED_WRITE_FIELD(this, offset, value);
  CONDITIONAL_WRITE_BARRIER(Heap::FromWritableHeapObject(this), this, offset,
                            value, mode);
}

Object** PreParsedScopeData::child_data_start() const {
  return HeapObject::RawField(this, kChildDataStartOffset);
}

CAST_ACCESSOR(UncompiledData)
INT32_ACCESSORS(UncompiledData, start_position, kStartPositionOffset)
INT32_ACCESSORS(UncompiledData, end_position, kEndPositionOffset)

CAST_ACCESSOR(UncompiledDataWithoutPreParsedScope)

CAST_ACCESSOR(UncompiledDataWithPreParsedScope)
ACCESSORS(UncompiledDataWithPreParsedScope, pre_parsed_scope_data,
          PreParsedScopeData, kPreParsedScopeDataOffset)

CAST_ACCESSOR(InterpreterData)
ACCESSORS(InterpreterData, bytecode_array, BytecodeArray, kBytecodeArrayOffset)
ACCESSORS(InterpreterData, interpreter_trampoline, Code,
          kInterpreterTrampolineOffset)

CAST_ACCESSOR(SharedFunctionInfo)
DEFINE_DEOPT_ELEMENT_ACCESSORS(SharedFunctionInfo, Object)

ACCESSORS(SharedFunctionInfo, name_or_scope_info, Object,
          kNameOrScopeInfoOffset)
ACCESSORS(SharedFunctionInfo, function_data, Object, kFunctionDataOffset)
ACCESSORS(SharedFunctionInfo, script, Object, kScriptOffset)
ACCESSORS(SharedFunctionInfo, function_identifier_or_debug_info, Object,
          kFunctionIdentifierOrDebugInfoOffset)

INT_ACCESSORS(SharedFunctionInfo, function_literal_id, kFunctionLiteralIdOffset)
#if V8_SFI_HAS_UNIQUE_ID
INT_ACCESSORS(SharedFunctionInfo, unique_id, kUniqueIdOffset)
#endif
UINT16_ACCESSORS(SharedFunctionInfo, length, kLengthOffset)
UINT16_ACCESSORS(SharedFunctionInfo, internal_formal_parameter_count,
                 kFormalParameterCountOffset)
UINT16_ACCESSORS(SharedFunctionInfo, expected_nof_properties,
                 kExpectedNofPropertiesOffset)
UINT16_ACCESSORS(SharedFunctionInfo, raw_function_token_offset,
                 kFunctionTokenOffsetOffset)
INT_ACCESSORS(SharedFunctionInfo, flags, kFlagsOffset)

bool SharedFunctionInfo::HasSharedName() const {
  Object* value = name_or_scope_info();
  if (value->IsScopeInfo()) {
    return ScopeInfo::cast(value)->HasSharedFunctionName();
  }
  return value != kNoSharedNameSentinel;
}

String* SharedFunctionInfo::Name() const {
  if (!HasSharedName()) return GetReadOnlyRoots().empty_string();
  Object* value = name_or_scope_info();
  if (value->IsScopeInfo()) {
    if (ScopeInfo::cast(value)->HasFunctionName()) {
      return String::cast(ScopeInfo::cast(value)->FunctionName());
    }
    return GetReadOnlyRoots().empty_string();
  }
  return String::cast(value);
}

void SharedFunctionInfo::SetName(String* name) {
  Object* maybe_scope_info = name_or_scope_info();
  if (maybe_scope_info->IsScopeInfo()) {
    ScopeInfo::cast(maybe_scope_info)->SetFunctionName(name);
  } else {
    DCHECK(maybe_scope_info->IsString() ||
           maybe_scope_info == kNoSharedNameSentinel);
    set_name_or_scope_info(name);
  }
  UpdateFunctionMapIndex();
}

AbstractCode* SharedFunctionInfo::abstract_code() {
  if (HasBytecodeArray()) {
    return AbstractCode::cast(GetBytecodeArray());
  } else {
    return AbstractCode::cast(GetCode());
  }
}

int SharedFunctionInfo::function_token_position() const {
  int offset = raw_function_token_offset();
  if (offset == kFunctionTokenOutOfRange) {
    return kNoSourcePosition;
  } else {
    return StartPosition() - offset;
  }
}

BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, is_wrapped,
                    SharedFunctionInfo::IsWrappedBit)
BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, allows_lazy_compilation,
                    SharedFunctionInfo::AllowLazyCompilationBit)
BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, has_duplicate_parameters,
                    SharedFunctionInfo::HasDuplicateParametersBit)
BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, is_declaration,
                    SharedFunctionInfo::IsDeclarationBit)

BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, native,
                    SharedFunctionInfo::IsNativeBit)
BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, is_asm_wasm_broken,
                    SharedFunctionInfo::IsAsmWasmBrokenBit)
BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags,
                    requires_instance_fields_initializer,
                    SharedFunctionInfo::RequiresInstanceFieldsInitializer)

BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, name_should_print_as_anonymous,
                    SharedFunctionInfo::NameShouldPrintAsAnonymousBit)
BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, is_anonymous_expression,
                    SharedFunctionInfo::IsAnonymousExpressionBit)
BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, deserialized,
                    SharedFunctionInfo::IsDeserializedBit)
BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, has_reported_binary_coverage,
                    SharedFunctionInfo::HasReportedBinaryCoverageBit)

BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, is_named_expression,
                    SharedFunctionInfo::IsNamedExpressionBit)
BIT_FIELD_ACCESSORS(SharedFunctionInfo, flags, is_toplevel,
                    SharedFunctionInfo::IsTopLevelBit)

bool SharedFunctionInfo::optimization_disabled() const {
  return disable_optimization_reason() != BailoutReason::kNoReason;
}

BailoutReason SharedFunctionInfo::disable_optimization_reason() const {
  return DisabledOptimizationReasonBits::decode(flags());
}

LanguageMode SharedFunctionInfo::language_mode() {
  STATIC_ASSERT(LanguageModeSize == 2);
  return construct_language_mode(IsStrictBit::decode(flags()));
}

void SharedFunctionInfo::set_language_mode(LanguageMode language_mode) {
  STATIC_ASSERT(LanguageModeSize == 2);
  // We only allow language mode transitions that set the same language mode
  // again or go up in the chain:
  DCHECK(is_sloppy(this->language_mode()) || is_strict(language_mode));
  int hints = flags();
  hints = IsStrictBit::update(hints, is_strict(language_mode));
  set_flags(hints);
  UpdateFunctionMapIndex();
}

FunctionKind SharedFunctionInfo::kind() const {
  return FunctionKindBits::decode(flags());
}

void SharedFunctionInfo::set_kind(FunctionKind kind) {
  int hints = flags();
  hints = FunctionKindBits::update(hints, kind);
  hints = IsClassConstructorBit::update(hints, IsClassConstructor(kind));
  hints = IsDerivedConstructorBit::update(hints, IsDerivedConstructor(kind));
  set_flags(hints);
  UpdateFunctionMapIndex();
}

bool SharedFunctionInfo::needs_home_object() const {
  return NeedsHomeObjectBit::decode(flags());
}

void SharedFunctionInfo::set_needs_home_object(bool value) {
  int hints = flags();
  hints = NeedsHomeObjectBit::update(hints, value);
  set_flags(hints);
  UpdateFunctionMapIndex();
}

bool SharedFunctionInfo::construct_as_builtin() const {
  return ConstructAsBuiltinBit::decode(flags());
}

void SharedFunctionInfo::CalculateConstructAsBuiltin() {
  bool uses_builtins_construct_stub = false;
  if (HasBuiltinId()) {
    int id = builtin_id();
    if (id != Builtins::kCompileLazy && id != Builtins::kEmptyFunction) {
      uses_builtins_construct_stub = true;
    }
  } else if (IsApiFunction()) {
    uses_builtins_construct_stub = true;
  }

  int f = flags();
  f = ConstructAsBuiltinBit::update(f, uses_builtins_construct_stub);
  set_flags(f);
}

int SharedFunctionInfo::function_map_index() const {
  // Note: Must be kept in sync with the FastNewClosure builtin.
  int index =
      Context::FIRST_FUNCTION_MAP_INDEX + FunctionMapIndexBits::decode(flags());
  DCHECK_LE(index, Context::LAST_FUNCTION_MAP_INDEX);
  return index;
}

void SharedFunctionInfo::set_function_map_index(int index) {
  STATIC_ASSERT(Context::LAST_FUNCTION_MAP_INDEX <=
                Context::FIRST_FUNCTION_MAP_INDEX + FunctionMapIndexBits::kMax);
  DCHECK_LE(Context::FIRST_FUNCTION_MAP_INDEX, index);
  DCHECK_LE(index, Context::LAST_FUNCTION_MAP_INDEX);
  index -= Context::FIRST_FUNCTION_MAP_INDEX;
  set_flags(FunctionMapIndexBits::update(flags(), index));
}

void SharedFunctionInfo::clear_padding() {
  memset(reinterpret_cast<void*>(this->address() + kSize), 0,
         kAlignedSize - kSize);
}

void SharedFunctionInfo::UpdateFunctionMapIndex() {
  int map_index = Context::FunctionMapIndex(
      language_mode(), kind(), true, HasSharedName(), needs_home_object());
  set_function_map_index(map_index);
}

void SharedFunctionInfo::DontAdaptArguments() {
  // TODO(leszeks): Revise this DCHECK now that the code field is gone.
  DCHECK(!HasWasmExportedFunctionData());
  set_internal_formal_parameter_count(kDontAdaptArgumentsSentinel);
}

int SharedFunctionInfo::StartPosition() const {
  Object* maybe_scope_info = name_or_scope_info();
  if (maybe_scope_info->IsScopeInfo()) {
    ScopeInfo* info = ScopeInfo::cast(maybe_scope_info);
    if (info->HasPositionInfo()) {
      return info->StartPosition();
    }
  } else if (HasUncompiledData()) {
    // Works with or without scope.
    return uncompiled_data()->start_position();
  } else if (IsApiFunction() || HasBuiltinId()) {
    DCHECK_IMPLIES(HasBuiltinId(), builtin_id() != Builtins::kCompileLazy);
    return 0;
  }
  return kNoSourcePosition;
}

int SharedFunctionInfo::EndPosition() const {
  Object* maybe_scope_info = name_or_scope_info();
  if (maybe_scope_info->IsScopeInfo()) {
    ScopeInfo* info = ScopeInfo::cast(maybe_scope_info);
    if (info->HasPositionInfo()) {
      return info->EndPosition();
    }
  } else if (HasUncompiledData()) {
    // Works with or without scope.
    return uncompiled_data()->end_position();
  } else if (IsApiFunction() || HasBuiltinId()) {
    DCHECK_IMPLIES(HasBuiltinId(), builtin_id() != Builtins::kCompileLazy);
    return 0;
  }
  return kNoSourcePosition;
}

void SharedFunctionInfo::SetPosition(int start_position, int end_position) {
  Object* maybe_scope_info = name_or_scope_info();
  if (maybe_scope_info->IsScopeInfo()) {
    ScopeInfo* info = ScopeInfo::cast(maybe_scope_info);
    if (info->HasPositionInfo()) {
      info->SetPositionInfo(start_position, end_position);
    }
  } else if (HasUncompiledData()) {
    if (HasUncompiledDataWithPreParsedScope()) {
      // Clear out preparsed scope data, since the position setter invalidates
      // any scope data.
      ClearPreParsedScopeData();
    }
    uncompiled_data()->set_start_position(start_position);
    uncompiled_data()->set_end_position(end_position);
  } else {
    UNREACHABLE();
  }
}

Code* SharedFunctionInfo::GetCode() const {
  // ======
  // NOTE: This chain of checks MUST be kept in sync with the equivalent CSA
  // GetSharedFunctionInfoCode method in code-stub-assembler.cc.
  // ======

  Isolate* isolate = GetIsolate();
  Object* data = function_data();
  if (data->IsSmi()) {
    // Holding a Smi means we are a builtin.
    DCHECK(HasBuiltinId());
    return isolate->builtins()->builtin(builtin_id());
  } else if (data->IsBytecodeArray()) {
    // Having a bytecode array means we are a compiled, interpreted function.
    DCHECK(HasBytecodeArray());
    return isolate->builtins()->builtin(Builtins::kInterpreterEntryTrampoline);
  } else if (data->IsFixedArray()) {
    // Having a fixed array means we are an asm.js/wasm function.
    DCHECK(HasAsmWasmData());
    return isolate->builtins()->builtin(Builtins::kInstantiateAsmJs);
  } else if (data->IsUncompiledData()) {
    // Having uncompiled data (with or without scope) means we need to compile.
    DCHECK(HasUncompiledData());
    return isolate->builtins()->builtin(Builtins::kCompileLazy);
  } else if (data->IsFunctionTemplateInfo()) {
    // Having a function template info means we are an API function.
    DCHECK(IsApiFunction());
    return isolate->builtins()->builtin(Builtins::kHandleApiCall);
  } else if (data->IsWasmExportedFunctionData()) {
    // Having a WasmExportedFunctionData means the code is in there.
    DCHECK(HasWasmExportedFunctionData());
    return wasm_exported_function_data()->wrapper_code();
  } else if (data->IsInterpreterData()) {
    Code* code = InterpreterTrampoline();
    DCHECK(code->IsCode());
    DCHECK(code->is_interpreter_trampoline_builtin());
    return code;
  }
  UNREACHABLE();
}

bool SharedFunctionInfo::IsInterpreted() const { return HasBytecodeArray(); }

ScopeInfo* SharedFunctionInfo::scope_info() const {
  Object* maybe_scope_info = name_or_scope_info();
  if (maybe_scope_info->IsScopeInfo()) {
    return ScopeInfo::cast(maybe_scope_info);
  }
  return ScopeInfo::Empty(GetIsolate());
}

void SharedFunctionInfo::set_scope_info(ScopeInfo* scope_info,
                                        WriteBarrierMode mode) {
  // Move the existing name onto the ScopeInfo.
  Object* name = name_or_scope_info();
  if (name->IsScopeInfo()) {
    name = ScopeInfo::cast(name)->FunctionName();
  }
  DCHECK(name->IsString() || name == kNoSharedNameSentinel);
  // Only set the function name for function scopes.
  scope_info->SetFunctionName(name);
  if (HasInferredName() && inferred_name()->length() != 0) {
    scope_info->SetInferredFunctionName(inferred_name());
  }
  WRITE_FIELD(this, kNameOrScopeInfoOffset,
              reinterpret_cast<Object*>(scope_info));
  CONDITIONAL_WRITE_BARRIER(GetHeap(), this, kNameOrScopeInfoOffset,
                            reinterpret_cast<Object*>(scope_info), mode);
}

ACCESSORS(SharedFunctionInfo, raw_outer_scope_info_or_feedback_metadata,
          HeapObject, kOuterScopeInfoOrFeedbackMetadataOffset)

HeapObject* SharedFunctionInfo::outer_scope_info() const {
  DCHECK(!is_compiled());
  DCHECK(!HasFeedbackMetadata());
  return raw_outer_scope_info_or_feedback_metadata();
}

bool SharedFunctionInfo::HasOuterScopeInfo() const {
  ScopeInfo* outer_info = nullptr;
  if (!is_compiled()) {
    if (!outer_scope_info()->IsScopeInfo()) return false;
    outer_info = ScopeInfo::cast(outer_scope_info());
  } else {
    if (!scope_info()->HasOuterScopeInfo()) return false;
    outer_info = scope_info()->OuterScopeInfo();
  }
  return outer_info->length() > 0;
}

ScopeInfo* SharedFunctionInfo::GetOuterScopeInfo() const {
  DCHECK(HasOuterScopeInfo());
  if (!is_compiled()) return ScopeInfo::cast(outer_scope_info());
  return scope_info()->OuterScopeInfo();
}

void SharedFunctionInfo::set_outer_scope_info(HeapObject* value,
                                              WriteBarrierMode mode) {
  DCHECK(!is_compiled());
  DCHECK(raw_outer_scope_info_or_feedback_metadata()->IsTheHole());
  DCHECK(value->IsScopeInfo() || value->IsTheHole());
  return set_raw_outer_scope_info_or_feedback_metadata(value, mode);
}

bool SharedFunctionInfo::HasFeedbackMetadata() const {
  return raw_outer_scope_info_or_feedback_metadata()->IsFeedbackMetadata();
}

FeedbackMetadata* SharedFunctionInfo::feedback_metadata() const {
  DCHECK(HasFeedbackMetadata());
  return FeedbackMetadata::cast(raw_outer_scope_info_or_feedback_metadata());
}

void SharedFunctionInfo::set_feedback_metadata(FeedbackMetadata* value,
                                               WriteBarrierMode mode) {
  DCHECK(!HasFeedbackMetadata());
  DCHECK(value->IsFeedbackMetadata());
  return set_raw_outer_scope_info_or_feedback_metadata(value, mode);
}

bool SharedFunctionInfo::is_compiled() const {
  Object* data = function_data();
  return data != Smi::FromEnum(Builtins::kCompileLazy) &&
         !data->IsUncompiledData();
}

uint16_t SharedFunctionInfo::GetLength() const {
  DCHECK(is_compiled());
  DCHECK(HasLength());
  return length();
}

bool SharedFunctionInfo::HasLength() const {
  return length() != kInvalidLength;
}

bool SharedFunctionInfo::has_simple_parameters() {
  return scope_info()->HasSimpleParameters();
}

bool SharedFunctionInfo::IsApiFunction() const {
  return function_data()->IsFunctionTemplateInfo();
}

FunctionTemplateInfo* SharedFunctionInfo::get_api_func_data() {
  DCHECK(IsApiFunction());
  return FunctionTemplateInfo::cast(function_data());
}

bool SharedFunctionInfo::HasBytecodeArray() const {
  return function_data()->IsBytecodeArray() ||
         function_data()->IsInterpreterData();
}

BytecodeArray* SharedFunctionInfo::GetBytecodeArray() const {
  DCHECK(HasBytecodeArray());
  if (HasDebugInfo() && GetDebugInfo()->HasInstrumentedBytecodeArray()) {
    return GetDebugInfo()->OriginalBytecodeArray();
  } else if (function_data()->IsBytecodeArray()) {
    return BytecodeArray::cast(function_data());
  } else {
    DCHECK(function_data()->IsInterpreterData());
    return InterpreterData::cast(function_data())->bytecode_array();
  }
}

BytecodeArray* SharedFunctionInfo::GetDebugBytecodeArray() const {
  DCHECK(HasBytecodeArray());
  DCHECK(HasDebugInfo() && GetDebugInfo()->HasInstrumentedBytecodeArray());
  if (function_data()->IsBytecodeArray()) {
    return BytecodeArray::cast(function_data());
  } else {
    DCHECK(function_data()->IsInterpreterData());
    return InterpreterData::cast(function_data())->bytecode_array();
  }
}

void SharedFunctionInfo::SetDebugBytecodeArray(BytecodeArray* bytecode) {
  DCHECK(HasBytecodeArray());
  if (function_data()->IsBytecodeArray()) {
    set_function_data(bytecode);
  } else {
    DCHECK(function_data()->IsInterpreterData());
    interpreter_data()->set_bytecode_array(bytecode);
  }
}

void SharedFunctionInfo::set_bytecode_array(BytecodeArray* bytecode) {
  DCHECK(function_data() == Smi::FromEnum(Builtins::kCompileLazy) ||
         HasUncompiledData());
  set_function_data(bytecode);
}

Code* SharedFunctionInfo::InterpreterTrampoline() const {
  DCHECK(HasInterpreterData());
  return interpreter_data()->interpreter_trampoline();
}

bool SharedFunctionInfo::HasInterpreterData() const {
  return function_data()->IsInterpreterData();
}

InterpreterData* SharedFunctionInfo::interpreter_data() const {
  DCHECK(HasInterpreterData());
  return InterpreterData::cast(function_data());
}

void SharedFunctionInfo::set_interpreter_data(
    InterpreterData* interpreter_data) {
  DCHECK(FLAG_interpreted_frames_native_stack);
  set_function_data(interpreter_data);
}

bool SharedFunctionInfo::HasAsmWasmData() const {
  return function_data()->IsFixedArray();
}

FixedArray* SharedFunctionInfo::asm_wasm_data() const {
  DCHECK(HasAsmWasmData());
  return FixedArray::cast(function_data());
}

void SharedFunctionInfo::set_asm_wasm_data(FixedArray* data) {
  DCHECK(function_data() == Smi::FromEnum(Builtins::kCompileLazy) ||
         HasUncompiledData() || HasAsmWasmData());
  set_function_data(data);
}

bool SharedFunctionInfo::HasBuiltinId() const {
  return function_data()->IsSmi();
}

int SharedFunctionInfo::builtin_id() const {
  DCHECK(HasBuiltinId());
  int id = Smi::ToInt(function_data());
  DCHECK(Builtins::IsBuiltinId(id));
  return id;
}

void SharedFunctionInfo::set_builtin_id(int builtin_id) {
  DCHECK(Builtins::IsBuiltinId(builtin_id));
  DCHECK_NE(builtin_id, Builtins::kDeserializeLazy);
  set_function_data(Smi::FromInt(builtin_id), SKIP_WRITE_BARRIER);
}

bool SharedFunctionInfo::HasUncompiledData() const {
  return function_data()->IsUncompiledData();
}

UncompiledData* SharedFunctionInfo::uncompiled_data() const {
  DCHECK(HasUncompiledData());
  return UncompiledData::cast(function_data());
}

void SharedFunctionInfo::set_uncompiled_data(UncompiledData* uncompiled_data) {
  DCHECK(function_data() == Smi::FromEnum(Builtins::kCompileLazy));
  DCHECK(uncompiled_data->IsUncompiledData());
  set_function_data(uncompiled_data);
}

bool SharedFunctionInfo::HasUncompiledDataWithPreParsedScope() const {
  return function_data()->IsUncompiledDataWithPreParsedScope();
}

UncompiledDataWithPreParsedScope*
SharedFunctionInfo::uncompiled_data_with_pre_parsed_scope() const {
  DCHECK(HasUncompiledDataWithPreParsedScope());
  return UncompiledDataWithPreParsedScope::cast(function_data());
}

void SharedFunctionInfo::set_uncompiled_data_with_pre_parsed_scope(
    UncompiledDataWithPreParsedScope* uncompiled_data_with_pre_parsed_scope) {
  DCHECK(function_data() == Smi::FromEnum(Builtins::kCompileLazy));
  DCHECK(uncompiled_data_with_pre_parsed_scope
             ->IsUncompiledDataWithPreParsedScope());
  set_function_data(uncompiled_data_with_pre_parsed_scope);
}

bool SharedFunctionInfo::HasUncompiledDataWithoutPreParsedScope() const {
  return function_data()->IsUncompiledDataWithoutPreParsedScope();
}

void SharedFunctionInfo::ClearPreParsedScopeData() {
  DCHECK(HasUncompiledDataWithPreParsedScope());
  UncompiledDataWithPreParsedScope* data =
      uncompiled_data_with_pre_parsed_scope();

  // Trim off the pre-parsed scope data from the uncompiled data by swapping the
  // map, leaving only an uncompiled data without pre-parsed scope.
  DisallowHeapAllocation no_gc;
  Heap* heap = Heap::FromWritableHeapObject(data);

  // Swap the map.
  heap->NotifyObjectLayoutChange(data, UncompiledDataWithPreParsedScope::kSize,
                                 no_gc);
  STATIC_ASSERT(UncompiledDataWithoutPreParsedScope::kSize <
                UncompiledDataWithPreParsedScope::kSize);
  STATIC_ASSERT(UncompiledDataWithoutPreParsedScope::kSize ==
                UncompiledData::kSize);
  data->synchronized_set_map(
      GetReadOnlyRoots().uncompiled_data_without_pre_parsed_scope_map());

  // Fill the remaining space with filler.
  heap->CreateFillerObjectAt(
      data->address() + UncompiledDataWithoutPreParsedScope::kSize,
      UncompiledDataWithPreParsedScope::kSize -
          UncompiledDataWithoutPreParsedScope::kSize,
      ClearRecordedSlots::kNo);

  // Ensure that the clear was successful.
  DCHECK(HasUncompiledDataWithoutPreParsedScope());
}

bool SharedFunctionInfo::HasWasmExportedFunctionData() const {
  return function_data()->IsWasmExportedFunctionData();
}

WasmExportedFunctionData* SharedFunctionInfo::wasm_exported_function_data()
    const {
  DCHECK(HasWasmExportedFunctionData());
  return WasmExportedFunctionData::cast(function_data());
}

bool SharedFunctionInfo::HasDebugInfo() const {
  return function_identifier_or_debug_info()->IsDebugInfo();
}

DebugInfo* SharedFunctionInfo::GetDebugInfo() const {
  DCHECK(HasDebugInfo());
  return DebugInfo::cast(function_identifier_or_debug_info());
}

Object* SharedFunctionInfo::function_identifier() const {
  Object* result;
  if (HasDebugInfo()) {
    result = GetDebugInfo()->function_identifier();
  } else {
    result = function_identifier_or_debug_info();
  }
  DCHECK(result->IsSmi() || result->IsString() || result->IsUndefined());
  return result;
}

void SharedFunctionInfo::SetDebugInfo(DebugInfo* debug_info) {
  DCHECK(!HasDebugInfo());
  DCHECK_EQ(debug_info->function_identifier(),
            function_identifier_or_debug_info());
  set_function_identifier_or_debug_info(debug_info);
}

bool SharedFunctionInfo::HasBuiltinFunctionId() {
  return function_identifier()->IsSmi();
}

BuiltinFunctionId SharedFunctionInfo::builtin_function_id() {
  DCHECK(HasBuiltinFunctionId());
  return static_cast<BuiltinFunctionId>(Smi::ToInt(function_identifier()));
}

void SharedFunctionInfo::set_builtin_function_id(BuiltinFunctionId id) {
  DCHECK(!HasDebugInfo());
  set_function_identifier_or_debug_info(Smi::FromInt(id));
}

bool SharedFunctionInfo::HasInferredName() {
  return function_identifier()->IsString();
}

String* SharedFunctionInfo::inferred_name() {
  if (HasInferredName()) {
    return String::cast(function_identifier());
  }
  DCHECK(function_identifier()->IsUndefined() || HasBuiltinFunctionId());
  return GetReadOnlyRoots().empty_string();
}

void SharedFunctionInfo::set_inferred_name(String* inferred_name) {
  DCHECK(function_identifier_or_debug_info()->IsUndefined() ||
         HasInferredName() || HasDebugInfo());
  if (HasDebugInfo()) {
    GetDebugInfo()->set_function_identifier(inferred_name);
  } else {
    set_function_identifier_or_debug_info(inferred_name);
  }
}

bool SharedFunctionInfo::IsUserJavaScript() {
  Object* script_obj = script();
  if (script_obj->IsUndefined()) return false;
  Script* script = Script::cast(script_obj);
  return script->IsUserJavaScript();
}

bool SharedFunctionInfo::IsSubjectToDebugging() {
  return IsUserJavaScript() && !HasAsmWasmData();
}

bool SharedFunctionInfo::CanDiscardCompiled() const {
  bool can_decompile = (HasBytecodeArray() || HasAsmWasmData() ||
                        HasUncompiledDataWithPreParsedScope());
  return can_decompile;
}

// static
void SharedFunctionInfo::DiscardCompiled(
    Isolate* isolate, Handle<SharedFunctionInfo> shared_info) {
  DCHECK(shared_info->CanDiscardCompiled());

  int start_position = shared_info->StartPosition();
  int end_position = shared_info->EndPosition();

  if (shared_info->is_compiled()) {
    HeapObject* outer_scope_info;
    if (shared_info->scope_info()->HasOuterScopeInfo()) {
      outer_scope_info = shared_info->scope_info()->OuterScopeInfo();
    } else {
      outer_scope_info = ReadOnlyRoots(isolate).the_hole_value();
    }
    // Raw setter to avoid validity checks, since we're performing the unusual
    // task of decompiling.
    shared_info->set_raw_outer_scope_info_or_feedback_metadata(
        outer_scope_info);
  } else {
    DCHECK(shared_info->outer_scope_info()->IsScopeInfo() ||
           shared_info->outer_scope_info()->IsTheHole());
  }

  if (shared_info->HasUncompiledDataWithPreParsedScope()) {
    // If this is uncompiled data with a pre-parsed scope data, we can just
    // clear out the scope data and keep the uncompiled data.
    shared_info->ClearPreParsedScopeData();
  } else {
    // Create a new UncompiledData, without pre-parsed scope, and update the
    // function data to point to it. Use the raw function data setter to avoid
    // validity checks, since we're performing the unusual task of decompiling.
    Handle<UncompiledData> data =
        isolate->factory()->NewUncompiledDataWithoutPreParsedScope(
            start_position, end_position);
    shared_info->set_function_data(*data);
  }
}

}  // namespace internal
}  // namespace v8

#include "src/objects/object-macros-undef.h"

#endif  // V8_OBJECTS_SHARED_FUNCTION_INFO_INL_H_
