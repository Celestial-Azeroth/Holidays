#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Unit.h"
#include "World.h"

// TODO fix this somehow
bool modHEnabled = true;
bool modHAnnouncer = true;

bool fridayIsWeekend = false;

// Returns which weekend number it is.
uint32 GetWeekendOfMonth(int32 dayOfMonth)
{
	if (dayOfMonth >= 1 && dayOfMonth <= 7)
		return 1; 	// Weekend 1 = 1-7, 
	else if (dayOfMonth >= 8 && dayOfMonth <= 14)
		return 2; 	// Weekend 2 = 8-14, 
	else if (dayOfMonth >= 15 && dayOfMonth <= 21)
		return 3; 	// Weekend 3 = 15-21, 
	else if (dayOfMonth >= 22 && dayOfMonth <= 28)
		return 4; 	// Weekend 4 = 22-28, 
	else
		return 5;  	// Weekend 5 = 29-31
}

uint32 GetDayOfWeek()
{
	time_t now = time(0);
	tm* localTime = localtime(&now);
	return localTime->tm_wday; // Weekday (0 = Sunday) 
}

uint32 GetDayOfMonth()
{
	time_t now = time(0);
	tm* localTime = localtime(&now);
	return localTime->tm_mday; // Weekday (0 = Sunday) 
}

bool IsWeekend(int32 weekDay)
{
	if (weekDay == 0 || weekDay == 6 || (fridayIsWeekend && weekDay == 5))
		return true;

	return false;
}

bool IsWorkday(int32 weekDay)
{
	if (weekDay != 0 && weekDay != 6)
		return true;

	return false;
}

bool ShouldDoubleItem(int32 dayOfWeek, Item* item)
{
	const ItemTemplate* itemTemplate = item->GetTemplate();
	if (!itemTemplate)
		return false;

	if (dayOfWeek == 1 || dayOfWeek == 3)
	{
		switch (itemTemplate->Class)
		{
		case ITEM_CLASS_TRADE_GOODS:
			return true;
		case ITEM_CLASS_REAGENT:
			return true;
		}
	}

	if (dayOfWeek == 2 || dayOfWeek == 4)
	{
		switch (itemTemplate->Class)
		{
		case ITEM_CLASS_MONEY:
			return true;
		}
	}


	return false;
}

class HolidaysConfig : public WorldScript
{
public:
	HolidaysConfig() : WorldScript("HolidaysConfig_World") {}

	void OnBeforeConfigLoad(bool reload) override
	{
		if (!reload)
		{
			SetConfigSettings();
		}
	}

	void SetConfigSettings()
	{
		modHEnabled = sConfigMgr->GetOption<bool>("Holidays.Enable", true);
		modHAnnouncer = sConfigMgr->GetOption<bool>("Holidays.AnnounceOnLogin", true);

		fridayIsWeekend = sConfigMgr->GetOption<bool>("Holidays.FridayIsWeekend", false);
	}
};


class HolidaysAnnouncer : public PlayerScript
{
public:
	HolidaysAnnouncer() : PlayerScript("Holidays") {}

	void OnLogin(Player* player) override
	{
		if (!modHEnabled || !modHAnnouncer)
			return;

		ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Holidays|r module.");
		AnnounceEvent(player);
	}

	void AnnounceEvent(Player* player)
	{
		int32 dayOfWeek = GetDayOfWeek();
		int32 dayOfMonth = GetDayOfMonth();

		// If Special weekend we 4x exp
		if (GetWeekendOfMonth(dayOfMonth) == 5)
		{
			ChatHandler(player->GetSession()).SendSysMessage("Special Weekend Event Active: XP Rate |cff4CFF004x|r.");
		}
		else if (IsWeekend(dayOfWeek)) // If regular weekend we 2x exp
		{
			ChatHandler(player->GetSession()).SendSysMessage("Weekend Event Active: XP Rate |cff4CFF002x|r.");
		}

		if (dayOfWeek == 1 || dayOfWeek == 3)
		{
			ChatHandler(player->GetSession()).SendSysMessage("Event Active: |cff4CFF00Item Day|r.");
		}

		if (dayOfWeek == 2 || dayOfWeek == 4)
		{
			ChatHandler(player->GetSession()).SendSysMessage("Event Active: |cff4CFF00Money Day|r.");
		}
	}
};

class HolidaysEventer : public PlayerScript
{
public:
	HolidaysEventer() : PlayerScript("Holidays") {}

	void OnGiveXP(Player* player, uint32& amount, Unit* victim, uint8 xpSource) override
	{
		int32 dayOfWeek = GetDayOfWeek();
		int32 dayOfMonth = GetDayOfMonth();

		// If Special weekend we 4x exp
		if (GetWeekendOfMonth(dayOfMonth) == 5)
		{
			amount *= 4;
		}
		else if (IsWeekend(dayOfWeek)) // If regular weekend we 2x exp
		{
			amount *= 2;
		}
	}

	void OnLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid) override
	{
		int32 dayOfWeek = GetDayOfWeek();

		if (ShouldDoubleItem(dayOfWeek, item))
		{
			if (urand(0, 99) < 50)
			{
				uint32 bonusCount = count;
				player->AddItem(item->GetEntry(), bonusCount);

				ChatHandler(player->GetSession()).SendSysMessage("Bonus loot! You received double the yield!");
			}
		}
	}
};


void AddHolidaysScripts()
{
	new HolidaysConfig();
	new HolidaysAnnouncer();
	new HolidaysEventer();
}