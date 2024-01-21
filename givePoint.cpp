/*
 * givePoints
 *   Copyright (C) 2020 Benjamin Jackson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bzfsAPI.h"

class givePoints : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
    virtual const char* Name();
    virtual void Init(const char* config);
    virtual void Cleanup();
    virtual bool SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);
};

BZ_PLUGIN(givePoints)

const char* givePoints::Name()
{
    return "givePoints 1.0.0";
}

void givePoints::Init(const char* config)
{
    bz_registerCustomSlashCommand("give", this);
}

void givePoints::Cleanup()
{
    Flush();

    bz_removeCustomSlashCommand("give");
}

double ConvertToNum(std::string message)
{

    int messagelength = (int)message.length();

    if (messagelength > 0 && messagelength < 6)
    {
        double messagevalue = 0;
        double ten = 1;

        for (int i = (messagelength - 1); i >= 0; i--)
        {

            if (message[i] < '0' || message[i] > '9')  // got something other than a number
                return 0;

            ten *= 10;
            messagevalue += (((double)message[i] - '0') / 10) * ten;
        }
        if (messagevalue)
            return messagevalue;
    }

    return 0;
}

bool givePoints::SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
    if (command == "give")
    {
        bz_BasePlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);
        if (params->size() != 1 && params->size() != 2)
        {
            bz_sendTextMessage(BZ_SERVER, playerID, "Syntax: 'Player callsign' '#' Number of points to give them, can't be more than you have. If the callsign you are trying to give points has a space use '' at the beginning and end.");
            bz_freePlayerRecord(fromPlayer);
            return true;
        }
        bz_BasePlayerRecord* toPlayer;
        int playerScore = bz_getPlayerWins(playerID) - bz_getPlayerLosses(playerID);
        int score = ConvertToNum(params->get(1));
        toPlayer = bz_getPlayerBySlotOrCallsign(params->get(0).c_str());

        if (toPlayer)
        {
            if (toPlayer->team == eObservers)
            {
                bz_sendTextMessage(BZ_SERVER, playerID, "You can't give points to an observer.");
                bz_freePlayerRecord(toPlayer);
                return true;
            }
            if (playerScore > 0 && score > 0 && score <= playerScore && toPlayer->callsign != fromPlayer->callsign)
            {
                bz_incrementPlayerWins(toPlayer->playerID, score);
                bz_sendTextMessagef(BZ_SERVER, toPlayer->playerID, "%s gave %i points to you.", bz_getPlayerCallsign(fromPlayer->playerID), (int)(score));
                bz_sendTextMessagef(BZ_SERVER, fromPlayer->playerID, "Gave %s %i points.", bz_getPlayerCallsign(toPlayer->playerID), (int)(score));
                bz_incrementPlayerLosses(fromPlayer->playerID, score);
                bz_freePlayerRecord(toPlayer);
                return true;
            }
            if (toPlayer->callsign == fromPlayer->callsign)
            {
                bz_sendTextMessage(BZ_SERVER, playerID, "You are not allowed to give yourself points.");
                bz_freePlayerRecord(toPlayer);
                return true;
            }
            if (score > playerScore)
            {
                bz_sendTextMessage(BZ_SERVER, playerID, "You can't give away more points than you have.");
                bz_freePlayerRecord(toPlayer);
                return true;
            }
            else
            {
                bz_sendTextMessagef(BZ_SERVER, fromPlayer->playerID, "%s is an invalid quantity of points.", params->get(1).c_str());
                bz_freePlayerRecord(toPlayer);
            }
        }
        else
        {
            bz_sendTextMessagef(BZ_SERVER, playerID, "%s does not exist to give points to", params->get(0));
            bz_freePlayerRecord(toPlayer);
        }

        bz_freePlayerRecord(fromPlayer);
        return true;
    }

    return false;
}
