/* Copyright (C) 2006 - 2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Kurinnaxx
SD%Complete: 100
SDComment: Set in DB trap Despawn Time
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "scriptPCH.h"
#include "ruins_of_ahnqiraj.h"

enum
{
    GO_TRAP                 =   180647,

    SPELL_MORTALWOUND       =   25646,
    SPELL_SUMMON_SANDTRAP   =   25648,
    SPELL_SANDTRAP_EFFECT   =   25656,
    SPELL_ENRAGE            =   26527,
    SPELL_WIDE_SLASH        =   25814,
    SPELL_TRASH             =   3391,
    SPELL_INVOCATION        =   26446,

    SAY_BREACHED            =   -1509022
};

struct boss_kurinnaxxAI : public ScriptedAI
{
    boss_kurinnaxxAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiInvocation_Timer;
    uint32 m_uiMortalWound_Timer;
    uint32 m_uiSandTrap_Timer;
    uint32 m_uiCleanSandTrap_Timer;
    uint32 m_uiTrash_Timer;
    uint32 m_uiWideSlash_Timer;
    bool m_bHasEnraged;

    void Reset()
    {
        m_uiInvocation_Timer = 10000;
        m_uiMortalWound_Timer = 7000;
        m_uiSandTrap_Timer = 7000;
        m_uiCleanSandTrap_Timer = 0;
        m_uiTrash_Timer = 10000;
        m_uiWideSlash_Timer = 15000;
        m_bHasEnraged = false;
        if (m_pInstance)
            m_pInstance->SetData(TYPE_KURINNAXX, NOT_STARTED);
    }

    void Aggro(Unit* pPuller)
    {
        m_creature->SetInCombatWithZone();
        if (m_pInstance)
            m_pInstance->SetData(TYPE_KURINNAXX, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance)
            return;

        //tempsummon since ossirian is not created when this event occurs
        if (Unit* pOssirian = m_creature->SummonCreature(NPC_OSSIRIAN,
                              m_creature->GetPositionX(),
                              m_creature->GetPositionY(),
                              m_creature->GetPositionZ() - 40.0f,
                              0,
                              TEMPSUMMON_TIMED_DESPAWN, 1000))
            DoScriptText(SAY_BREACHED, pOssirian);

        m_pInstance->SetData(TYPE_KURINNAXX, DONE);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (!m_bHasEnraged && ((m_creature->GetHealth() * 100) / m_creature->GetMaxHealth()) <= 30 && !m_creature->IsNonMeleeSpellCasted(false))
        {
            DoCast(m_creature->getVictim(), SPELL_ENRAGE);
            m_bHasEnraged = true;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // if no one gets to the trap in 5 seconds delete the trap
        if (m_uiCleanSandTrap_Timer < uiDiff)
        {
            if (GameObject* pTrap = GetClosestGameObjectWithEntry(m_creature, GO_TRAP, DEFAULT_VISIBILITY_DISTANCE))
            {
                m_creature->SendObjectDeSpawnAnim(pTrap->GetGUID());
                pTrap->Delete();
            }
        }
        else
            m_uiCleanSandTrap_Timer -= uiDiff;

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        /* Moral wound */
        if (m_uiMortalWound_Timer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MORTALWOUND) == CAST_OK)
                m_uiMortalWound_Timer = 9000;
        }
        else
            m_uiMortalWound_Timer -= uiDiff;

        /* Summon trap */
        if (m_uiSandTrap_Timer < uiDiff)
        {
            if (Unit* pUnit = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (GameObject* trap = m_creature->SummonGameObject(GO_TRAP, pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ(), 0, 0, 0, 0, 0, 0))
                    trap->SetOwnerGuid(m_creature->GetObjectGuid());
                m_uiSandTrap_Timer = urand(5100, 7000); /** Random timer for sandtrap between 1 and 7s */
                m_uiCleanSandTrap_Timer = 5000;
            }
        }
        else
            m_uiSandTrap_Timer -= uiDiff;

        /* WideSlash */
        if (m_uiWideSlash_Timer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_WIDE_SLASH) == CAST_OK)
                m_uiWideSlash_Timer = 10000 + (rand() % 10000);
        }
        else
            m_uiWideSlash_Timer -= uiDiff;

        /** Invoque player in front of him */
        if (m_uiInvocation_Timer < uiDiff)
        {
            if (Unit* pUnit = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_PLAYER))
            {

            	float x, y, z;

                m_creature->SendSpellGo(pUnit, 25681);
            	m_creature->GetRelativePositions(10.0f, 0.0f, 0.0f, x, y, z);
            	pUnit->NearLandTo(x, y, z+3.5f, pUnit->GetOrientation());
                m_uiInvocation_Timer = urand(5000, 10000);
            }
        }
        else
            m_uiInvocation_Timer -= uiDiff;


        /* Trash */
        if (m_uiTrash_Timer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_TRASH) == CAST_OK)
                m_uiTrash_Timer = 10000 + (rand() % 10000);
        }
        else
            m_uiTrash_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_kurinnaxx(Creature* pCreature)
{
    return new boss_kurinnaxxAI(pCreature);
}

void AddSC_boss_kurinnaxx()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_kurinnaxx";
    newscript->GetAI = &GetAI_boss_kurinnaxx;
    newscript->RegisterSelf();
}
