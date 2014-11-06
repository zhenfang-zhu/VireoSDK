/**
 
Copyright (c) 2014 National Instruments Corp.
 
This software is subject to the terms described in the LICENSE.TXT file
 
SDG
*/

/*! \file
    \brief  C entry points for core Vireo functions. Used when the runtime is built and loaded as a library
 */

#include "ExecutionContext.h"
#include "VirtualInstrument.h"
#include "TypeDefiner.h"
#include "EggShell.h"
#include "TDCodecLVFlat.h"

using namespace Vireo;

//------------------------------------------------------------
VIREO_EXPORT Int32 Vireo_Version()
{
    return 0x00010004; // TODO
}
//------------------------------------------------------------
VIREO_EXPORT void* EggShell_Create(EggShell* parent)
{
    return EggShell::Create(parent);
}
//------------------------------------------------------------
VIREO_EXPORT void EggShell_REPL(EggShell* pShell, const Utf8Char* commands, Int32 length)
{
    if (length == -1) {
        length = (Int32)strlen((const char*)commands);
    }
    SubString  comandBuffer(commands, commands + length);
    pShell->REPL(&comandBuffer);
}
//------------------------------------------------------------
VIREO_EXPORT Int32 EggShell_ExecuteSlices(EggShell* pShell, Int32 numSlices)
{
    return pShell->TheExecutionContext()->ExecuteSlices(numSlices, 20);
}
//------------------------------------------------------------
VIREO_EXPORT void EggShell_SetDelayedLoad(EggShell* pShell, bool value)
{
}
//------------------------------------------------------------
VIREO_EXPORT TypeRef EggShell_GetTypeList(EggShell* eggShell)
{
    return eggShell->TheTypeManager()->GetTypeList();
}
//------------------------------------------------------------
VIREO_EXPORT void EggShell_Delete(EggShell* pShell)
{
    if (pShell != null)
        pShell->Delete();
}
//------------------------------------------------------------
VIREO_EXPORT void ExecutionContext_EnqueueRunQueue(ExecutionContextRef pContext, VIClump* pClump)
{
    pContext->EnqueueRunQueue(pClump);
}
//------------------------------------------------------------
VIREO_EXPORT Int32 ExecutionContext_ExecuteSlices(ExecutionContextRef pContext, Int32 numSlices)
{
    return pContext->ExecuteSlices(numSlices, 1);
}
//------------------------------------------------------------
VirtualInstrument* EggShell_FindVI(EggShell* pShell, const char* viName)
{
    if (pShell == null)
        return null;
    
    SubString viNameString(viName);
    return (VirtualInstrument *) pShell->TheExecutionContext()->TheTypeManager()->FindNamedObject(&viNameString);
}
//------------------------------------------------------------
TypeRef EggShell_FindVIEltAddress(EggShell* pShell, const char* viName, const char* eltName, void** ppData)
{
    VirtualInstrument *vi  = EggShell_FindVI(pShell, viName);
    
    if (vi == null)
        return null;
    
    SubString eltNameString(eltName);
    return vi->GetVIElementAddressFromPath(&eltNameString, ppData);
}
//------------------------------------------------------------
VIREO_EXPORT Int32 EggShell_PeekMemory(EggShell* pShell, const char* viName, const char* eltName, Int32 bufferSize, char* buffer)
{
    memset(buffer, 0, bufferSize);
    
    void *pData = null;
    TypeRef actualType = EggShell_FindVIEltAddress(pShell, viName, eltName, &pData);
    if(actualType == null)
        return -1;

    ExecutionContextScope scope(pShell->TheExecutionContext());
    STACK_VAR(String, flatDataString);

    // Write flattened data to the string
    FlattenData(actualType, pData, flatDataString.Value, true);

    // Copy data to buffer
    Int32 flatDataSize = flatDataString.Value->Length();
    memcpy(buffer, flatDataString.Value->Begin(), Min(bufferSize, flatDataSize));

    return flatDataSize;
}
//------------------------------------------------------------
VIREO_EXPORT Int32 EggShell_PokeMemory(EggShell* pShell, const char* viName, const char* eltName, Int32 bufferSize, char* buffer)
{
    void *pData = null;
    TypeRef actualType = EggShell_FindVIEltAddress(pShell, viName, eltName, &pData);
    if(actualType == null)
        return -1;
    
    ExecutionContextScope scope(pShell->TheExecutionContext());
    SubBinaryBuffer subBuffer((UInt8*)buffer, (UInt8*)buffer+bufferSize);

    // Write unflattened data to the element
    if (UnflattenData(&subBuffer, true, 0, null, actualType, pData) == -1) {
        return -1;
    } else {
        return bufferSize;
    }
}
//------------------------------------------------------------
VIREO_EXPORT void EggShell_PokeDouble(EggShell* pShell, const char* viName, const char* eltName, Double d)
{
    void *pData = null;
    TypeRef actualType = EggShell_FindVIEltAddress(pShell, viName, eltName, &pData);
    if(actualType == null)
        return;
    
    WriteDoubleToMemory(actualType->BitEncoding(), actualType->TopAQSize(), pData, d);
}
//------------------------------------------------------------
VIREO_EXPORT Double EggShell_PeekDouble(EggShell* pShell, const char* viName, const char* eltName)
{
    void *pData = null;
    TypeRef actualType = EggShell_FindVIEltAddress(pShell, viName, eltName, &pData);
    if(actualType == null)
        return -1;

    Double d;
    ReadDoubleFromMemory(actualType->BitEncoding(), actualType->TopAQSize(), pData, &d);
    return d;
}
//------------------------------------------------------------
VIREO_EXPORT void Clump_DecrementFireCount(VIClump* clump)
{
    clump->Trigger();
}
//------------------------------------------------------------
VIREO_EXPORT TypeRef TypeManager_Define(TypeManagerRef typeManager, const char* typeName, const char* typeString)
{
    //   TypeManagerScope scope(typeManager);
    return TypeDefiner::Define(typeManager, typeName, typeString);
}
//------------------------------------------------------------
VIREO_EXPORT TypeRef TypeManager_FindType(TypeManagerRef typeManager, const char* typeName)
{
    SubString temp(typeName);
    return typeManager->FindType(&temp);
}
//------------------------------------------------------------
VIREO_EXPORT Int32 TypeRef_TopAQSize(TypeRef typeRef)
{
    return typeRef->TopAQSize();
}
//------------------------------------------------------------
VIREO_EXPORT Boolean TypeRef_IsFlat(TypeRef typeRef)
{
    return typeRef->IsFlat();
}
//------------------------------------------------------------
VIREO_EXPORT Boolean TypeRef_IsValid(TypeRef typeRef)
{
    return typeRef->IsValid();
}
//------------------------------------------------------------
VIREO_EXPORT Boolean TypeRef_HasCustomDefault(TypeRef typeRef)
{
    return typeRef->HasCustomDefault();
}
//------------------------------------------------------------
VIREO_EXPORT EncodingEnum TypeRef_BitEncoding(TypeRef typeRef)
{
    return typeRef->BitEncoding();
}
//------------------------------------------------------------
VIREO_EXPORT Int32 TypeRef_Alignment(TypeRef typeRef)
{
    return typeRef->AQAlignment();
}
//------------------------------------------------------------
VIREO_EXPORT void TypeRef_Name(TypeRef typeRef, Int32* bufferSize, char* buffer)
{
    SubString name;
    typeRef->GetName(&name);
    *bufferSize = name.CopyToBoundedBuffer(*bufferSize, (Utf8Char*)buffer);
}
//------------------------------------------------------------
VIREO_EXPORT void TypeRef_ElementName(TypeRef typeRef, Int32* bufferSize, char* buffer)
{
    SubString name;
    typeRef->GetElementName(&name);
    *bufferSize = name.CopyToBoundedBuffer(*bufferSize, (Utf8Char*)buffer);
}
//------------------------------------------------------------
VIREO_EXPORT Int32 TypeRef_ElementOffset(TypeRef typeRef)
{
    return typeRef->ElementOffset();
}
//------------------------------------------------------------
VIREO_EXPORT Int32 TypeRef_Rank(TypeRef typeRef)
{
    return typeRef->Rank();
}
//------------------------------------------------------------
VIREO_EXPORT PointerTypeEnum TypeRef_PointerType(TypeRef typeRef)
{
    return typeRef->PointerType();
}
//------------------------------------------------------------
VIREO_EXPORT TypeRef TypeRef_Next(TypeRef typeRef)
{
    return typeRef->Next();
}
//------------------------------------------------------------
VIREO_EXPORT UsageTypeEnum TypeRef_ElementUsageType(TypeRef typeRef)
{
    return typeRef->ElementUsageType();
}
//------------------------------------------------------------
VIREO_EXPORT Int32 TypeRef_SubElementCount(TypeRef typeRef)
{
    return typeRef->SubElementCount();
}
//------------------------------------------------------------
VIREO_EXPORT TypeRef TypeRef_GetSubElementByPath(TypeRef typeRef, char* buffer, Int32 *offset)
{
    SubString string(buffer);
    return typeRef->GetSubElementOffsetFromPath(&string, offset);
}
//------------------------------------------------------------
VIREO_EXPORT TypeRef TypeRef_GetSubElementByName(TypeRef typeRef, char* buffer)
{
    SubString string(buffer);
    return typeRef->GetSubElementByName(&string);
}
//------------------------------------------------------------
VIREO_EXPORT TypeRef TypeRef_GetSubElementByIndex(TypeRef typeRef, Int32 index)
{
    return typeRef->GetSubElement(index);
}
//------------------------------------------------------------
//------------------------------------------------------------
VIREO_EXPORT Int32 Data_RawBlockSize(TypedBlock* object)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    return object->AQBlockLength(object->Length());
}
//------------------------------------------------------------
VIREO_EXPORT Int32 Data_Length(TypedBlock* object)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    return object->Length();
}
//------------------------------------------------------------
VIREO_EXPORT TypeRef Data_Type(TypedBlock* object)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    return object->Type();
}
//------------------------------------------------------------
VIREO_EXPORT Int32 Data_GetLength(TypedBlock* object, Int32 dimension)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    return object->GetLength(dimension);
}
//------------------------------------------------------------
VIREO_EXPORT void Data_Resize1D(TypedBlock* object, Int32 size)
{
    VIREO_ASSERT(TypedBlock::ValidateHandle(object));
    object->Resize1D(size);
}
//------------------------------------------------------------
VIREO_EXPORT void Data_ResizeDimensions(TypedBlock* object, Int32 rank, IntIndex* sizes)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    object->ResizeDimensions(rank, sizes, false, false);
}
//------------------------------------------------------------
VIREO_EXPORT void* Data_RawPointerFromOffset(TypedBlock* object, Int32 offset)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    return object->RawBegin() + offset;
}
//------------------------------------------------------------
VIREO_EXPORT void Data_Read1Byte(TypedBlock* object, Int32 offset, Int8* value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *value = *(Int8*)object->BeginAtAQ(offset);
}
//------------------------------------------------------------
VIREO_EXPORT void Data_Write1Byte(TypedBlock* object, Int32 offset, Int8 value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *(Int8*)object->BeginAtAQ(offset) = value;
}
//------------------------------------------------------------
VIREO_EXPORT void Data_Read2Bytes(TypedBlock* object, Int32 offset, Int16* value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *value = *(Int16*)object->BeginAtAQ(offset);
}
//------------------------------------------------------------
VIREO_EXPORT void Data_Write2Bytes(TypedBlock* object, Int32 offset, Int16 value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *(Int16*)object->BeginAtAQ(offset) = value;
}
//------------------------------------------------------------
VIREO_EXPORT void Data_Read4Bytes(TypedBlock* object, Int32 offset, Int32* value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *value = *(Int32*)object->BeginAtAQ(offset);
}
//------------------------------------------------------------
VIREO_EXPORT void Data_Write4Bytes(TypedBlock* object, Int32 offset, Int32 value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *(Int32*)object->BeginAtAQ(offset) = value;
}
//------------------------------------------------------------
VIREO_EXPORT void Data_Read8Bytes(TypedBlock* object, Int32 offset, Int64* value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *value = *(Int64*)object->BeginAtAQ(offset);
}
//------------------------------------------------------------
VIREO_EXPORT void Data_Write8Bytes(TypedBlock* object, Int32 offset, Int64 value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *(Int64*)object->BeginAtAQ(offset) = value;
}
//------------------------------------------------------------
VIREO_EXPORT void Data_ReadPointer(TypedBlock* object, Int32 offset, void** value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *value = *(void**)object->BeginAtAQ(offset);
}
//------------------------------------------------------------
VIREO_EXPORT void Data_WritePointer(TypedBlock* object, Int32 offset, void* value)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    *(void**)object->BeginAtAQ(offset) = value;
}
//------------------------------------------------------------
VIREO_EXPORT void Data_ReadBytes(TypedBlock* object, Int32 offset, Int32 count, Int32* buffer)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    memcpy(buffer, object->BeginAtAQ(offset), count);
}
//------------------------------------------------------------
VIREO_EXPORT void Data_WriteBytes(TypedBlock* object, Int32 offset, Int32 count, Int32* buffer)
{
    VIREO_ASSERT( TypedBlock::ValidateHandle(object));
    memcpy(object->BeginAtAQ(offset), buffer, count);
}
