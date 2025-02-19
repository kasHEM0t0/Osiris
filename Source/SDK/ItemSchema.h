#pragma once

#include <cstddef>

#include "Inconstructible.h"
#include "Pad.h"
#include "Entity.h"
#include "UtlMap.h"
#include "UtlMemory.h"
#include "UtlString.h"
#include "UtlVector.h"
#include "VirtualMethod.h"

#include "Constants/ItemId.h"
#include "Helpers/EconItemFunctions.h"

enum class WeaponId : short;

enum class EconRarity : std::uint8_t {
    Default = 0,
    Gray,
    LightBlue,
    Blue,
    Purple,
    Pink,
    Red,
    Gold
};

namespace csgo::pod
{

struct StickerKit {
    int id;
    int rarity;
    UtlString name;
    UtlString description;
    UtlString itemName;
    PAD(2 * sizeof(UtlString))
    UtlString inventoryImage;
    int tournamentID;
    int tournamentTeamID;
    int tournamentPlayerID;
};

}

union AttributeDataUnion {
    float asFloat;
    std::uint32_t asUint32;
    char* asBlobPointer;
};

struct StaticAttrib {
    std::uint16_t defIndex;
    AttributeDataUnion value;
    bool forceGCToGenerate;
};
static_assert(sizeof(StaticAttrib) == WIN32_LINUX(12, 24));

struct EconTool {
    INCONSTRUCTIBLE(EconTool)

    PAD(sizeof(std::uintptr_t))
    const char* typeName;
};

namespace csgo::pod { struct EconItemDefinition; }

class EconItemDefinition : public VirtualCallableFromPOD<EconItemDefinition, csgo::pod::EconItemDefinition> {
public:
    VIRTUAL_METHOD(WeaponId, getWeaponId, 0, (), ())
    VIRTUAL_METHOD(const char*, getItemBaseName, 2, (), ())
    VIRTUAL_METHOD(const char*, getItemTypeName, 3, (), ())
    VIRTUAL_METHOD(const char*, getInventoryImage, 5, (), ())
    VIRTUAL_METHOD(const char*, getPlayerDisplayModel, 6, (), ())
    VIRTUAL_METHOD(const char*, getWorldDisplayModel, 7, (), ())
    VIRTUAL_METHOD(std::uint8_t, getRarity, 12, (), ())
    VIRTUAL_METHOD_V(int, getNumberOfSupportedStickerSlots, 44, (), ())

    std::uint8_t getQuality() const noexcept
    {
        return *reinterpret_cast<std::uint8_t*>(getThis() + WIN32_LINUX(0x2B, 0x4B));
    }

    int getCapabilities() const noexcept
    {
        return *reinterpret_cast<int*>(getThis() + WIN32_LINUX(0x148, 0x1F8));
    }

    int getItemType() const noexcept
    {
        return *reinterpret_cast<int*>(getThis() + WIN32_LINUX(0x130, 0x1C8));
    }

    bool isServiceMedal() const noexcept
    {
        return getItemType() == 5; /* prestige_coin */
    }

    bool isTournamentCoin() const noexcept
    {
        return getItemType() == 8; /* fan_shield */
    }

    const UtlVector<StaticAttrib>& getStaticAttributes() const noexcept
    {
        return *reinterpret_cast<const UtlVector<StaticAttrib>*>(getThis() + WIN32_LINUX(0x30, 0x50));
    }

    std::uint32_t getAttributeValue(std::uint16_t attributeDefinitionIndex) const noexcept
    {
        const auto& staticAttributes = getStaticAttributes();
        for (int i = 0; i < staticAttributes.size; ++i) {
            if (staticAttributes[i].defIndex == attributeDefinitionIndex)
                return staticAttributes[i].value.asUint32;
        }
        return 0;
    }

    std::uint32_t getCrateSeriesNumber() const noexcept
    {
        return getAttributeValue(68 /* "set supply crate series" */);
    }

    bool hasCrateSeries() const noexcept
    {
        return getCrateSeriesNumber() != 0;
    }

    std::uint32_t getTournamentEventID() const noexcept
    {
        return getAttributeValue(137 /* "tournament event id" */);
    }

    std::uint32_t getStickerID() const noexcept
    {
        return getAttributeValue(113 /* "sticker slot 0 id" */);
    }

    bool hasTournamentEventID() const noexcept
    {
        return getTournamentEventID() != 0;
    }

    std::uint32_t getServiceMedalYear() const noexcept
    {
        return getAttributeValue(221 /* "prestige year" */);
    }

    bool isPaintable() const noexcept { return getCapabilities() & 1; /* ITEM_CAP_PAINTABLE */ }
    bool isPatchable() const noexcept { return getCapabilities() & (1 << 22); /* ITEM_CAP_CAN_PATCH */ }

    const char* getDefinitionName() const noexcept
    {
        return *reinterpret_cast<const char**>(getThis() + WIN32_LINUX(0x1DC, 0x2E0));
    }

    EconTool* getEconTool() const noexcept
    {
        return *reinterpret_cast<EconTool**>(getThis() + WIN32_LINUX(0x140, 0x1E8));
    }

    int getLoadoutSlot(csgo::Team team) const noexcept
    {
        if (team >= csgo::Team::None && team <= csgo::Team::CT)
            return reinterpret_cast<int*>(getThis() + WIN32_LINUX(0x28C, 0x3F4))[static_cast<int>(team)];
        return *reinterpret_cast<int*>(getThis() + WIN32_LINUX(0x268, 0x3BC));
    }
};

struct ItemListEntry {
    int itemDef;
    int paintKit;
    int paintKitSeed;
    float paintKitWear;
    std::uint32_t stickerKit;
    std::uint32_t musicKit;
    bool isNestedList;
    bool isUnusualList;
    PAD(2)

    auto weaponId() const noexcept
    {
        return static_cast<WeaponId>(itemDef);
    }
};

namespace csgo::pod { struct EconLootListDefinition; }

class EconLootListDefinition : public VirtualCallableFromPOD<EconLootListDefinition, csgo::pod::EconLootListDefinition> {
public:
    VIRTUAL_METHOD(const char*, getName, 0, (), ())
    VIRTUAL_METHOD(const UtlVector<ItemListEntry>&, getLootListContents, 1, (), ())

    bool willProduceStatTrak() const noexcept
    {
        return *reinterpret_cast<bool*>(getThis() + WIN32_LINUX(0x3C, 0x5C));
    }
};

struct EconItemQualityDefinition {
    int value;
    const char* name;
    unsigned weight;
    bool explicitMatchesOnly;
    bool canSupportSet;
    const char* hexColor;
};

struct AlternateIconData {
    UtlString simpleName;
    UtlString largeSimpleName;
    UtlString iconURLSmall;
    UtlString iconURLLarge;
    PAD(WIN32_LINUX(28, 48))
};

struct EconMusicDefinition {
    int id;
    const char* name;
    const char* nameLocalized;
    PAD(2 * sizeof(const char*))
    const char* inventoryImage;
};

class EconItemAttributeDefinition;

namespace csgo::pod { struct ItemSchema; }

class ItemSchema : public VirtualCallableFromPOD<ItemSchema, csgo::pod::ItemSchema> {
public:
    VIRTUAL_METHOD(csgo::pod::EconItemDefinition*, getItemDefinitionInterface, 4, (int id), (id))
    VIRTUAL_METHOD(const char*, getRarityName, 19, (uint8_t rarity), (rarity))
    VIRTUAL_METHOD(EconItemAttributeDefinition*, getAttributeDefinitionInterface, 27, (int index), (index))
    VIRTUAL_METHOD(int, getItemSetCount, 28, (), ())
    VIRTUAL_METHOD(csgo::pod::EconLootListDefinition*, getLootList, 31, (const char* name, int* index = nullptr), (name, index))
    VIRTUAL_METHOD(csgo::pod::EconLootListDefinition*, getLootList, 32, (int index), (index))
    VIRTUAL_METHOD(int, getLootListCount, 34, (), ())
    VIRTUAL_METHOD(csgo::pod::EconItemDefinition*, getItemDefinitionByName, 42, (const char* name), (name))

    auto getItemDefinitionInterface(WeaponId id) const noexcept
    {
        return getItemDefinitionInterface(static_cast<int>(id));
    }
};

namespace csgo::pod { struct ItemSystem; }

class ItemSystem : public VirtualCallableFromPOD<ItemSystem, csgo::pod::ItemSystem> {
public:
    VIRTUAL_METHOD(csgo::pod::ItemSchema*, getItemSchema, 0, (), ())
};

namespace csgo::pod { struct EconItem; }

class EconItem : private VirtualCallable {
public:
    EconItem(RetSpoofInvoker invoker, csgo::pod::EconItem* pod, const EconItemFunctions& econItemFunctions)
        : VirtualCallable{ invoker, std::uintptr_t(pod) }, functions{ econItemFunctions }
    {
    }

    using VirtualCallable::getThis;

#if IS_WIN32()
    VIRTUAL_METHOD(void, destructor, 0, (), (true))
#else
    VIRTUAL_METHOD(void, destructor, 1, (), ())
#endif

    void setDynamicAttributeValue(EconItemAttributeDefinition* attribute, void* value) const noexcept
    {
#if IS_WIN32()
        getInvoker().invokeThiscall<void>(getThis(), functions.setDynamicAttributeValue, attribute, value);
#else
        getInvoker().invokeCdecl<void>(functions.setDynamicAttributeValue, nullptr, getThis(), attribute, value);
#endif
    }

    void removeDynamicAttribute(EconItemAttributeDefinition* attribute) const noexcept
    {
        getInvoker().invokeThiscall<void>(getThis(), functions.removeDynamicAttribute, attribute);
    }

    [[nodiscard]] csgo::pod::EconItem* getPOD() const noexcept
    {
        return reinterpret_cast<csgo::pod::EconItem*>(getThis());
    }

private:
    const EconItemFunctions& functions;
};

class EconItemAttributeSetter {
public:
    explicit EconItemAttributeSetter(ItemSchema itemSchema)
        : itemSchema{ itemSchema } {}

    void setPaintKit(EconItem& econItem, float paintKit) noexcept { setAttributeValue(econItem, 6, &paintKit); }
    void setSeed(EconItem& econItem, float seed) noexcept { setAttributeValue(econItem, 7, &seed); }
    void setWear(EconItem& econItem, float wear) noexcept { setAttributeValue(econItem, 8, &wear); }
    void setAlternateIcon(EconItem& econItem, int alternateIcon) noexcept { setAttributeValue(econItem, 70, &alternateIcon); }
    void setTradableAfterDate(EconItem& econItem, std::uint32_t date) noexcept { setAttributeValue(econItem, 75, &date); }
    void setStatTrak(EconItem& econItem, int value) noexcept { setAttributeValue(econItem, 80, &value); }
    void setStatTrakType(EconItem& econItem, int type) noexcept { setAttributeValue(econItem, 81, &type); }
    void setTournamentID(EconItem& econItem, int id) noexcept { setAttributeValue(econItem, 137, &id); }
    void setTournamentStage(EconItem& econItem, int stage) noexcept { setAttributeValue(econItem, 138, &stage); }
    void setTournamentTeam1(EconItem& econItem, int team) noexcept { setAttributeValue(econItem, 139, &team); }
    void setTournamentTeam2(EconItem& econItem, int team) noexcept { setAttributeValue(econItem, 140, &team); }
    void setMusicID(EconItem& econItem, int musicID) noexcept { setAttributeValue(econItem, 166, &musicID); }
    void setCampaignCompletion(EconItem& econItem, std::uint32_t bits) noexcept { setAttributeValue(econItem, 185, &bits); }
    void setTournamentPlayer(EconItem& econItem, int player) noexcept { setAttributeValue(econItem, 223, &player); }
    void setSpecialEventID(EconItem& econItem, int id) noexcept { setAttributeValue(econItem, 267, &id); }
    void setIssueDate(EconItem& econItem, std::uint32_t date) noexcept { setAttributeValue(econItem, 222, &date); }
    void setSpraysRemaining(EconItem& econItem, std::uint32_t n) noexcept { setAttributeValue(econItem, 232, &n); }
    void setDropsAwarded(EconItem& econItem, std::uint32_t n) noexcept { setAttributeValue(econItem, 237, &n); }
    void setDropsRedeemed(EconItem& econItem, std::uint32_t n) noexcept { setAttributeValue(econItem, 240, &n); }
    void setItemsCount(EconItem& econItem, std::uint32_t itemsCount) noexcept { setAttributeValue(econItem, 270, &itemsCount); }
    void setModificationDate(EconItem& econItem, std::uint32_t date) noexcept { setAttributeValue(econItem, 271, &date); }
    void setCasketItemIdLow(EconItem& econItem, std::uint32_t itemIdLow) noexcept { setAttributeValue(econItem, 272, &itemIdLow); }
    void setCasketItemIdHigh(EconItem& econItem, std::uint32_t itemIdHigh) noexcept { setAttributeValue(econItem, 273, &itemIdHigh); }
    void removeCasketItemId(EconItem& econItem) noexcept { removeAttribute(econItem, 272); removeAttribute(econItem, 273); }

    void setStickerID(EconItem& econItem, int slot, int stickerID) noexcept
    {
        if (slot >= 0 && slot <= 5)
            setAttributeValue(econItem, 113 + 4 * slot, &stickerID);
    }

    void setStickerWear(EconItem& econItem, int slot, float wear) noexcept
    {
        if (slot >= 0 && slot <= 5)
            setAttributeValue(econItem, 114 + 4 * slot, &wear);
    }

private:
    void setAttributeValue(EconItem& econItem, int index, void* value) noexcept
    {
        if (const auto attribute = itemSchema.getAttributeDefinitionInterface(index))
            econItem.setDynamicAttributeValue(attribute, value);
    }

    void removeAttribute(EconItem& econItem, int index) noexcept
    {
        if (const auto attribute = itemSchema.getAttributeDefinitionInterface(index))
            econItem.removeDynamicAttribute(attribute);
    }

    ItemSchema itemSchema;
};

namespace csgo::pod { struct SharedObject; }
class SharedObject : public VirtualCallableFromPOD<SharedObject, csgo::pod::SharedObject> {
public:
    VIRTUAL_METHOD_V(int, getTypeID, 1, (), ())
};

namespace csgo::pod { struct SharedObjectTypeCache; }
class SharedObjectTypeCache : public VirtualCallableFromPOD<SharedObjectTypeCache, csgo::pod::SharedObjectTypeCache> {
public:
    VIRTUAL_METHOD_V(void, addObject, 1, (csgo::pod::SharedObject* object), (object))
    VIRTUAL_METHOD_V(void, removeObject, 3, (csgo::pod::SharedObject* object), (object))
};

class ClientSharedObjectCache : private VirtualCallable {
public:
    ClientSharedObjectCache(VirtualCallable virtualCallable, std::uintptr_t createBaseTypeCacheFn)
        : VirtualCallable{ virtualCallable }, createBaseTypeCache{ createBaseTypeCacheFn }
    {    
    }

    using VirtualCallable::getThis;

    csgo::pod::SharedObjectTypeCache* findBaseTypeCache(int classID) const noexcept
    {
        return getInvoker().invokeThiscall<csgo::pod::SharedObjectTypeCache*>(getThis(), createBaseTypeCache, classID);
    }

private:
    std::uintptr_t createBaseTypeCache;
};

namespace csgo::pod { struct InventoryManager; }
namespace csgo::pod { struct CSPlayerInventory; }

class InventoryManager : public VirtualCallableFromPOD<InventoryManager, csgo::pod::InventoryManager> {
public:
    VIRTUAL_METHOD_V(bool, equipItemInSlot, 20, (csgo::Team team, int slot, csgo::ItemId itemID, bool swap = false), (team, slot, itemID, swap))
    VIRTUAL_METHOD_V(csgo::pod::CSPlayerInventory*, getLocalInventory, 23, (), ())
    VIRTUAL_METHOD_V(void, updateInventoryEquippedState, 29, (csgo::pod::CSPlayerInventory* inventory, csgo::ItemId itemID, csgo::Team team, int slot, bool swap), (inventory, itemID, team, slot, swap))
};
