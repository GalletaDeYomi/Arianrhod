#include "edao.h"

NAKED VOID CBattle::NakedGetPredefinedMagicNumber()
{
    INLINE_ASM
    {
        mov     ecx, [ebp - 08h];
        mov     edx, [ebp + 08h];
        call    CBattle::GetPredefinedMagicNumber
        mov     [ebp - 20h], eax;
        ret;
    }
}

ULONG FASTCALL CBattle::GetPredefinedMagicNumber(PMONSTER_STATUS MSData)
{
    ULONG_PTR MagicDefined;
    PCRAFT_AI_INFO Magic;

    MagicDefined = 0;
    Magic = MSData->MagicAiInfo;

    for (ULONG_PTR Count = countof(MSData->MagicAiInfo); Count; ++Magic, --Count)
    {
        if (Magic->CraftIndex == 0)
            break;

        ++MagicDefined;
    }

    return MagicDefined;
}

NAKED VOID CBattle::OverWriteCraftWithChrCraft()
{
    INLINE_ASM
    {
        mov     dword ptr [ebp - 20h], 0x0;
        mov     ecx, [ebp - 08h];
        call    CBattle::GetActor
        mov     ecx, eax;
        call    CActor::GetPartyChipMap
        mov     edx, dword ptr [ebp - 14h];
        movzx   eax, word ptr [eax + edx * 2];
        mov     ecx, [esp];
        cmp     eax, MINIMUM_CUSTOM_CHAR_ID
        mov     edx, 0x9A37F4;
        cmovae  ecx, edx;
        mov     [esp], ecx;
        ret;
    }
}

NAKED VOID CBattle::NakedOverWriteBattleStatusWithChrStatus()
{
    INLINE_ASM
    {
        mov     ecx, [ebp - 08h];
        mov     edx, edi;
        push    eax;
        call    CBattle::OverWriteBattleStatusWithChrStatus
        ret;
    }
}

PMONSTER_STATUS FASTCALL CBattle::OverWriteBattleStatusWithChrStatus(PMONSTER_STATUS MSData, PCHAR_STATUS ChrStatus)
{
    MSData = PtrSub(MSData, FIELD_OFFSET(MONSTER_STATUS, ChrStatus[BattleStatusFinal].MaximumHP));

    PCHAR_STATUS Raw = &MSData->ChrStatus[BattleStatusRaw];
    PCHAR_STATUS Final = &MSData->ChrStatus[BattleStatusFinal];

    *Final = *ChrStatus;

    if (GetActor()->GetPartyChipMap()[MSData->CharID] < MINIMUM_CUSTOM_CHAR_ID)
        return MSData;

    Final->MaximumHP    += Raw->MaximumHP   / 2;
    Final->InitialHP    += Raw->InitialHP   / 2;
    Final->MaximumEP    += Raw->MaximumEP   / 2;
    Final->InitialEP    += Raw->InitialEP   / 2;
    Final->STR          += Raw->STR         * 2 / 3;
    Final->DEF          += Raw->DEF         * 2 / 3;
    Final->ATS          += Raw->ATS         * 2 / 3;
    Final->ADF          += Raw->ADF         * 2 / 3;
    Final->DEX          += Raw->DEX         * 2 / 3;
    Final->AGL          += Raw->AGL         * 2 / 3;
    Final->MOV          += Raw->MOV         * 2 / 3;
    Final->SPD          += Raw->SPD         * 2 / 3;

    return MSData;
}

NAKED VOID CBattle::NakedIsChrStatusNeedRefresh()
{
    INLINE_ASM
    {
        mov     ecx, [ebp - 0Ch];
        mov     edx, dword ptr [ebp - 8Ch];
        push    dword ptr [ebp - 25E8h];
        push    eax;
        call    CBattle::IsChrStatusNeedRefresh
        neg     eax;
        ret;
    }
}

BOOL FASTCALL CBattle::IsChrStatusNeedRefresh(ULONG_PTR ChrPosition, PCHAR_STATUS CurrentStatus, ULONG_PTR PrevLevel)
{
    PMONSTER_STATUS MSData;

    MSData = GetMonsterStatus() + ChrPosition;

    if (GetActor()->GetPartyChipMap()[MSData->CharID] < MINIMUM_CUSTOM_CHAR_ID)
    {
        return CurrentStatus->Level != PrevLevel;
    }

    return TRUE;
}

NAKED ULONG CBattle::GetChrIdForSCraft()
{
    INLINE_ASM
    {
        mov     ecx, [ebp - 08h];
        call    CBattle::GetActor
        mov     ecx, eax;
        call    CActor::GetPartyChipMap
        mov     ecx, dword ptr [ebp - 14h];
        movzx   ecx, [ecx]MONSTER_STATUS.CharID;
        movzx   eax, [eax + ecx * 2];
        cmp     eax, MINIMUM_CUSTOM_CHAR_ID
        cmovae  ecx, eax;
        ret;
    }
}

NAKED VOID CBattle::NakedGetTurnVoiceChrId()
{
    INLINE_ASM
    {
        mov     ecx, [ebp - 0Ch];
        mov     edx, [ebp + 08h];
        call    CBattle::GetTurnVoiceChrId;
        mov     ecx, eax;
        ret;
    }
}

ULONG FASTCALL CBattle::GetTurnVoiceChrId(PMONSTER_STATUS MSData)
{
    ULONG ChrId, PartyId;

    ChrId = MSData->CharID;
    PartyId = GetActor()->GetPartyChipMap()[ChrId];

    return PartyId < MINIMUM_CUSTOM_CHAR_ID ? ChrId : PartyId;
}

NAKED VOID CBattle::NakedGetSBreakVoiceChrId()
{
    INLINE_ASM
    {
        jmp CBattle::NakedGetTurnVoiceChrId;
    }
}

/************************************************************************
  EDAO
************************************************************************/

NAKED VOID EDAO::NakedGetChrSBreak()
{
    INLINE_ASM
    {
        mov     ecx, eax;
        jmp     EDAO::GetChrSBreak;
    }
}

VOID FASTCALL EDAO::GetChrSBreak(PMONSTER_STATUS MSData)
{
    ULONG           ChrId;
    PCRAFT_AI_INFO  Craft;

    ChrId = GetBattle()->GetActor()->GetPartyChipMap()[MSData->CharID];
    if (ChrId < MINIMUM_CUSTOM_CHAR_ID)
    {
        MSData->CurrentActionIndex = GetSBreakList()[MSData->CharID];
        return;
    }

    Craft = MSData->SCraftAiInfo;
    for (ULONG_PTR Count = countof(MSData->SCraftAiInfo); Count; ++Craft, --Count)
    {
        if (Craft->CraftIndex == 0)
            break;
    }

    //Craft = Craft == MSData->SCraftAiInfo ? Craft : (Craft - 1);
    Craft = PtrSub(Craft, ((Craft == MSData->SCraftAiInfo) - 1) & sizeof(*Craft));

    MSData->SelectedCraft.MagicAriaActionIndex  = Craft->MagicAriaActionIndex;
    MSData->SelectedCraft.ActionIndex           = Craft->ActionIndex;
    MSData->CurrentActionIndex                  = Craft->CraftIndex;
}

/************************************************************************
  CGlobal
************************************************************************/

PCSTR CGlobal::GetMagicDescription(USHORT MagicId)
{
    if (MagicId < MINIMUM_CUSTOM_CRAFT_INDEX)
        return (this->*StubGetMagicDescription)(MagicId);

    CBattle*        Battle;
    PMONSTER_STATUS MSData;

    Battle = GetEDAO()->GetBattle();

    MSData = &Battle->GetMonsterStatus()[Battle->GetCurrentChrIndex()];

    return MSData->CraftDescription[MagicId - MINIMUM_CUSTOM_CRAFT_INDEX].Description;
}

PBYTE CGlobal::GetMagicQueryTable(USHORT MagicId)
{
    if (MagicId < MINIMUM_CUSTOM_CRAFT_INDEX)
        return (this->*StubGetMagicQueryTable)(MagicId);

    static BYTE StaticMagicQueryTable[] = { 233, 233, 233, 233, 233, 233, 233, 233 };

    return StaticMagicQueryTable;
}