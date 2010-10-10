/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2010 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#include "inspircd.h"
#include "account.h"

/* $ModDesc: Allow listing and viewing of accounts */

static dynamic_reference<AccountProvider> accounts("account");
static dynamic_reference<AccountDBProvider> db("accountdb");

/** Handle /ACCTLIST
 */
class CommandAcctlist : public Command
{
	TSBoolExtItem& hidden;
 public:
	CommandAcctlist(Module* Creator, TSBoolExtItem& hidden_ref) : Command(Creator,"ACCTLIST", 0, 1), hidden(hidden_ref)
	{
		syntax = "[pattern]";
	}

	CmdResult Handle (const std::vector<std::string>& parameters, User *user)
	{
		// XXX: Use numerics instead of NOTICEs?
		bool displayAll = parameters.empty() || parameters[0] == "*";
		bool canSeeHidden = user->HasPrivPermission("accounts/auspex");
		irc::string username = accounts ? accounts->GetAccountName(user) : "";
		std::pair<time_t, bool>* ext;
		for(AccountDB::const_iterator iter = db->GetDB().begin(); iter != db->GetDB().end(); ++iter)
			if(displayAll || InspIRCd::Match(iter->second->name, parameters[0]))
				if(canSeeHidden || username == iter->second->name || ((ext = hidden.get(iter->second)) && !ext->second)) // default to hidden
					user->WriteServ("NOTICE %s :%s", user->nick.c_str(), iter->second->name.c_str());
		user->WriteServ("NOTICE %s :End of account list", user->nick.c_str());
		return CMD_SUCCESS;
	}
};

/** Handle /ACCTSHOW
 */
class CommandAcctshow : public Command
{
 public:
	CommandAcctshow(Module* Creator) : Command(Creator,"ACCTSHOW", 1, 1)
	{
		flags_needed = 'o'; syntax = "<account name>";
	}

	CmdResult Handle (const std::vector<std::string>& parameters, User *user)
	{
		AccountDBEntry* entry = db->GetAccount(parameters[0], true);
		if(!entry)
		{
			user->WriteServ("NOTICE %s :No such account", user->nick.c_str());
			return CMD_FAILURE;
		}
		user->WriteServ("NOTICE %s :Account \"%s\" TS: %lu Hash type: \"%s\" Hash/Password TS: %lu Connect class: \"%s\""
			" Connect class TS: %lu", user->nick.c_str(), entry->name.c_str(), entry->ts, entry->hash.c_str(),
			entry->hash_password_ts, entry->connectclass.c_str(), entry->connectclass_ts);
		for(Extensible::ExtensibleStore::const_iterator it = entry->GetExtList().begin(); it != entry->GetExtList().end(); ++it)
		{
			std::string value = it->first->serialize(FORMAT_USER, entry, it->second);
			if (!value.empty())
				user->WriteServ("NOTICE %s :%s: %s", user->nick.c_str(), it->first->name.c_str(), value.c_str());
		}
		return CMD_SUCCESS;
	}
};

/** Handle /SETHIDDEN
 */
class CommandSethidden : public Command
{
 public:
	TSBoolExtItem& hidden;
	CommandSethidden(Module* Creator, TSBoolExtItem& hidden_ref) : Command(Creator,"SETHIDDEN", 1, 1), hidden(hidden_ref)
	{
		syntax = "OFF|ON";
	}

	CmdResult Handle (const std::vector<std::string>& parameters, User *user)
	{
		AccountDBEntry* entry;
		if(!accounts || !accounts->IsRegistered(user) || !(entry = db->GetAccount(accounts->GetAccountName(user), false)))
		{
			user->WriteServ("NOTICE %s :You are not logged in", user->nick.c_str());
			return CMD_FAILURE;
		}
		bool newsetting;
		if(irc::string(parameters[0]) == "ON")
			newsetting = true;
		else if(irc::string(parameters[0]) == "OFF")
			newsetting = false;
		else
		{
			user->WriteServ("NOTICE %s :Unknown setting", user->nick.c_str());
			return CMD_FAILURE;
		}
		hidden.set(entry, std::make_pair(ServerInstance->Time(), newsetting));
		db->SendUpdate(entry, "hidden");
		user->WriteServ("NOTICE %s :Account hiding for %s %s successfully", user->nick.c_str(), entry->name.c_str(), newsetting ? "enabled" : "disabled");
		return CMD_SUCCESS;
	}
};

class ModuleAccountList : public Module
{
	TSBoolExtItem hidden;
	CommandAcctlist cmd_acctlist;
	CommandAcctshow cmd_acctshow;
	CommandSethidden cmd_sethidden;

 public:
	ModuleAccountList() : hidden("hidden", this), cmd_acctlist(this, hidden), cmd_acctshow(this), cmd_sethidden(this, hidden)
	{
	}

	void init()
	{
		if(!db) throw ModuleException("m_account_list requires that m_account be loaded");
		ServerInstance->Modules->AddService(hidden);
		ServerInstance->Modules->AddService(cmd_acctlist);
		ServerInstance->Modules->AddService(cmd_acctshow);
		ServerInstance->Modules->AddService(cmd_sethidden);
	}

	void Prioritize()
	{
		ServerInstance->Modules->SetPriority(this, I_ModuleInit, PRIORITY_AFTER, ServerInstance->Modules->Find("m_account.so"));
	}

	Version GetVersion()
	{
		return Version("Allow listing and viewing of accounts", VF_VENDOR|VF_OPTCOMMON);
	}
};

MODULE_INIT(ModuleAccountList)