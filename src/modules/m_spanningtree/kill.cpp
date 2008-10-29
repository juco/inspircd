/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2008 InspIRCd Development Team
 * See: http://www.inspircd.org/wiki/index.php/Credits
 *
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */


#include "inspircd.h"
#include "xline.h"

#include "m_spanningtree/treesocket.h"
#include "m_spanningtree/treeserver.h"
#include "m_spanningtree/utils.h"

/* $ModDep: m_spanningtree/utils.h m_spanningtree/treeserver.h m_spanningtree/treesocket.h */



bool TreeSocket::RemoteKill(const std::string &prefix, std::deque<std::string> &params)
{
	if (params.size() != 2)
		return true;

	User* who = this->ServerInstance->FindNick(params[0]);

	if (who)
	{
		/* Prepend kill source, if we don't have one */
		if (*(params[1].c_str()) != '[')
		{
			TreeServer* ts = Utils->FindServer(prefix);
			params[1] = "[" + (ts ? ts->GetName() : prefix) + "] Killed (" + params[1] +")";
		}
		std::string reason = params[1];
		params[1] = ":" + params[1];
		Utils->DoOneToAllButSender(prefix,"KILL",params,prefix);
		// NOTE: This is safe with kill hiding on, as RemoteKill is only reached if we have a server prefix.
		// in short this is not executed for USERS.
		who->Write(":%s KILL %s :%s (%s)", prefix.c_str(), who->nick.c_str(), prefix.c_str(), reason.c_str());
		this->ServerInstance->Users->QuitUser(who, reason);
	}
	return true;
}
