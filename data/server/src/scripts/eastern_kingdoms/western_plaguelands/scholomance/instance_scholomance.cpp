/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Instance_Scholomance
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "scriptPCH.h"
#include "scholomance.h"

struct instance_scholomance : public ScriptedInstance
{
    instance_scholomance(Map* pMap) : ScriptedInstance(pMap)
    {
        Initialize();
    };

    std::string strInstData;
    uint32 m_auiEncounter[INSTANCE_SCHOLOMANCE_MAX_ENCOUNTER];

    uint64 m_uiVectusGUID;
    uint64 m_uiMardukeGUID;

    uint64 m_uiGateKirtonosGUID;
    uint64 m_uiGateGandlingGUID;
    uint64 m_uiGateMiliciaGUID;
    uint64 m_uiGateTheolenGUID;
    uint64 m_uiGatePolkeltGUID;
    uint64 m_uiGateRavenianGUID;
    uint64 m_uiGateBarovGUID;
    uint64 m_uiGateIlluciaGUID;

    uint32 m_uiBoneMinionCount0;
    uint32 m_uiBoneMinionCount1;
    uint32 m_uiBoneMinionCount2;
    uint32 m_uiBoneMinionCount3;
    uint32 m_uiBoneMinionCount4;
    uint32 m_uiBoneMinionCount5;

    void Initialize()
    {
        memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

        m_uiGateKirtonosGUID = 0;
        m_uiGateGandlingGUID = 0;
        m_uiGateMiliciaGUID = 0;
        m_uiGateTheolenGUID = 0;
        m_uiGatePolkeltGUID = 0;
        m_uiGateRavenianGUID = 0;
        m_uiGateBarovGUID = 0;
        m_uiGateIlluciaGUID = 0;

        m_uiBoneMinionCount0 = 0;
        m_uiBoneMinionCount1 = 0;
        m_uiBoneMinionCount2 = 0;
        m_uiBoneMinionCount3 = 0;
        m_uiBoneMinionCount4 = 0;
        m_uiBoneMinionCount5 = 0;

    }
    void OnCreatureCreate(Creature* pCreature)
    {
        switch (pCreature->GetEntry())
        {
            case NPC_VECTUS:
                m_uiVectusGUID = pCreature->GetGUID();
                break;
            case NPC_MARDUKE:
                m_uiMardukeGUID = pCreature->GetGUID();
                break;
        }
    }
    void OnGameObjectCreate(GameObject* pGo)
    {
        switch (pGo->GetEntry())
        {
            case GO_GATE_KIRTONOS:
                m_uiGateKirtonosGUID = pGo->GetGUID();
                break;
            case GO_GATE_GANDLING:
                m_uiGateGandlingGUID = pGo->GetGUID();
                break;
            case GO_GATE_MALICIA:
                m_uiGateMiliciaGUID = pGo->GetGUID();
                break;
            case GO_GATE_THEOLEN:
                m_uiGateTheolenGUID = pGo->GetGUID();
                break;
            case GO_GATE_POLKELT:
                m_uiGatePolkeltGUID = pGo->GetGUID();
                break;
            case GO_GATE_RAVENIAN:
                m_uiGateRavenianGUID = pGo->GetGUID();
                break;
            case GO_GATE_BAROV:
                m_uiGateBarovGUID = pGo->GetGUID();
                break;
            case GO_GATE_ILLUCIA:
                m_uiGateIlluciaGUID = pGo->GetGUID();
                break;
            case GO_VIEWING_ROOM_DOOR:
                if (GetData(TYPE_VIEWING_ROOM_DOOR) == DONE)
                    pGo->UseDoorOrButton();
                break;
        }
    }

    uint32 GetData(uint32 uiType)
    {
        if (uiType == TYPE_GANDLING)
        {
            if (m_auiEncounter[TYPE_GANDLING] == NOT_STARTED && m_auiEncounter[TYPE_ALEXEIBAROV] == DONE && m_auiEncounter[TYPE_THEOLEN] == DONE && m_auiEncounter[TYPE_RAVENIAN] == DONE &&
                    m_auiEncounter[TYPE_POLKELT] == DONE && m_auiEncounter[TYPE_MALICIA] == DONE && m_auiEncounter[TYPE_ILLUCIABAROV] == DONE)
                m_auiEncounter[TYPE_GANDLING] = SPECIAL;
            return m_auiEncounter[TYPE_GANDLING];
        }
        if (uiType < INSTANCE_SCHOLOMANCE_MAX_ENCOUNTER)
            return m_auiEncounter[uiType];
        return 0;
    }
    uint64 GetData64(uint32 uiData)
    {
        switch (uiData)
        {
            case DATA_VECTUS:
                return m_uiVectusGUID;
            case DATA_MARDUKE:
                return m_uiMardukeGUID;
        }
        return 0;
    }
    void OnCreatureDeath(Creature *who)
    {
        switch (who->GetEntry())
        {
            case NPC_KIRTONOS:
                if (GameObject* pGo = instance->GetGameObject(m_uiGateKirtonosGUID))
                    if (pGo->GetGoState() != GO_STATE_ACTIVE) // ferm�e
                        DoUseDoorOrButton(m_uiGateKirtonosGUID);
                break;
        }
    }

    void SetData(uint32 uiType, uint32 uiData)
    {
        switch (uiType)
        {
            case TYPE_GANDLING:
                m_auiEncounter[TYPE_GANDLING] = uiData;
                if (uiData == FAIL || uiData == DONE)
                {
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateMiliciaGUID))
                        if (pGo->GetGoState() != GO_STATE_ACTIVE)
                            DoUseDoorOrButton(m_uiGateMiliciaGUID);
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateTheolenGUID))
                        if (pGo->GetGoState() != GO_STATE_ACTIVE)
                            DoUseDoorOrButton(m_uiGateTheolenGUID);
                    if (GameObject* pGo = instance->GetGameObject(m_uiGatePolkeltGUID))
                        if (pGo->GetGoState() != GO_STATE_ACTIVE)
                            DoUseDoorOrButton(m_uiGatePolkeltGUID);
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateRavenianGUID))
                        if (pGo->GetGoState() != GO_STATE_ACTIVE)
                            DoUseDoorOrButton(m_uiGateRavenianGUID);
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateBarovGUID))
                        if (pGo->GetGoState() != GO_STATE_ACTIVE)
                            DoUseDoorOrButton(m_uiGateBarovGUID);
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateIlluciaGUID))
                        if (pGo->GetGoState() != GO_STATE_ACTIVE)
                            DoUseDoorOrButton(m_uiGateIlluciaGUID);
                }
                break;
            case TYPE_KIRTONOS:
                m_auiEncounter[TYPE_KIRTONOS] = uiData;
                if (uiData == IN_PROGRESS)
                    DoUseDoorOrButton(m_uiGateKirtonosGUID);
                else if (uiData == FAIL)
                {
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateKirtonosGUID))
                        if (pGo->GetGoState() != GO_STATE_ACTIVE) // ferm�e
                            DoUseDoorOrButton(m_uiGateKirtonosGUID);
                }
                break;
            case TYPE_ALEXEIBAROV:
            case TYPE_THEOLEN:
            case TYPE_RAVENIAN:
            case TYPE_POLKELT:
            case TYPE_MALICIA:
            case TYPE_ILLUCIABAROV:
            case TYPE_VIEWING_ROOM_DOOR:
                m_auiEncounter[uiType] = uiData;
                break;
            case TYPE_BONEMINION0:
                m_auiEncounter[TYPE_BONEMINION0] = uiData; // eventAI a changer : virer le onplayer kill et mettre on evade ou reached home ?
                if (uiData == IN_PROGRESS)
                {
                    m_uiBoneMinionCount0 = 0;
                    if (GameObject* pGo = instance->GetGameObject(m_uiGatePolkeltGUID))
                        if (pGo->GetGoState() == GO_STATE_ACTIVE) // ouverte
                            DoUseDoorOrButton(m_uiGatePolkeltGUID);
                }
                else if (uiData == DONE)
                {
                    ++m_uiBoneMinionCount0;
                    if (m_uiBoneMinionCount0 > 2)
                        if (GameObject* pGo = instance->GetGameObject(m_uiGatePolkeltGUID))
                            if (pGo->GetGoState() != GO_STATE_ACTIVE) // ferm�e
                                DoUseDoorOrButton(m_uiGatePolkeltGUID);
                }
                break;
            case TYPE_BONEMINION1:
                m_auiEncounter[TYPE_BONEMINION1] = uiData;
                if (uiData == IN_PROGRESS)
                {
                    m_uiBoneMinionCount1 = 0;
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateTheolenGUID))
                        if (pGo->GetGoState() == GO_STATE_ACTIVE) // ouverte
                            DoUseDoorOrButton(m_uiGateTheolenGUID);
                }
                else if (uiData == DONE)
                {
                    ++m_uiBoneMinionCount1;
                    if (m_uiBoneMinionCount1 > 3)
                        if (GameObject* pGo = instance->GetGameObject(m_uiGateTheolenGUID))
                            if (pGo->GetGoState() != GO_STATE_ACTIVE) // ferm�e
                                DoUseDoorOrButton(m_uiGateTheolenGUID);
                }
                break;
            case TYPE_BONEMINION2:
                m_auiEncounter[TYPE_BONEMINION2] = uiData;
                if (uiData == IN_PROGRESS)
                {
                    m_uiBoneMinionCount2 = 0;
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateMiliciaGUID))
                        if (pGo->GetGoState() == GO_STATE_ACTIVE) // ouverte
                            DoUseDoorOrButton(m_uiGateMiliciaGUID);
                }
                else if (uiData == DONE)
                {
                    ++m_uiBoneMinionCount2;
                    if (m_uiBoneMinionCount2 > 2)
                        if (GameObject* pGo = instance->GetGameObject(m_uiGateMiliciaGUID))
                            if (pGo->GetGoState() != GO_STATE_ACTIVE) // ferm�e
                                DoUseDoorOrButton(m_uiGateMiliciaGUID);
                }
                break;
            case TYPE_BONEMINION3:
                m_auiEncounter[TYPE_BONEMINION3] = uiData;
                if (uiData == IN_PROGRESS)
                {
                    m_uiBoneMinionCount3 = 0;
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateIlluciaGUID))
                        if (pGo->GetGoState() == GO_STATE_ACTIVE) // ouverte
                            DoUseDoorOrButton(m_uiGateIlluciaGUID);
                }
                else if (uiData == DONE)
                {
                    ++m_uiBoneMinionCount3;
                    if (m_uiBoneMinionCount3 > 2)
                        if (GameObject* pGo = instance->GetGameObject(m_uiGateIlluciaGUID))
                            if (pGo->GetGoState() != GO_STATE_ACTIVE) // ferm�e
                                DoUseDoorOrButton(m_uiGateIlluciaGUID);
                }
                break;
            case TYPE_BONEMINION4:
                m_auiEncounter[TYPE_BONEMINION4] = uiData;
                if (uiData == IN_PROGRESS)
                {
                    m_uiBoneMinionCount4 = 0;
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateBarovGUID))
                        if (pGo->GetGoState() == GO_STATE_ACTIVE) // ouverte
                            DoUseDoorOrButton(m_uiGateBarovGUID);
                }
                else if (uiData == DONE)
                {
                    ++m_uiBoneMinionCount4;
                    if (m_uiBoneMinionCount4 > 3)
                        if (GameObject* pGo = instance->GetGameObject(m_uiGateBarovGUID))
                            if (pGo->GetGoState() != GO_STATE_ACTIVE) // ferm�e
                                DoUseDoorOrButton(m_uiGateBarovGUID);
                }
                break;
            case TYPE_BONEMINION5:
                m_auiEncounter[TYPE_BONEMINION5] = uiData;
                if (uiData == IN_PROGRESS)
                {
                    m_uiBoneMinionCount5 = 0;
                    if (GameObject* pGo = instance->GetGameObject(m_uiGateRavenianGUID))
                        if (pGo->GetGoState() == GO_STATE_ACTIVE) // ouverte
                            DoUseDoorOrButton(m_uiGateRavenianGUID);
                }
                else if (uiData == DONE)
                {
                    ++m_uiBoneMinionCount5;
                    if (m_uiBoneMinionCount5 > 2)
                        if (GameObject* pGo = instance->GetGameObject(m_uiGateRavenianGUID))
                            if (pGo->GetGoState() != GO_STATE_ACTIVE) // ferm�e
                                DoUseDoorOrButton(m_uiGateRavenianGUID);
                }
                break;
            default:
                break;
        }

        if (uiData == DONE)
        {
            std::ostringstream saveStream;
            for (uint8 i = 0; i < INSTANCE_SCHOLOMANCE_MAX_ENCOUNTER; ++i)
                saveStream << m_auiEncounter[i] << " ";
            strInstData = saveStream.str();

            SaveToDB();
            OUT_SAVE_INST_DATA_COMPLETE;
        }
        SummonGandlingIfPossible();
    }
    /** Load / save system */
    const char* Save()
    {
        return strInstData.c_str();
    }

    void Load(const char* chrIn)
    {
        if (!chrIn)
            return;
        std::istringstream loadStream(chrIn);
        for (uint8 i = 0; i < INSTANCE_SCHOLOMANCE_MAX_ENCOUNTER; ++i)
        {
            loadStream >> m_auiEncounter[i];
            if (m_auiEncounter[i] == IN_PROGRESS)
                m_auiEncounter[i] = NOT_STARTED;
        }
        SummonGandlingIfPossible();

        OUT_LOAD_INST_DATA_COMPLETE;
    }
    void SummonGandlingIfPossible()
    {
        if (GetData(TYPE_GANDLING) == SPECIAL)
        {
            instance->SummonCreature(NPC_GANDLING, 180.73f, -9.43856f, 75.507f, 1.61399f, TEMPSUMMON_DEAD_DESPAWN, 0);
            SetData(TYPE_GANDLING, IN_PROGRESS);
        }
    }
};

InstanceData* GetInstanceData_instance_scholomance(Map* pMap)
{
    return new instance_scholomance(pMap);
}

bool GOOpen_brazier_herald(Player* pUser, GameObject *pGo)
{
    if (InstanceData* pInst = pGo->GetInstanceData())
    {
        // Verif pour voir si le boss est deja SPAWN
        if (pInst->GetData(TYPE_KIRTONOS) != NOT_STARTED)
            return false;
        pInst->SetData(TYPE_KIRTONOS, IN_PROGRESS);
        Creature* kirtonos = pUser->SummonCreature(NPC_KIRTONOS, 309.65f, 93.47f, 101.66f, 0.03f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 1800000);
        if (kirtonos && kirtonos->AI())
            kirtonos->AI()->AttackStart(pUser);
    }

    return true;
}
enum
{
    SPELL_MULTI_SHOT        = 20735,
    SPELL_SHOOT             = 16100,//not used for now
    SPELL_SHIELD_BASH       = 11972
};
struct Locations
{
    float x, y, z;
};
//tourne en rond.
static Locations ronde[] =
{
    {221.356903f, 133.581757f, 109.640160f},
    {221.075699f, 160.426987f, 109.640160f},
    {181.770874f, 159.967178f, 109.604874f},
    {181.937195f, 133.052261f, 109.602188f}
};

struct boss_lordblackwoodAI : public ScriptedAI
{
    boss_lordblackwoodAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        LastWayPoint = 0;
        Reset();
    }

    uint32 ShieldBash_Timer;
    uint32 MultiShot_Timer;
    uint32 LastWayPoint;

    void Reset()
    {
        ShieldBash_Timer = 8000;
        MultiShot_Timer = 1000;
        m_creature->GetMotionMaster()->MovePoint(LastWayPoint, ronde[LastWayPoint].x, ronde[LastWayPoint].y, ronde[LastWayPoint].z);
    }

    void MovementInform(uint32 uiType, uint32 uiPointId)
    {
        if (!m_creature->getVictim())
        {
            m_creature->SetWalk(true);
            if (uiPointId < 3)
                m_creature->GetMotionMaster()->MovePoint(uiPointId + 1, ronde[uiPointId + 1].x, ronde[uiPointId + 1].y, ronde[uiPointId + 1].z);
            else if (uiPointId == 3)
                m_creature->GetMotionMaster()->MovePoint(0, ronde[0].x, ronde[0].y, ronde[0].z);
        }
        if (uiPointId >= 0 && uiPointId < 4)
            LastWayPoint = uiPointId;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (!m_creature->IsWithinMeleeRange(m_creature->getVictim()))
        {
            if (MultiShot_Timer < diff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MULTI_SHOT) == CAST_OK)
                    MultiShot_Timer = 2000;
            }
            else
                MultiShot_Timer -= diff;
        }
        if (ShieldBash_Timer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHIELD_BASH) == CAST_OK)
                ShieldBash_Timer = 8000;
        }
        else
            ShieldBash_Timer -= diff;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_lordblackwood(Creature* pCreature)
{
    return new boss_lordblackwoodAI(pCreature);
}

struct go_viewing_room_door : public GameObjectAI
{
    go_viewing_room_door(GameObject* pGo) : GameObjectAI(pGo) {}

    bool OnUse(Unit* user)
    {
        // Save door state to database
        if (user && user->GetInstanceData())
            user->GetInstanceData()->SetData(TYPE_VIEWING_ROOM_DOOR, DONE);
        return false;
    }
};

GameObjectAI* GOGetAI_go_viewing_room_door(GameObject *pGo)
{
    return new go_viewing_room_door(pGo);
}

void AddSC_instance_scholomance()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_scholomance";
    newscript->GetInstanceData = &GetInstanceData_instance_scholomance;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_brazier_herald";
    newscript->GOOpen = &GOOpen_brazier_herald;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_lord_blackwood";
    newscript->GetAI = &GetAI_boss_lordblackwood;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_viewing_room_door";
    newscript->GOGetAI = &GOGetAI_go_viewing_room_door;
    newscript->RegisterSelf();
}
