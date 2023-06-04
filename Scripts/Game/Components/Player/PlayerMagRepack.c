class MR_PlayerMagRepackComponentClass : ScriptComponentClass
{
}

class MR_PlayerMagRepackComponent : ScriptComponent
{
    SCR_CharacterControllerComponent controller_;
    SCR_InventoryStorageManagerComponent inventoryManager_;

    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);	
        
        if (SCR_Global.IsEditMode())
        {
            return;
        }
        
        SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast( owner.FindComponent(SCR_CharacterControllerComponent) );		
                
        if (!controller)
        {
            return;
        }

        controller.m_OnControlledByPlayer.Insert(this.OnControlledByPlayer);
        controller.m_OnPlayerDeath.Insert(this.OnPlayerDeath);
        controller_ = controller;

    }

    protected void OnControlledByPlayer(IEntity owner, bool controlled)
    {
        if (!controlled)
        {
            UnregisterInputs();
            return;
        }

        if (!owner)
        {
            return;
        }
        RegisterInputs();

        PlayerController playerController = GetGame().GetPlayerController();
        if (!playerController ) { return; }

        ChimeraCharacter player = ChimeraCharacter.Cast(playerController.GetControlledEntity());
        if (!player) { return; }
        inventoryManager_ = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));

    }

	protected void OnPlayerDeath(SCR_CharacterControllerComponent charController, IEntity instigator)
    {
        if (!charController || charController != controller_)
        {
            return;
        }
        UnregisterInputs();
    }

    void RegisterInputs()
    {
        InputManager inputManager = GetGame().GetInputManager();
        Print("Registering inputs!");
        if (!inputManager)
        {
            return;
        }
        Print("Got input manager to register");
        inputManager.AddActionListener("MagRepackOpenMenu", EActionTrigger.PRESSED, RepackAllMags);
        Print("Added action listener");
    }

    void UnregisterInputs()
    {
        Print("Unregistering inputs!");
        InputManager inputManager = GetGame().GetInputManager();
        Print("Got input manager to unregister");
        if (!inputManager)
        {
            return;
        }
        Print("Removing action listener");

        inputManager.RemoveActionListener("MagRepackOpenMenu", EActionTrigger.PRESSED, RepackAllMags);
    }

    BaseMagazineComponent ToMagazine(IEntity item)
    {
        return BaseMagazineComponent.Cast(item.FindComponent(BaseMagazineComponent));
    }

    bool IsRepackableMag(IEntity item)
    {
        BaseMagazineComponent magazine = ToMagazine(item);
        BaseMuzzleComponent parentMuzzle = BaseMuzzleComponent.Cast(item.GetParent().FindComponent(BaseMuzzleComponent));
        bool isInWeapon = parentMuzzle != null;
        bool isUsed = magazine && magazine.GetAmmoCount() < magazine.GetMaxAmmoCount();
        return magazine && !isInWeapon && isUsed;
    }

    void GetRepackableMagazines(notnull out array<IEntity> magItems)
    {
        array<IEntity> items = {};
        inventoryManager_.GetItems(items);
        foreach (IEntity item: items)
        {
            if (IsRepackableMag(item))
            {
                magItems.Insert(item);
            }
        }
    }

    bool CanRepack()
    {
        array<IEntity> magItems = {};
        GetRepackableMagazines(magItems);
        return magItems.Count() >= 2;
    }

    void RepackAllMags()
    {
        while (CanRepack())
        {
            RepackMags();
        }
    }

    void RepackMags()
    {
        if (!inventoryManager_) { return; }
        array<IEntity> magItems = {};
        GetRepackableMagazines(magItems);
        if (magItems.Count() < 2)
        {
            Print("Nothing to repack!");
            return;
        }

        BaseMagazineComponent minMag = GetMinMagazine(magItems);
        BaseMagazineComponent maxMag = GetMaxMagazine(magItems);
        int targetMagRemaining = maxMag.GetMaxAmmoCount() - maxMag.GetAmmoCount();
        int amountToTransfer = Math.Min(minMag.GetAmmoCount(), targetMagRemaining);
        maxMag.SetAmmoCount(maxMag.GetAmmoCount() + amountToTransfer);
        minMag.SetAmmoCount(minMag.GetAmmoCount() - amountToTransfer);
        foreach (IEntity item: magItems)
        {
            if (ToMagazine(item).GetAmmoCount() == 0)
            {
                bool success = inventoryManager_.TryRemoveItemFromInventory(item);
            }
        }

    }

    BaseMagazineComponent GetMinMagazine(notnull array<IEntity> items)
    {
        if (items.Count() < 1)
        {
            return null;
        }

        int minSeen = 1000*1000;
        BaseMagazineComponent minMag = null;
        foreach (IEntity item: items)
        {
            BaseMagazineComponent mag = ToMagazine(item);
            int count = mag.GetAmmoCount();
            if (count < minSeen)
            {
                minSeen = count;
                minMag = mag;
            }
        }
        return minMag;
    }

    BaseMagazineComponent GetMaxMagazine(notnull array<IEntity> items)
    {
        if (items.Count() < 1)
        {
            return null;
        }

        int maxSeen = -1;
        BaseMagazineComponent maxMag = null;
        foreach (IEntity item: items)
        {
            BaseMagazineComponent mag = ToMagazine(item);
            int count = mag.GetAmmoCount();
            if (count > maxSeen)
            {
                maxSeen = count;
                maxMag = mag;
            }
        }
        return maxMag;
    }
}
