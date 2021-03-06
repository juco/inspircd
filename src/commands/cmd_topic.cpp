/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2007-2008 Robin Burchell <robin+git@viroteck.net>
 *   Copyright (C) 2008 Craig Edwards <craigedwards@brainbox.cc>
 *   Copyright (C) 2008 Thomas Stagner <aquanight@inspircd.org>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "inspircd.h"

/** Handle /TOPIC. These command handlers can be reloaded by the core,
 * and handle basic RFC1459 commands. Commands within modules work
 * the same way, however, they can be fully unloaded, where these
 * may not.
 */
class CommandTopic : public SplitCommand
{
	ChanModeReference secretmode;
	ChanModeReference topiclockmode;

 public:
	/** Constructor for topic.
	 */
	CommandTopic(Module* parent)
		: SplitCommand(parent, "TOPIC", 1, 2)
		, secretmode(parent, "secret")
		, topiclockmode(parent, "topiclock")
	{
		syntax = "<channel> [<topic>]";
		Penalty = 2;
	}

	/** Handle command.
	 * @param parameters The parameters to the comamnd
	 * @param pcnt The number of parameters passed to teh command
	 * @param user The user issuing the command
	 * @return A value from CmdResult to indicate command success or failure.
	 */
	CmdResult HandleLocal(const std::vector<std::string>& parameters, LocalUser* user);
};

CmdResult CommandTopic::HandleLocal(const std::vector<std::string>& parameters, LocalUser* user)
{
	Channel* c = ServerInstance->FindChan(parameters[0]);
	if (!c)
	{
		user->WriteNumeric(ERR_NOSUCHNICK, "%s :No such nick/channel", parameters[0].c_str());
		return CMD_FAILURE;
	}

	if (parameters.size() == 1)
	{
		if (c)
		{
			if ((c->IsModeSet(secretmode)) && (!c->HasUser(user)))
			{
				user->WriteNumeric(ERR_NOSUCHNICK, "%s :No such nick/channel", c->name.c_str());
				return CMD_FAILURE;
			}

			if (c->topic.length())
			{
				user->WriteNumeric(RPL_TOPIC, "%s :%s", c->name.c_str(), c->topic.c_str());
				user->WriteNumeric(RPL_TOPICTIME, "%s %s %lu", c->name.c_str(), c->setby.c_str(), (unsigned long)c->topicset);
			}
			else
			{
				user->WriteNumeric(RPL_NOTOPICSET, "%s :No topic is set.", c->name.c_str());
			}
		}
		return CMD_SUCCESS;
	}

	std::string t = parameters[1]; // needed, in case a module wants to change it
	ModResult res;
	FIRST_MOD_RESULT(OnPreTopicChange, res, (user,c,t));

	if (res == MOD_RES_DENY)
		return CMD_FAILURE;
	if (res != MOD_RES_ALLOW)
	{
		if (!c->HasUser(user))
		{
			user->WriteNumeric(ERR_NOTONCHANNEL, "%s :You're not on that channel!", c->name.c_str());
			return CMD_FAILURE;
		}
		if (c->IsModeSet(topiclockmode) && !ServerInstance->OnCheckExemption(user, c, "topiclock").check(c->GetPrefixValue(user) >= HALFOP_VALUE))
		{
			user->WriteNumeric(ERR_CHANOPRIVSNEEDED, "%s :You do not have access to change the topic on this channel", c->name.c_str());
			return CMD_FAILURE;
		}
	}

	c->SetTopic(user, t);
	return CMD_SUCCESS;
}


COMMAND_INIT(CommandTopic)
