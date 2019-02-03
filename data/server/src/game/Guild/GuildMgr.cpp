/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
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

#include "GuildMgr.h"
#include "Guild.h"
#include "Log.h"
#include "ObjectGuid.h"
#include "Database/DatabaseEnv.h"
#include "Policies/SingletonImp.h"
#include "ProgressBar.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"

INSTANTIATE_SINGLETON_1(GuildMgr);

GuildMgr::GuildMgr()
{
}

GuildMgr::~GuildMgr()
{
    for (GuildMap::iterator itr = m_GuildMap.begin(); itr != m_GuildMap.end(); ++itr)
        delete itr->second;

    CleanUpPetitions();
}

void GuildMgr::CleanUpPetitions()
{
    for (auto iter = m_petitionMap.begin(); iter != m_petitionMap.end(); ++iter)
        delete iter->second; // will clean up signatures too

    m_petitionMap.clear();
}

void GuildMgr::AddGuild(Guild* guild)
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_guildMutex);
    m_GuildMap[guild->GetId()] = guild;
}

void GuildMgr::RemoveGuild(uint32 guildId)
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_guildMutex);
    m_GuildMap.erase(guildId);
}

Guild* GuildMgr::GetGuildById(uint32 guildId) const
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_guildMutex);
    GuildMap::const_iterator itr = m_GuildMap.find(guildId);
    if (itr != m_GuildMap.end())
        return itr->second;

    return NULL;
}

Guild* GuildMgr::GetGuildByName(std::string const& name) const
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_guildMutex);
    for (GuildMap::const_iterator itr = m_GuildMap.begin(); itr != m_GuildMap.end(); ++itr)
        if (itr->second->GetName() == name)
            return itr->second;

    return NULL;
}

Guild* GuildMgr::GetGuildByLeader(ObjectGuid const& guid) const
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_guildMutex);
    for (GuildMap::const_iterator itr = m_GuildMap.begin(); itr != m_GuildMap.end(); ++itr)
        if (itr->second->GetLeaderGuid() == guid)
            return itr->second;

    return NULL;
}

std::string GuildMgr::GetGuildNameById(uint32 guildId) const
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_guildMutex);
    GuildMap::const_iterator itr = m_GuildMap.find(guildId);
    if (itr != m_GuildMap.end())
        return itr->second->GetName();

    return "";
}

void GuildMgr::LoadGuilds()
{
    uint32 count = 0;

    //                                                    0             1          2          3           4           5           6
    QueryResult *result = CharacterDatabase.Query("SELECT guild.guildid,guild.name,leaderguid,EmblemStyle,EmblemColor,BorderStyle,BorderColor,"
                          //   7               8    9    10
                          "BackgroundColor,info,motd,createdate FROM guild ORDER BY guildid ASC");

    if (!result)
    {
        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u guild definitions", count);
        return;
    }

    // load guild ranks
    //                                                                0       1   2     3
    QueryResult *guildRanksResult   = CharacterDatabase.Query("SELECT guildid,rid,rname,rights FROM guild_rank ORDER BY guildid ASC, rid ASC");

    // load guild members
    //                                                                0       1                 2    3     4
    QueryResult *guildMembersResult = CharacterDatabase.Query("SELECT guildid,guild_member.guid,rank,pnote,offnote,"
                                      //   5                6                 7                 8                9                       10
                                      "characters.name, characters.level, characters.class, characters.zone, characters.logout_time, characters.account "
                                      "FROM guild_member LEFT JOIN characters ON characters.guid = guild_member.guid ORDER BY guildid ASC");

    BarGoLink bar(result->GetRowCount());

    do
    {
        //Field *fields = result->Fetch();

        bar.step();
        ++count;

        Guild *newGuild = new Guild;
        if (!newGuild->LoadGuildFromDB(result) ||
                !newGuild->LoadRanksFromDB(guildRanksResult) ||
                !newGuild->LoadMembersFromDB(guildMembersResult) ||
                !newGuild->CheckGuildStructure()
           )
        {
            newGuild->Disband();
            delete newGuild;
            continue;
        }
        newGuild->LoadGuildEventLogFromDB();
        AddGuild(newGuild);
    }
    while (result->NextRow());

    delete result;
    delete guildRanksResult;
    delete guildMembersResult;

    //delete unused LogGuid records in guild_eventlog table
    //you can comment these lines if you don't plan to change CONFIG_UINT32_GUILD_EVENT_LOG_COUNT
    CharacterDatabase.PExecute("DELETE FROM guild_eventlog WHERE LogGuid > '%u'", sWorld.getConfig(CONFIG_UINT32_GUILD_EVENT_LOG_COUNT));

    sLog.outString();
    sLog.outString(">> Loaded %u guild definitions", count);
}

void GuildMgr::LoadPetitions()
{
    CleanUpPetitions(); // for reload
    uint32 count = 0;

    //                                                    0          1             2            3
    QueryResult* result = CharacterDatabase.Query("SELECT ownerguid, petitionguid, charterguid, name FROM petition");

    if (!result)
    {
        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u petitions", count);
        return;
    }

    // load signatures
    //                                                                0          1             2           3
    QueryResult* petitionSignatures = CharacterDatabase.Query("SELECT ownerguid, petitionguid, playerguid, player_account FROM petition_sign");

    BarGoLink bar(result->GetRowCount());

    do
    {
        bar.step();
        ++count;

        Petition *petition = new Petition;
        if (!petition->LoadFromDB(result))
        {
            petition->Delete();
            delete petition;
            continue;
        }

        m_petitionMap[petition->GetId()] = petition;
    } while (result->NextRow());
    delete result;

    if (petitionSignatures)
    {
        do
        {
            Field *fields = petitionSignatures->Fetch();

            ObjectGuid ownerGuid = ObjectGuid(HIGHGUID_PLAYER, fields[0].GetUInt32());
            uint32 petitionId = fields[1].GetUInt32();
            ObjectGuid playerGuid = ObjectGuid(HIGHGUID_PLAYER, fields[2].GetUInt32());
            uint32 accountId = fields[3].GetUInt32();

            Petition* petition = GetPetitionById(petitionId);

            if (!petition)
            {
                // Signatures for a petition that does not exist. Delete it
                sLog.outErrorDb("Signatures exist for petition %u that does not exist", petitionId);
                CharacterDatabase.PExecute("DELETE FROM petition_sign WHERE petitionguid = '%u'", petitionId);
                continue;
            }

            if (ownerGuid != petition->GetOwnerGuid())
            {
                sLog.outErrorDb("Signatures exist for petition %u with a different owner, updating", petitionId);
                CharacterDatabase.PExecute("UPDATE petition_sign SET ownerguid = '%u' WHERE petitionguid = '%u'",
                    petition->GetOwnerGuid().GetCounter(), petition->GetId());

                ownerGuid = petition->GetOwnerGuid();
            }

            PetitionSignature* signature = new PetitionSignature(petition, playerGuid, accountId);
            petition->AddSignature(signature);

        } while (petitionSignatures->NextRow());
        delete petitionSignatures;
    }

    sLog.outString();
    sLog.outString(">> Loaded %u petitions", count);
}

Petition::~Petition()
{
    for (PetitionSignatureList::iterator iter = m_signatures.begin(); iter != m_signatures.end(); ++iter)
        delete *iter;

    m_signatures.clear();
}

void GuildMgr::CreatePetition(uint32 id, Player* player, const ObjectGuid& charterGuid, std::string& name)
{
    Petition* petition = new Petition(id, ObjectGuid(charterGuid), ObjectGuid(player->GetObjectGuid()), name);
    petition->SetTeam(player->GetTeam());
    petition->SaveToDB();

    ACE_Guard<ACE_Thread_Mutex> guard(m_petitionsMutex);
    m_petitionMap[petition->GetId()] = petition;
}

void GuildMgr::DeletePetition(Petition* petition)
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_petitionsMutex);
    m_petitionMap.erase(petition->GetId());

    petition->Delete();
    delete petition;
}

Petition* GuildMgr::GetPetitionById(uint32 id)
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_petitionsMutex);
    PetitionMap::iterator iter = m_petitionMap.find(id);
    if (iter != m_petitionMap.end())
        return iter->second;

    return nullptr;
}

Petition* GuildMgr::GetPetitionByCharterGuid(const ObjectGuid& charterGuid)
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_petitionsMutex);
    for (PetitionMap::iterator iter = m_petitionMap.begin(); iter != m_petitionMap.end(); ++iter)
    {
        Petition* petition = iter->second;
        if (petition->GetCharterGuid() == charterGuid)
            return petition;
    }

    return nullptr;
}

Petition* GuildMgr::GetPetitionByOwnerGuid(const ObjectGuid& ownerGuid)
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_petitionsMutex);
    for (PetitionMap::iterator iter = m_petitionMap.begin(); iter != m_petitionMap.end(); ++iter)
    {
        Petition* petition = iter->second;
        if (petition->GetOwnerGuid() == ownerGuid)
            return petition;
    }

    return nullptr;
}

bool Petition::LoadFromDB(QueryResult* result)
{
    if (!result)
    {
        sLog.outErrorDb("[Petitions] Unable to load petitions from DB");
        return false;
    }

    // SELECT ownerguid, petitionguid, name FROM petition
    Field* fields = result->Fetch();

    m_ownerGuid = ObjectGuid(HIGHGUID_PLAYER, fields[0].GetUInt32());
    m_id = fields[1].GetUInt32();
    m_charterGuid = ObjectGuid(HIGHGUID_ITEM, fields[2].GetUInt32());
    m_name = fields[3].GetString();

    m_team = sObjectMgr.GetPlayerTeamByGUID(m_ownerGuid);

    return true;
}

void Petition::Delete()
{
    // Only delete if initialized
    if (m_id)
    {
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM petition WHERE petitionguid = '%u'", m_id);
        CharacterDatabase.PExecute("DELETE FROM petition_sign WHERE petitionguid = '%u'", m_id);
        CharacterDatabase.CommitTransaction();
    }
}

void Petition::BuildSignatureData(WorldPacket& data)
{
    for (auto iter = m_signatures.cbegin(); iter != m_signatures.cend(); ++iter)
    {
        PetitionSignature *signature = *iter;
        data << signature->GetSignatureGuid();
        data << 0;
    }
}

bool Petition::Rename(std::string& newname)
{
    std::string db_newname = newname;
    CharacterDatabase.escape_string(db_newname);
    CharacterDatabase.PExecute("UPDATE petition SET name = '%s' WHERE petitionguid = '%u'",
        db_newname.c_str(), m_id);

    DEBUG_LOG("Petition %u renamed to '%s'", m_id, newname.c_str());

    m_name = newname;

    return true;
}

void Petition::SaveToDB()
{
    std::string escaped_name = m_name;
    CharacterDatabase.escape_string(escaped_name);
    CharacterDatabase.PExecute("INSERT INTO petition (ownerguid, petitionguid, charterguid, name) VALUES ('%u', '%u', '%u', '%s')",
        m_ownerGuid.GetCounter(), m_id, m_charterGuid.GetCounter(), escaped_name.c_str());
}

PetitionSignature* Petition::GetSignatureForPlayer(Player* player)
{
    PetitionSignature* signature = nullptr;
    // Note that in pretty much any case if the player has a signature on
    // this petition, then the account has a signature. Therefore, it will
    // return here
    if (signature = GetSignatureForAccount(player->GetSession()->GetAccountId()))
        return signature;

    if (signature = GetSignatureForPlayerGuid(player->GetObjectGuid()))
        return signature;

    return nullptr;
}

PetitionSignature* Petition::GetSignatureForAccount(uint32 accountId)
{
    for (auto iter = m_signatures.cbegin(); iter != m_signatures.cend(); ++iter)
    {
        PetitionSignature* signature = *iter;
        if (signature->GetSignatureAccountId() == accountId)
            return signature;
    }

    return nullptr;
}

PetitionSignature* Petition::GetSignatureForPlayerGuid(const ObjectGuid& guid)
{
    for (auto iter = m_signatures.cbegin(); iter != m_signatures.cend(); ++iter)
    {
        PetitionSignature* signature = *iter;
        if (signature->GetSignatureGuid() == guid)
            return signature;
    }

    return nullptr;
}

void Petition::AddSignature(PetitionSignature* signature)
{
    m_signatures.push_back(signature);
}

bool Petition::AddNewSignature(Player* player)
{
    if (IsComplete())
        return false;

    PetitionSignature* signature = new PetitionSignature(this, player);
    signature->SaveToDB();
    AddSignature(signature);

    return true;
}

PetitionSignature::PetitionSignature(Petition* petition, Player* player)
    : m_petition(petition), m_playerGuid(player->GetObjectGuid()),
    m_playerAccount(player->GetSession()->GetAccountId())
{

}

void PetitionSignature::SaveToDB()
{
    CharacterDatabase.PExecute("INSERT INTO petition_sign (ownerguid, petitionguid, playerguid, player_account) VALUES ('%u', '%u', '%u','%u')",
        m_petition->GetOwnerGuid().GetCounter(), m_petition->GetId(), m_playerGuid.GetCounter(), m_playerAccount);
}
