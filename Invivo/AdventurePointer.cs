using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using UnityStandardAssets.Utility;

public class AdventurePointer : MonoBehaviour
{
    #region Fields
    RaycastHit hit;
    public Material Outliner;
    private Material LastKnownMaterial;
    private GameObject LastKnownGameObject;
    public static bool AmIHittingSomething;
    private bool WillBreak;

    public GameObject Tooltip;
    
    private Text ToolTipText;
    public InventoryManager MyInventory;
    public PrototypeEventController CurrentController;
    private bool CanPickUpSomething;
    private GameObject PickupableObject;

    public GameObject PickupRefPos;

    public static float RAYCAST_DISTANCE = 4f;
    public bool AmIRayCasting=false;
    public GameObject hititem;
    public RaycastHit pubhit;
    
    public CrosshairManager Crosshair;
    public GameObject ThePadlockUI;

    #endregion

    #region Initialize Method
    void Start()
    {
        ToolTipText = Tooltip.GetComponent<Text>();
    }
    #endregion
    
    #region Update Method
    void Update()
    {
        ResetingMaterials();
        Raycasting();
        Interaction();
        PickingUpAndThrowing();
    }

    private void PickingUpAndThrowing()
    {
        if (PickupableObject !=null)
        {
            // For as long as i keep the LMB down, the pickupable object stays in my grasp.
            if (Input.GetMouseButton(0) && PickupableObject.GetComponent<PickupObject>().ThrowOK)
            {
                ToolTipText.text = "Throw [RMB]";
                PickupableObject.transform.position = PickupRefPos.transform.position;
                PickupableObject.GetComponent<Rigidbody>().useGravity = false;
                PickupableObject.GetComponent<Rigidbody>().isKinematic = true;
                PickupableObject.transform.parent = gameObject.transform;
            }
            else //It just drops down when i release it.
            {
                
                PickupableObject.GetComponent<Rigidbody>().useGravity = true;
                PickupableObject.GetComponent<Rigidbody>().isKinematic = false;
                PickupableObject.transform.parent = null;
                PickupableObject = null;
            }

            //Added Value: If i press the RMB while i hold the object, i can actually throw it, if i am allowed to do so.
            if (Input.GetMouseButtonDown(1) && PickupableObject.GetComponent<PickupObject>().Throwable && PickupableObject.GetComponent<PickupObject>().ThrowOK)
            {
                PickupableObject.GetComponent<Rigidbody>().useGravity = true;
                PickupableObject.GetComponent<Rigidbody>().isKinematic = false;
                PickupableObject.transform.parent = null;
                PickupableObject.GetComponent<Rigidbody>().AddForce(transform.forward * 500);
                PickupableObject.GetComponent<PickupObject>().Throw();
                PickupableObject = null;
            }
        }
    }
    #endregion

    #region Adventure Item Management
    private void Interaction()
    {
        
        if (Input.GetButtonDown("Interact") && !InventoryManager.InventoryManipulation)
        {
            if (LastKnownGameObject)
            {

                switch (LastKnownGameObject.GetComponent<InteractiveItem>().ObjectType)
                {
                    case InteractiveItem.ItemType.SpecialCase:
                        break;
                    case InteractiveItem.ItemType.Switch:
                        LastKnownGameObject.GetComponent<SwitchScript>().Toggle();
                        break;
                    case InteractiveItem.ItemType.Safe:
                        LastKnownGameObject.GetComponent<SafeScript>().AttemptToOpen();
                        break;
                    case InteractiveItem.ItemType.Padlock:

                        if (LastKnownGameObject.GetComponent<PadlockScript>().KeyThatUnlocksMe == "")
                        {
                            LastKnownGameObject.GetComponent<PadlockScript>().OpenUpMasterAndPassValues();
                            return;
                        }
                        else
                        {
                            if (MyInventory.ItemsInSlots[MyInventory.IndexOfActivatedItem].ItemName == LastKnownGameObject.GetComponent<PadlockScript>().KeyThatUnlocksMe)
                            {
                                LastKnownGameObject.GetComponent<PadlockScript>().UnlockStuff();
                                return;
                            }
                            else
                            {
                                return;
                            }
                        }

                        break;
                    case InteractiveItem.ItemType.Poster:
                         LastKnownGameObject.GetComponent<PosterScript>().LookAtPoster();
                        break;
                    case InteractiveItem.ItemType.InventoryItem:
                        LastKnownGameObject.SetActive(false);
                        InvItemScript invItemScript = LastKnownGameObject.GetComponent<InvItemScript>();
                        MyInventory.AddItemToInventory(new Inventory_Item(invItemScript.itemName, invItemScript.itemDescription, invItemScript.itemSprite.name));
                        LastKnownGameObject = null; 
                        return;
                        break;
                    case InteractiveItem.ItemType.Document:
                        LastKnownGameObject.SetActive(false);
                        LastKnownGameObject.GetComponent<DocumentScript>().DocumentPickup();
                        LastKnownGameObject = null; 
                    return;   
                        break;
                    default:
                        break;
                }
            }
        }
    }

    private void ResetLastObjectToNormal()
    {
        LastKnownGameObject.GetComponent<Renderer>().material.shader = LastKnownGameObject.GetComponent<InteractiveItem>().NormalOne;
        LastKnownGameObject = null;
        AmIHittingSomething = false;
        ToolTipText.text = "";
    }

    private void Raycasting()
    {

        if(DragRigidbody.IsOnDragMode)
        {
            ToolTipText.text = "Throw [RMB]\nDrop [Release E]";
            return;
        }

        AmIRayCasting= Physics.Raycast(transform.position, transform.forward, out hit, RAYCAST_DISTANCE) && !InventoryManager.InventoryManipulation;
        pubhit = hit;

        if (AmIRayCasting) 
        {
            hititem = hit.collider.transform.gameObject;
            if (hititem.GetComponent<InteractiveItem>())
            {
                InteractiveItem item = hititem.GetComponent<InteractiveItem>();

                switch (item.ObjectType)
                {
                    case InteractiveItem.ItemType.SpecialCase:
                        break;
                    case InteractiveItem.ItemType.Switch:
                            
                            if (item.GetComponent<SwitchScript>().ToggleOnce && item.GetComponent<SwitchScript>().HasToggledOnce) return;
                            Crosshair.ChangeSprite(CrosshairManager.Sprites.Push);
                            AmIHittingSomething = true;
                            ToolTipText.text = "Pull Switch [E]";
                            LastKnownGameObject = hititem;
                            return;

                        break;
                    case InteractiveItem.ItemType.Safe:
                        if (item.gameObject.GetComponent<SafeScript>().IsPoweredOn)
                        {
                            Crosshair.ChangeSprite(CrosshairManager.Sprites.Safe);

                            if (item.gameObject.GetComponent<SafeScript>().IsUnlocked) return;
                            AmIHittingSomething = true;
                            ToolTipText.text = "Enter Code [E]";
                            LastKnownGameObject = hititem;
                            hititem.GetComponent<Renderer>().material.shader = item.HoverOne;
                            return;
                        }
                        else
                        {
                            ToolTipText.text = "Disabled Panel";
                        }
                        break;
                    case InteractiveItem.ItemType.Padlock:
                        Crosshair.ChangeSprite(CrosshairManager.Sprites.Padlock);

                        if (item.gameObject.GetComponent<PadlockScript>().IsUnlocked) return;

                        if (item.gameObject.GetComponent<PadlockScript>().KeyThatUnlocksMe == "")
                        {
                            AmIHittingSomething = true;
                            ToolTipText.text = "Try Combination [E]";
                            LastKnownGameObject = hititem;
                            hititem.GetComponent<Renderer>().material.shader = item.HoverOne;
                            return;
                        }
                        else
                        {
                            if (MyInventory.ItemsInSlots[MyInventory.IndexOfActivatedItem].ItemName == item.gameObject.GetComponent<PadlockScript>().KeyThatUnlocksMe)
                            {
                                AmIHittingSomething = true;
                                ToolTipText.text = "Unlock [E]";
                                LastKnownGameObject = hititem;
                                hititem.GetComponent<Renderer>().material.shader = item.HoverOne;
                                return;
                            }
                            else
                            {
                                ToolTipText.text = "Needs Key";
                                return;
                            }
                        }

                        

                        break;
                    case InteractiveItem.ItemType.Poster:
                        
                        Crosshair.ChangeSprite(CrosshairManager.Sprites.View);

                        AmIHittingSomething = true;
                        ToolTipText.text = "Read [E]";
                        LastKnownGameObject = hititem;
                        hititem.GetComponent<Renderer>().material.shader = item.HoverOne;
                        return;

                        break;
                    case InteractiveItem.ItemType.InventoryItem:

                        LastKnownGameObject.SetActive(false);
                        InvItemScript invItemScript = LastKnownGameObject.GetComponent<InvItemScript>();
                        MyInventory.AddItemToInventory(new Inventory_Item(invItemScript.itemName, invItemScript.itemDescription, invItemScript.itemSprite.name));
                        LastKnownGameObject = null;
                        return;

                        break;
                    case InteractiveItem.ItemType.Document:

                        Crosshair.ChangeSprite(CrosshairManager.Sprites.Grab);
                        AmIHittingSomething = true;
                        ToolTipText.text = item.gameObject.GetComponent<DocumentScript>().DocName +" [E]";
                        LastKnownGameObject = hititem;
                        hititem.GetComponent<Renderer>().material.shader = item.HoverOne;
                        return;

                        break;
                    default:
                        break;
                }
            }
            else if (hititem.CompareTag("Pickupable"))
            {
                Crosshair.ChangeSprite(CrosshairManager.Sprites.Push);
                
                ToolTipText.text = "Pick Up [Hold E]";
                return;
            }
            else if (hititem.CompareTag("DrawerPart"))
            {
                if (hititem.gameObject.transform.parent.gameObject.GetComponent<Drawer>().isLocked)
                {
                    Crosshair.ChangeSprite(CrosshairManager.Sprites.Locked);
                }
                else
                {
                    Crosshair.ChangeSprite(CrosshairManager.Sprites.Grab);
                }
               

                if (hititem.gameObject.transform.parent.gameObject.GetComponent<Drawer>().currentDrawerState == Drawer.DrawerState.Closed)
                {
                    ToolTipText.text = "Open [E]";
                }
                else if (hititem.gameObject.transform.parent.gameObject.GetComponent<Drawer>().currentDrawerState == Drawer.DrawerState.FullyOpened)
                {
                    ToolTipText.text = "Close [E]";
                }

                return;
            }
            else if (hititem.CompareTag("PortaPart"))
            {
                if (hititem.gameObject.transform.parent.gameObject.GetComponent<Door>().isLocked)
                {
                    Crosshair.ChangeSprite(CrosshairManager.Sprites.Locked);
                }
                else
                {
                    Crosshair.ChangeSprite(CrosshairManager.Sprites.Grab);
                }

                ToolTipText.text = "Open/Close [E + Movement]";
                return;
            }
           
        }

        ResetRayCasts();
        
    }

    public void ResetRayCasts()
    {
        AmIHittingSomething = false;
        ToolTipText.text = "";
        PickupableObject = null;
        hititem = null;
        Crosshair.ChangeSprite(CrosshairManager.Sprites.Neutral);
    }

    private void ResetingMaterials()
    {
        if (!AmIHittingSomething && LastKnownGameObject != null)
        {
            if(LastKnownGameObject.GetComponent<InteractiveItem>())
            {
                if(!LastKnownGameObject.GetComponent<SwitchScript>())
                LastKnownGameObject.GetComponent<Renderer>().material.shader = LastKnownGameObject.GetComponent<InteractiveItem>().NormalOne;
            }
            
            LastKnownGameObject = null;
        }
    }
    #endregion
}
