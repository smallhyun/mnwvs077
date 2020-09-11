#include "CharacterDBAccessor.h"
#include "GW_ItemSlotEquip.h"
#include "GW_ItemSlotBundle.h"
#include "GW_ItemSlotPet.h"
#include "GW_CharacterStat.h"
#include "GW_CharacterLevel.h"
#include "GW_Avatar.hpp"
#include "GW_Account.h"
#include "GW_CashItemInfo.h"
#include "GW_CharacterSlotCount.h"

#include "..\WvsLib\Net\InPacket.h"
#include "..\WvsLib\Net\OutPacket.h"
#include "..\WvsLib\Net\SocketBase.h"
#include "..\WvsLib\Net\PacketFlags\CenterPacketFlags.hpp"
#include "..\WvsLib\Net\PacketFlags\ShopPacketFlags.hpp"
#include "..\WvsLib\Memory\ZMemory.h"

#include "GW_CharacterList.hpp"
#include "GA_Character.hpp"
#include "GW_FuncKeyMapped.h"

std::vector<int> CharacterDBAccessor::PostLoadCharacterListRequest(SocketBase *pSrv, int uLocalSocketSN, int nAccountID, int nWorldID)
{
	OutPacket oPacket;
	GW_CharacterList chrList;
	chrList.Load(nAccountID, nWorldID);
	oPacket.Encode2(CenterSendPacketFlag::CharacterListResponse);
	oPacket.Encode4(uLocalSocketSN);

	oPacket.Encode1(chrList.nCount);
	for (int i = 0; i < chrList.nCount; ++i)
	{
		GA_Character chrEntry;
		chrEntry.LoadCharacter(chrList.aCharacterList[i]);
		chrEntry.EncodeAvatar(&oPacket);
		oPacket.Encode1(0); //bRanking?
	}

	pSrv->SendPacket(&oPacket);
	return chrList.aCharacterList;
}

void CharacterDBAccessor::PostCheckDuplicatedID(SocketBase *pSrv, int uLocalSocketSN, int nAccountID, const std::string & sCharacterName)
{
	OutPacket oPacket;
	oPacket.Encode2(CenterSendPacketFlag::CheckDuplicatedIDResult);
	oPacket.Encode4(uLocalSocketSN);
	oPacket.Encode4(nAccountID);
	oPacket.EncodeStr(sCharacterName);
	oPacket.Encode1(QueryCharacterIDByName(sCharacterName) == -1 ? 0 : 1);
	pSrv->SendPacket(&oPacket);
}

void CharacterDBAccessor::PostCreateNewCharacterRequest(SocketBase *pSrv, int uLocalSocketSN, int nAccountID, int nWorldID, const std::string& strName, int nGender, int nFace, int nHair, int nSkin, const int* aBody, const int* aStat)
{
	OutPacket oPacket;
	oPacket.Encode2((short)CenterSendPacketFlag::CreateCharacterResult);
	oPacket.Encode4(uLocalSocketSN);

	GA_Character chrEntry;
	chrEntry.nAccountID = nAccountID;
	chrEntry.nWorldID = nWorldID;
	chrEntry.strName = strName;
	if (QueryCharacterIDByName(strName) == -1)
	{
		chrEntry.mStat->nGender = nGender;

		chrEntry.nFieldID = 0;
		chrEntry.nGuildID = chrEntry.nPartyID = chrEntry.mStat->nPOP = 0;

		chrEntry.mStat->nFace = nFace;
		chrEntry.mStat->nHair = nHair;
		chrEntry.mStat->nSkin = nSkin;

		chrEntry.mStat->nStr = aStat[eStatData_POS_Str];
		chrEntry.mStat->nDex = aStat[eStatData_POS_Dex];
		chrEntry.mStat->nInt = aStat[eStatData_POS_Int];
		chrEntry.mStat->nLuk = aStat[eStatData_POS_Luk];
		chrEntry.mStat->nHP = aStat[eStatData_POS_HP];
		chrEntry.mStat->nMP = aStat[eStatData_POS_MP];
		chrEntry.mStat->nMaxHP = aStat[eStatData_POS_MaxHP];
		chrEntry.mStat->nMaxMP = aStat[eStatData_POS_MaxMP];
		chrEntry.mStat->nJob = aStat[eStatData_POS_Job];
		chrEntry.mStat->nSubJob = aStat[eStatData_POS_SubJob];
		chrEntry.mLevel->nLevel = aStat[eStatData_POS_Level];
		chrEntry.mStat->nAP = aStat[eStatData_POS_AP];

		chrEntry.mSlotCount->aSlotCount[GW_ItemSlotBase::EQUIP] = 40;
		chrEntry.mSlotCount->aSlotCount[GW_ItemSlotBase::CONSUME] = 40;
		chrEntry.mSlotCount->aSlotCount[GW_ItemSlotBase::ETC] = 40;
		chrEntry.mSlotCount->aSlotCount[GW_ItemSlotBase::INSTALL] = 40;
		chrEntry.mSlotCount->aSlotCount[GW_ItemSlotBase::CASH] = 40;

		GW_ItemSlotEquip *pCapEquip = AllocObj(GW_ItemSlotEquip);
		pCapEquip->nItemID = aBody[eEQPData_POS_CapEquip];
		pCapEquip->nPOS = eBodyEquip_POS_Cap;

		GW_ItemSlotEquip *pCoatEquip = AllocObj(GW_ItemSlotEquip);
		pCoatEquip->nItemID = aBody[eEQPData_POS_CoatEquip];
		pCoatEquip->nPOS = eBodyEquip_POS_Coat;

		GW_ItemSlotEquip *pPantsEquip = AllocObj(GW_ItemSlotEquip);
		pPantsEquip->nItemID = aBody[eEQPData_POS_PantsEquip];
		pPantsEquip->nPOS = eBodyEquip_POS_Pants;

		GW_ItemSlotEquip *pWeaponEquip = AllocObj(GW_ItemSlotEquip);
		pWeaponEquip->nItemID = aBody[eEQPData_POS_WeaponEquip];
		pWeaponEquip->nPOS = eBodyEquip_POS_Weapon;
		pWeaponEquip->nPAD = 3;

		GW_ItemSlotEquip *pShoesEquip = AllocObj(GW_ItemSlotEquip);
		pShoesEquip->nItemID = aBody[eEQPData_POS_ShoesEquip];
		pShoesEquip->nPOS = eBodyEquip_POS_Shoes;

		GW_ItemSlotEquip *pCapeEquip = AllocObj(GW_ItemSlotEquip);
		pCapeEquip->nItemID = aBody[eEQPData_POS_CapeEquip];
		pCapeEquip->nPOS = eBodyEquip_POS_Cape;

		GW_ItemSlotEquip *pShieldEquip = AllocObj(GW_ItemSlotEquip);
		pShieldEquip->nItemID = aBody[eEQPData_POS_ShieldEquip];
		pShieldEquip->nPOS = eBodyEquip_POS_Shield;

		GW_ItemSlotEquip* aEquips[] = {
			pCapEquip,
			pCoatEquip,
			pPantsEquip,
			pWeaponEquip,
			pShoesEquip,
			pCapeEquip,
			pShieldEquip
		};
		int nEquipCount = sizeof(aEquips) / sizeof(GW_ItemSlotBase*);
		for (int i = 0; i < nEquipCount; ++i)
			if (aEquips[i]->nItemID > 0)
			{
				aEquips[i]->nType = GW_ItemSlotBase::GW_ItemSlotType::EQUIP;
				chrEntry.mItemSlot[1].insert({ aEquips[i]->nPOS, aEquips[i] });
			}
		chrEntry.Save(true);

		GW_FuncKeyMapped funcKeyMapped(chrEntry.nCharacterID);
		funcKeyMapped.Save(true);
		oPacket.Encode1(0);
		chrEntry.LoadCharacter(chrEntry.nCharacterID);
		chrEntry.EncodeAvatar(&oPacket);
	}
	else
		oPacket.Encode1(1);

	pSrv->SendPacket(&oPacket);
}

void CharacterDBAccessor::GetDefaultCharacterStat(int *aStat)
{
	aStat[eStatData_POS_Str] = 10;
	aStat[eStatData_POS_Dex] = 10;
	aStat[eStatData_POS_Int] = 10;
	aStat[eStatData_POS_Luk] = 10;
	aStat[eStatData_POS_HP] = 50;
	aStat[eStatData_POS_MP] = 50;
	aStat[eStatData_POS_MaxHP] = 50;
	aStat[eStatData_POS_MaxMP] = 50;
	aStat[eStatData_POS_Job] = 0;
	aStat[eStatData_POS_SubJob] = 0;
	aStat[eStatData_POS_Level] = 1;
	aStat[eStatData_POS_AP] = 0;
}

void CharacterDBAccessor::PostCharacterDataRequest(SocketBase *pSrv, int nClientSocketID, int nCharacterID, void *oPacket_)
{
	OutPacket *oPacket = (OutPacket*)oPacket_;
	if (QueryCharacterAccountID(nCharacterID) == -1)
	{
		oPacket->Encode1(0);
		return;
	}
	oPacket->Encode1(1); //Valid
	GA_Character chrEntry;
	chrEntry.Load(nCharacterID);
	oPacket->Encode4(chrEntry.nAccountID);
	chrEntry.EncodeCharacterData(oPacket, true);
	GW_FuncKeyMapped funcKeyMapped(chrEntry.nCharacterID);
	funcKeyMapped.Load();
	funcKeyMapped.Encode(oPacket);
}

void CharacterDBAccessor::PostBuyCashItemRequest(SocketBase * pSrv, int uClientSocketSN, int nCharacterID, void * iPacket_)
{
	InPacket *iPacket = (InPacket*)iPacket_;

	int nAccountID = iPacket->Decode4();
	int nChargeType = iPacket->Decode1();
	int nType = iPacket->Decode1();
	bool bIsPet = iPacket->Decode1() == 1;
	int nPrice = iPacket->Decode4();
	OutPacket oPacket;
	oPacket.Encode2(CenterSendPacketFlag::CashItemResult);
	oPacket.Encode4(uClientSocketSN);
	oPacket.Encode4(nCharacterID);
	GW_Account account;
	account.Load(nAccountID);

	if (account.QueryCash(nChargeType) >= nPrice)
	{
		oPacket.Encode2(ShopInternalPacketFlag::OnCenterResBuyDone);
		account.UpdateCash(nChargeType, -nPrice);

		ZSharedPtr<GW_ItemSlotBase> pItem;
		if (nType == GW_ItemSlotBase::GW_ItemSlotType::EQUIP)
			pItem = MakeShared<GW_ItemSlotEquip>();
		else if (bIsPet)
			pItem = MakeShared<GW_ItemSlotPet>();
		else
			pItem = MakeShared<GW_ItemSlotBundle>();

		pItem->nType = (nType == GW_ItemSlotBase::GW_ItemSlotType::EQUIP ?
			GW_ItemSlotBase::GW_ItemSlotType::EQUIP
			: GW_ItemSlotBase::GW_ItemSlotType::CASH);
		pItem->bIsCash = true;
		pItem->bIsPet = bIsPet;
		pItem->DecodeInventoryPosition(iPacket);
		pItem->Decode(iPacket, false);
		pItem->nCharacterID = nCharacterID;
		pItem->liCashItemSN = GW_ItemSlotBase::IncItemSN(GW_ItemSlotBase::GW_ItemSlotType::CASH);
		pItem->nPOS = GW_ItemSlotBase::LOCK_POS;
		pItem->liItemSN = -1;

		GW_CashItemInfo cashItemInfo;
		cashItemInfo.Decode(iPacket);
		cashItemInfo.nAccountID = nAccountID;
		cashItemInfo.cashItemOption.liCashItemSN = pItem->liCashItemSN;
		cashItemInfo.bLocked = true;
		cashItemInfo.nGWItemSlotInstanceType = pItem->nInstanceType;
		cashItemInfo.Save(true);

		//09/12/2019 added. fix for bundled items.
		if (nType == GW_ItemSlotBase::GW_ItemSlotType::CASH && !bIsPet)
			((GW_ItemSlotBundle*)pItem)->nNumber = cashItemInfo.nNumber;

		pItem->Save(nCharacterID);

		oPacket.Encode4(account.QueryCash(1));
		oPacket.Encode4(account.QueryCash(2));
		cashItemInfo.Encode(&oPacket);
	}
	else
		oPacket.Encode2(ShopInternalPacketFlag::OnCenterResBuyFailed);

	pSrv->SendPacket(&oPacket);
}


void CharacterDBAccessor::PostLoadLockerRequest(SocketBase *pSrv, int uClientSocketSN, int nCharacterID, void *iPacket_)
{
	InPacket *iPacket = (InPacket*)iPacket_;
	int nAccountID = iPacket->Decode4();
	auto aRes = GW_CashItemInfo::LoadAll(nAccountID);
	std::vector<GW_ItemSlotPet> aPet;
	OutPacket oPacket;
	oPacket.Encode2((short)CenterSendPacketFlag::CashItemResult);
	oPacket.Encode4(uClientSocketSN);
	oPacket.Encode4(nCharacterID);
	oPacket.Encode2(ShopInternalPacketFlag::OnCenterLoadLockerDone);
	oPacket.Encode2((short)aRes.size());
	decltype(aRes) aResWithoutPet;
	for (auto& info : aRes)
	{
		if (info.nGWItemSlotInstanceType != GW_ItemSlotBase::GW_ItemSlotInstanceType::GW_ItemSlotPet_Type)
			aResWithoutPet.push_back(info);
		info.Encode(&oPacket);
		if (info.nGWItemSlotInstanceType == GW_ItemSlotBase::GW_ItemSlotInstanceType::GW_ItemSlotPet_Type)
		{
			aPet.push_back({});
			aPet[(int)aPet.size() - 1].Load(info.cashItemOption.liCashItemSN);
		}
	}
	//oPacket.Encode2(aResWithoutPet.size());
	//for (auto& info : aResWithoutPet)
	//	info.Encode(&oPacket);
	oPacket.Encode4(0);
	//for (auto& pet : aPet)
	//	pet.Encode(&oPacket, false);

	/*oPacket.Encode2(0);
	oPacket.Encode2(0);
	oPacket.Encode2(0);
	oPacket.Encode2(0);*/

	pSrv->SendPacket(&oPacket);
}

void CharacterDBAccessor::PostUpdateCashRequest(SocketBase *pSrv, int uClientSocketSN, int nCharacterID, void *iPacket_)
{
	InPacket *iPacket = (InPacket*)iPacket_;
	int nAccountID = iPacket->Decode4();
	GW_Account account;
	account.Load(nAccountID);

	OutPacket oPacket;
	oPacket.Encode2((short)CenterSendPacketFlag::CashItemResult);
	oPacket.Encode4(uClientSocketSN);
	oPacket.Encode4(nCharacterID);
	oPacket.Encode2(ShopInternalPacketFlag::OnCenterUpdateCashDone);

	oPacket.Encode4(account.nNexonCash);
	oPacket.Encode4(account.nMaplePoint);
	oPacket.Encode4(0);
	oPacket.Encode4(0);

	pSrv->SendPacket(&oPacket);
}

void CharacterDBAccessor::PostMoveSlotToLockerRequest(SocketBase * pSrv, int uClientSocketSN, int nCharacterID, void * iPacket_)
{
	InPacket *iPacket = (InPacket*)iPacket_;
	int nAccountID = iPacket->Decode4();
	long long int liCashItemSN = iPacket->Decode8();
	int nType = iPacket->Decode1();
	GW_CashItemInfo cashItemInfo;
	cashItemInfo.Load(liCashItemSN);
	cashItemInfo.bLocked = true;
	cashItemInfo.Save();

	ZSharedPtr<GW_ItemSlotBase> pItem;
	pItem.reset(GW_ItemSlotBase::CreateItem(cashItemInfo.nGWItemSlotInstanceType));
	pItem->nType = GW_ItemSlotBase::GW_ItemSlotType::CASH;
	pItem->bIsCash = true;
	pItem->Load(liCashItemSN);
	pItem->nPOS = GW_ItemSlotBase::LOCK_POS;
	pItem->Save(nCharacterID);

	OutPacket oPacket;
	oPacket.Encode2((short)CenterSendPacketFlag::CashItemResult);
	oPacket.Encode4(uClientSocketSN);
	oPacket.Encode4(nCharacterID);
	oPacket.Encode2(ShopInternalPacketFlag::OnCenterMoveToLockerDone);
	oPacket.Encode8(liCashItemSN);
	oPacket.Encode1(nType);
	cashItemInfo.Encode(&oPacket);
	oPacket.Encode4(0);
	oPacket.Encode4(0);
	pSrv->SendPacket(&oPacket);
}

void CharacterDBAccessor::PostMoveLockerToSlotRequest(SocketBase * pSrv, int uClientSocketSN, int nCharacterID, void * iPacket_)
{
	InPacket *iPacket = (InPacket*)iPacket_;
	int nAccountID = iPacket->Decode4();
	long long int liCashItemSN = iPacket->Decode8();
	OutPacket oPacket;
	oPacket.Encode2((short)CenterSendPacketFlag::CashItemResult);
	oPacket.Encode4(uClientSocketSN);
	oPacket.Encode4(nCharacterID);

	GW_CashItemInfo cashItemInfo;
	cashItemInfo.Load(liCashItemSN);
	GA_Character characterData;
	characterData.Load(nCharacterID);

	ZSharedPtr<GW_ItemSlotBase> pItem;
	pItem.reset(GW_ItemSlotBase::CreateItem(cashItemInfo.nGWItemSlotInstanceType));
	pItem->nType = GW_ItemSlotBase::GW_ItemSlotType::CASH;
	pItem->bIsCash = true;
	pItem->Load(liCashItemSN);
	auto nPOS = characterData.FindEmptySlotPosition((int)pItem->nType);
	if (!nPOS)
	{
		oPacket.Encode2(ShopInternalPacketFlag::OnCenterMoveToSlotFailed);
		pSrv->SendPacket(&oPacket);
		return;
	}
	pItem->nPOS = nPOS;
	pItem->Save(nCharacterID);
	cashItemInfo.bLocked = false;
	cashItemInfo.Save();

	oPacket.Encode2(ShopInternalPacketFlag::OnCenterMoveToSlotDone);
	oPacket.Encode8(pItem->liItemSN);
	//oPacket.Encode1(1);
	oPacket.Encode2(nPOS);
	pItem->RawEncode(&oPacket);
	oPacket.Encode4(0);
	oPacket.Encode4(0);
	pSrv->SendPacket(&oPacket);
}

int CharacterDBAccessor::QueryCharacterIDByName(const std::string & strName)
{
	Poco::Data::Statement queryStatement(GET_DB_SESSION);
	queryStatement << "SELECT CharacterID FROM `Character` Where CharacterName = '" << strName << "'";
	queryStatement.execute();
	Poco::Data::RecordSet recordSet(queryStatement);
	return recordSet.rowCount() == 0 ? -1 : recordSet["CharacterID"];
}

int CharacterDBAccessor::QueryCharacterFriendMax(int nCharacterID)
{
	Poco::Data::Statement queryStatement(GET_DB_SESSION);
	queryStatement << "SELECT FriendMaxNum FROM `Character` Where CharacterID = " << nCharacterID;
	queryStatement.execute();
	Poco::Data::RecordSet recordSet(queryStatement);
	return recordSet.rowCount() == 0 ? -1 : recordSet["FriendMaxNum"];
}

int CharacterDBAccessor::QueryCharacterAccountID(int nCharacterID)
{
	Poco::Data::Statement queryStatement(GET_DB_SESSION);
	queryStatement << "SELECT AccountID FROM `Character` Where CharacterID = " << nCharacterID;
	queryStatement.execute();
	Poco::Data::RecordSet recordSet(queryStatement);
	return recordSet.rowCount() == 0 ? -1 : recordSet["AccountID"];
}

void CharacterDBAccessor::OnCharacterSaveRequest(void *iPacket)
{
	InPacket *iPacket_ = (InPacket*)iPacket;
	GA_Character chr;
	chr.DecodeCharacterData(iPacket_, true);
	chr.Save(false);
	GW_FuncKeyMapped keyMapped(chr.nCharacterID);
	keyMapped.Decode(iPacket_, false);
	keyMapped.Save(false);
}