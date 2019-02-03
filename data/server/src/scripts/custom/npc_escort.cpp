#include "scriptPCH.h"

enum
{
    POINT_LAST_POINT    = 0xFFFFFF
};

struct npc_escort_genericAI : public npc_escortAI
{
    npc_escort_genericAI(Creature* pCreature, CreatureEscortData const* data) : npc_escortAI(pCreature)
    {
        Reset();
        m_pEscortData = data;
        if (!m_pEscortData)
            sLog.outError("npc_escort : La creature %u n'a pas de donnees dans la table `script_escort_data` ! Le PNJ sera inactif.");
    }

    // ATTENTION : Peut etre NULL
    CreatureEscortData const* m_pEscortData;

    void Reset()
    {
        if (Player* pPlayer = GetPlayerForEscort())
        {
            if (pPlayer->GetQuestStatus(m_pEscortData->uiQuestEntry) == QUEST_STATUS_INCOMPLETE)
            {
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
            }
            else if (pPlayer->GetQuestStatus(m_pEscortData->uiQuestEntry) == QUEST_STATUS_FAILED)
            {
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
            }
        }
    }

    void EnterEvadeMode()
    {
        //m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop(true);
        m_creature->SetLootRecipient(NULL);

        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            sLog.outDebug("EscortAI has left combat and is now returning to CombatStartPosition.");

            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                float fPosX, fPosY, fPosZ;
                m_creature->GetCombatStartPosition(fPosX, fPosY, fPosZ);
                m_creature->GetMotionMaster()->MovePoint(POINT_LAST_POINT, fPosX, fPosY, fPosZ);
            }
        }
        else
        {
            m_creature->GetMotionMaster()->MoveTargetedHome();
            sLog.outDebug("EscortAI has left combat and MoveTargetedHome()");
        }

        Reset();
    }

    void WaypointReached(uint32 uiPointId)
    {
        if (!m_pEscortData)
            return;

        // Escorte finie.
        if (uiPointId == m_pEscortData->uiLastWaypointEntry)
        {
            if (Player* pPlayer = GetPlayerForEscort())
                pPlayer->GroupEventHappens(m_pEscortData->uiQuestEntry, m_creature);
        }
    }
    void JustDied(Unit* pKiller)
    {
        if (!m_pEscortData)
            return;

        if (Player* pPlayer = GetPlayerForEscort())
        {
            if (pPlayer->GetQuestStatus(m_pEscortData->uiQuestEntry) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(m_pEscortData->uiQuestEntry);
        }
    }
    void JustStartedEscort()
    {
        if (!m_pEscortData)
            return;

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);

        m_creature->setFaction(m_pEscortData->uiEscortFaction);
    }
    void OnQuestAccept(Player* pPlayer, const Quest* pQuest)
    {
        if (!m_pEscortData)
            return;

        if (pQuest->GetQuestId() == m_pEscortData->uiQuestEntry)
        {
            Start(false, pPlayer->GetGUID(), pQuest);
            m_creature->MonsterTextEmote("quest accepted", NULL);
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (!m_pEscortData)
            return;

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
                SetEscortPaused(false);
            return;
        }
        if (!HasEscortState(STATE_ESCORT_PAUSED))
            SetEscortPaused(true);

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_escort_genericAI(Creature* pCreature)
{
    CreatureEscortData const* data = sScriptMgr.GetEscortData(pCreature->GetEntry());
    npc_escort_genericAI* pTempAI = NULL;
    pTempAI = new npc_escort_genericAI(pCreature, data);
    return (CreatureAI*)pTempAI;
}

bool QuestAccept_npc_escort_genericAI(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (npc_escort_genericAI* pEscortAI = dynamic_cast<npc_escort_genericAI*>(pCreature->AI()))
        pEscortAI->OnQuestAccept(pPlayer, pQuest);
    return true;
}


void AddSC_npc_escort()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "npc_escort";
    newscript->GetAI = &GetAI_npc_escort_genericAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_escort_genericAI;
    newscript->RegisterSelf();
}