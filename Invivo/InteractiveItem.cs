using UnityEngine;
using System.Collections;

public class InteractiveItem : MonoBehaviour
{

    internal Shader NormalOne;
    internal Shader HoverOne;

    public enum ItemType
    {
        SpecialCase,
        Safe,
        Padlock,
        Poster,
        InventoryItem,
        Document,
        Switch
    }
    public ItemType ObjectType;

    // Leave this empty if NONE is selected from the enum above.
    public string Name;

    

    void Start()
    {
        AutoDetector();

        if(ObjectType != ItemType.Switch)
        {
            NormalOne = GetComponent<Renderer>().material.shader;
        }
        
        HoverOne = FlagManager.GetGlowShader();


       
    }

    private void AutoDetector()
    {
        if (GetComponent<PosterScript>())
        {
            ObjectType = ItemType.Poster;
        }
        if (GetComponent<InvItemScript>())
        {
            ObjectType = ItemType.InventoryItem;
        }
        if (GetComponent<DocumentScript>())
        {
            ObjectType = ItemType.Document;
        }
        if (GetComponent<PadlockScript>())
        {
            ObjectType = ItemType.Padlock;
        }
        if (GetComponent<SafeScript>())
        {
            ObjectType = ItemType.Safe;
        }
        if (GetComponent<SwitchScript>())
        {
            ObjectType = ItemType.Switch;
        }
    }
    
}
