using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class CrosshairManager : MonoBehaviour {

    public Image TheCrosshair;

    public Sprite SafeSprite;
    public Sprite PadlockSprite;
    public Sprite GrabSprite;

    public Sprite LockedSprite;
    public Sprite PushSprite;
    public Sprite NeutralSprite;

    public Sprite ViewSprite;
    public Sprite GearSprite;
    

    public enum Sprites
	{
        Locked,
        Safe,
        Padlock,
        Push,
        Grab,
        View,
        Gear,
        Neutral
	}

    public Sprites CurrentSprite = Sprites.Neutral;

    void Start()
    {
        ChangeSprite(Sprites.Neutral);
    }

	public void ChangeSprite(Sprites TheSprite)
    {
        
        CurrentSprite = TheSprite;
        switch (CurrentSprite)
        {
            case Sprites.Locked:
                TheCrosshair.sprite = LockedSprite;
                break;
            case Sprites.Push:
                TheCrosshair.sprite = PushSprite;
                break;
            case Sprites.Neutral:
                TheCrosshair.sprite = NeutralSprite;
                break;
            case Sprites.Grab:
                TheCrosshair.sprite = GrabSprite;
                break;
            case Sprites.Safe:
                TheCrosshair.sprite = SafeSprite;
                break;
            case Sprites.Padlock:
                TheCrosshair.sprite = PadlockSprite;
                break;
            case Sprites.View:
                TheCrosshair.sprite = ViewSprite;
                break;
            case Sprites.Gear:
                TheCrosshair.sprite = GearSprite;
                break;
            default:
                break;
        }
    }
}
