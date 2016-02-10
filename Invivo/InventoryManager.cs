using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using UnityEngine.EventSystems;

public class InventoryManager : MonoBehaviour
{
    #region Fields
    public GameObject InventoryUI;

    public GameObject[] InventorySlots = new GameObject[20];
    public GameObject[] Particles = new GameObject[20];
    public Inventory_Item[] ItemsInSlots = new Inventory_Item[20];
    public Sprite[] Sprites;
    public Sprite EmptySlot;


    public int IndexOfActivatedItem;

    public GameObject ItemNameText;
    public GameObject ItemDescText;

    public static bool InventoryManipulation = false;
    public bool DebugMode;

    #endregion


    void Start()
    {
        foreach (var item in InventoryUI.GetComponentsInChildren<CanvasRenderer>())
        {
            item.SetAlpha(0);
            if(item.gameObject.GetComponent<Button>())
            item.gameObject.GetComponent<Button>().interactable = false;
        }
    }

    void Update()
    {
        if (DebugMode)
        {
            if (Input.GetKeyUp(KeyCode.O)) AddItemToInventory(new Inventory_Item("The Bulldozer1", "It can bulldoze things.", "Knob"));

            if (Input.GetKeyUp(KeyCode.I)) AddItemToInventory(new Inventory_Item("The Bulldozer2", "It can bulldoze things.", "BG"));

            if (Input.GetKeyUp(KeyCode.P)) AddItemToInventory(new Inventory_Item("The Bulldozer3", "It can bulldoze things.", "BG"));

            if (Input.GetKeyUp(KeyCode.K)) RemoveItemFromInventory("The Bulldozer2");
        }

        if (Input.GetKeyUp(KeyCode.Escape))
        {
            IndexOfActivatedItem = GetFirstVacantSlot();
            if (InventoryManipulation)
                EnableCorrespondingSlot(IndexOfActivatedItem);
        }

        

        
        if (Input.GetKeyUp(KeyCode.Tab))
        {
            if (DocumentViewer.LeDocsOpen || PosterScript.IsInspectingSafeOrPoster) return;

            foreach (var item in InventoryUI.GetComponentsInChildren<CanvasRenderer>())
            {
                if (item.gameObject.GetComponent<Button>())
                    item.gameObject.GetComponent<Button>().interactable = false;

                item.SetAlpha(0);
                
            }

            ShutDownAllParticles();
            physicWalk.CanMoveAtAll = true;
        }

        if(Input.GetKeyDown(KeyCode.Tab))
        {
            if (DocumentViewer.LeDocsOpen || PosterScript.IsInspectingSafeOrPoster) return;

            foreach (var item in InventoryUI.GetComponentsInChildren<CanvasRenderer>())
            {
                item.SetAlpha(1);

                if (item.gameObject.GetComponent<Button>())
                item.gameObject.GetComponent<Button>().interactable = true;
            }
            EnableCorrespondingSlot(IndexOfActivatedItem);
            UpdateIcons();
            physicWalk.CanMoveAtAll = false;
        }
        InventoryManipulation = (Input.GetKey(KeyCode.Tab));
        InventoryManipulations();
    }



    public void AddItemToInventory(Inventory_Item Item)
    {
        if (GetFirstVacantSlot() == -1)
        {
            Debug.Log("Inventory is full!");
            return;

        }
        else
        {
            int NewIndex = GetFirstVacantSlot();
            ItemsInSlots[NewIndex] = new Inventory_Item(Item.ItemName, Item.ItemDescription, Item.ImageSource);
            ItemsInSlots[NewIndex].IndexInInventory = NewIndex;
        }
    }

    public int GetFirstVacantSlot()
    {
        for (int i = 0; i < ItemsInSlots.Length; i++)
        {
            if (ItemsInSlots[i] == null)
                return i;
        }
        return -1;
    }

    public void RemoveItemFromInventory(string NameToQuery)
    {
        int IndexToRemove = GetID_OfSlot(NameToQuery);
        if (IndexToRemove == -1)
        {
            Debug.Log("Item Was Not Found!");
            return;
        }
        else
        {
            RemoveItemFromInventory(IndexToRemove);
        }
    }

    public int GetID_OfSlot(string Name)
    {
        for (int i = 0; i < InventorySlots.Length; i++)
        {
            if (ItemsInSlots[i] != null)
                if (ItemsInSlots[i].ItemName == Name)
                    return ItemsInSlots[i].IndexInInventory;
        }
        return -1;
    }

    public void RemoveItemFromInventory(int IDToQuery)
    {
        ItemsInSlots[IDToQuery] = null;
        ShiftItemsUpUntilSlot(IDToQuery);
    }

    private void ShiftItemsUpUntilSlot(int IDToQuery)
    {
        for (int i = IDToQuery; i < 19; i++)
        {
            if (ItemsInSlots[i + 1] == null) break;
            ItemsInSlots[i] = new Inventory_Item(ItemsInSlots[i + 1].ItemName, ItemsInSlots[i + 1].ItemDescription, ItemsInSlots[i + 1].ImageSource);
            ItemsInSlots[i].IndexInInventory = i;
            ItemsInSlots[i + 1] = null;
        }
        ItemsInSlots[19] = null;
    }

    private void InventoryManipulations()
    {
        if (InventoryManipulation)
        {
            if (Input.GetKeyDown(KeyCode.DownArrow))
            {
                if (IndexOfActivatedItem < 10)
                {
                    IndexOfActivatedItem += 10;
                    EnableCorrespondingSlot(IndexOfActivatedItem);
                }
            }
            if (Input.GetKeyDown(KeyCode.UpArrow))
            {
                if (IndexOfActivatedItem > 9)
                {
                    IndexOfActivatedItem -= 10;
                    EnableCorrespondingSlot(IndexOfActivatedItem);
                }
            }
            if (Input.GetKeyDown(KeyCode.RightArrow))
            {
                if (IndexOfActivatedItem != 9 && IndexOfActivatedItem != 19)
                {
                    IndexOfActivatedItem++;
                    EnableCorrespondingSlot(IndexOfActivatedItem);
                }
            }
            if (Input.GetKeyDown(KeyCode.LeftArrow))
            {
                if (IndexOfActivatedItem != 0 && IndexOfActivatedItem != 19)
                {
                    IndexOfActivatedItem--;
                    EnableCorrespondingSlot(IndexOfActivatedItem);
                }
            }

        }
    }

    private void UpdateIcons()
    {
        for (int i = 0; i < InventorySlots.Length; i++)
        {
            if (ItemsInSlots[i] == null)
            {
                InventorySlots[i].GetComponent<Image>().enabled = false;
                continue;
            }

            InventorySlots[i].GetComponent<Image>().enabled = true;
            InventorySlots[i].GetComponent<Image>().sprite = GetSpriteByName(ItemsInSlots[i].ImageSource);
        }
    }

    private Sprite GetSpriteByName(string Name)
    {
        for (int i = 0; i < Sprites.Length; i++)
        {
            if (Sprites[i] != null)
                if (Sprites[i].name == Name) return Sprites[i];
        }
        return null;
    }

    public void EnableCorrespondingSlot(int Index)
    {

        if (ItemsInSlots[Index]!=null)
        {
            ItemNameText.GetComponent<Text>().text = ItemsInSlots[Index].ItemName;
            ItemDescText.GetComponent<Text>().text = ItemsInSlots[Index].ItemDescription;
        }
        else
        {
            ItemNameText.GetComponent<Text>().text = "Empty Slot";
            ItemDescText.GetComponent<Text>().text = "This is an empty Inventory Slot";
        }
        Debug.Log(":D");
        InventorySlots[Index].transform.parent.transform.gameObject.GetComponent<Button>().Select();
        
    }

    private void ShutDownAllParticles()
    {
        for (int i = 0; i < Particles.Length; i++)
        {
            Particles[i].SetActive(false);
        }
    }

    public void OnMouseClick(int Index)
    {
        IndexOfActivatedItem = Index - 1;
        EnableCorrespondingSlot(Index-1);
    }

    public void GetFocus()
    {

    }


}
