/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
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


#ifndef __TESTSUITE_H__
#define __TESTSUITE_H__

class InspIRCd;

class TestSuite : public Extensible
{
 private:
	InspIRCd* ServerInstance;
	bool RealGenerateUIDTests();

 public:
	TestSuite(InspIRCd* Instance);
	~TestSuite();

	bool DoThreadTests();
	bool DoWildTests();
	bool DoCommaSepStreamTests();
	bool DoSpaceSepStreamTests();
	bool DoGenerateUIDTests();
};

#endif
