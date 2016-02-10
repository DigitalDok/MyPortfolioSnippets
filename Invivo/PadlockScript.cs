using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using System;

public class PadlockScript : MonoBehaviour {

    public string NumberComboToUnlock;
    public bool IsUnlocked;
    public Door associatedDoorA;
    public Door associatedDoorB;

    // If it unlocks with a specific item, please write the item name EXACTLY as is, here.
    // If it requires code combination, leave this field blank ( "" )
    public string KeyThatUnlocksMe = "";
   
    public int AssociatedExplicitStoryDialogueIDIfAny;

    private AdventurePointer TheUIHolder;
    private DocumentViewer TheDocumentHolder;

    internal string CurrentSequence;

    //(PADLOCK SYSTEM SE LOCKBOX)
    // AssociatedLockbox = the lockbox gameobject that we want to deactivate on unlock.

    // (PADLOCK SYSTEM se DOOR)
    // Chains == the door chains, they get destroyed on unlock.
    // Padlock == the padlock model. It drops down with physics on unlock.

    public GameObject AssociatedLockbox;

    public GameObject Chains;
    public GameObject Padlock;

    public string AddedItemName;
    public string AddedItemDesc;
    public Sprite AddedItemSpriteName;

    
    public string AddedDocument = "";

    void Start()
    {
        TheUIHolder = GameObject.FindGameObjectWithTag("MainCamera").gameObject.GetComponent<AdventurePointer>();
        TheDocumentHolder = GameObject.Find("Document Viewer").GetComponent<DocumentViewer>();
        CurrentSequence = "0000";
    }

    
    private void UnlockDoors()
    {
        if (associatedDoorA != null)
        {
            associatedDoorA.isLocked = false;
        }

        if (associatedDoorB != null)
        {
            associatedDoorB.isLocked = false;
        }

    }

    internal void OpenUpMasterAndPassValues()
    {
        physicWalk.CanMoveAtAll = false;
        PosterScript.IsInspectingSafeOrPoster = true;

        TheUIHolder.ThePadlockUI.SetActive(true);
        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().CurrentPadlock = this;

        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().EnableFirstBtn();

        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().CorrectoSequence = NumberComboToUnlock;
        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().Num1 = Convert.ToInt16(CurrentSequence.Substring(0, 1));
        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().Num2 = Convert.ToInt16(CurrentSequence.Substring(1, 1));
        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().Num3 = Convert.ToInt16(CurrentSequence.Substring(2, 1));
        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().Num4 = Convert.ToInt16(CurrentSequence.Substring(3, 1));

        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().Num1Roll.text = CurrentSequence.Substring(0, 1);
        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().Num2Roll.text = CurrentSequence.Substring(1, 1);
        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().Num3Roll.text = CurrentSequence.Substring(2, 1);
        TheUIHolder.ThePadlockUI.GetComponent<PadlockMaster>().Num4Roll.text = CurrentSequence.Substring(3, 1);
    }

    internal void UnlockStuff()
    {
        IsUnlocked = true;
        GameObject.FindGameObjectWithTag("MainCamera").GetComponent<AdventurePointer>().ResetRayCasts();
        UnlockDoors();

        if(AssociatedLockbox!=null)
        {
            GameObject.FindGameObjectWithTag("Grandmaster").GetComponent<SubtitleManager>().ProgressThroughStoryExplicit(AssociatedExplicitStoryDialogueIDIfAny);

            if (AddedItemName != "")
                TheUIHolder.MyInventory.AddItemToInventory(new Inventory_Item(AddedItemName, AddedItemDesc, AddedItemSpriteName.name));

            if (AddedDocument != "")
                TheDocumentHolder.Act1DocumentsBool[TheDocumentHolder.DocumentMapper[AddedDocument]] = true;
            
            AssociatedLockbox.SetActive(false);

        }
        else
        {
            GetComponent<BoxCollider>().enabled = false;
            Chains.SetActive(false);
            Padlock.GetComponent<Rigidbody>().useGravity = true;
            Padlock.GetComponent<Rigidbody>().isKinematic = false;
        }
    }
}
