using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class SafeScript : MonoBehaviour
{
    public string NumberComboToUnlock;
    
    public bool IsUnlocked;

    public bool IsPoweredOn;

    // DESIGNER TIP
    // You can leave these empty, but if you want this panel to unlock doors, then drag and drop them here!

    public Door associatedDoorA;
    public Door associatedDoorB;

    #region References

    private GameObject TheSafeIcon;
    private GameObject TheCodeText;
    private GameObject RedLightOn;
    private GameObject RedLightOff;
    private GameObject GreenLightOn;
    private GameObject GreenLightOff;
    private Button FirstButton;
    
    #endregion


    private float CurTimer;
    public float Timer;

    public int ExplicitIDIfAny = -1;

    private bool ThisIsTheEnabledOne;
    
    private bool BlockIncomingInput;

    void Awake()
    {
        TheSafeIcon = GameObject.Find("Safe - Master");
        TheCodeText = GameObject.Find("CodeText");

        RedLightOn = GameObject.Find("RedLightOn");
        RedLightOff = GameObject.Find("RedLightOff");
        GreenLightOn = GameObject.Find("GreenLightOn");
        GreenLightOff = GameObject.Find("GreenLightOff");

        FirstButton = GameObject.Find("Panel - First Button").GetComponent<Button>();
    }

    void Start()
    {
        TheSafeIcon.SetActive(false);
    }

    void Update()
    {

        if (!ThisIsTheEnabledOne) return;
        if (Input.GetButtonDown("Cancel"))
        {
            if (TheSafeIcon.activeInHierarchy) TheSafeIcon.SetActive(false);
            physicWalk.CanMoveAtAll = true;
            ThisIsTheEnabledOne = false;
            PosterScript.IsInspectingSafeOrPoster = false;
        }

        if (TheCodeText.GetComponent<Text>().text.Length == 4)
        {
            CurTimer += Time.deltaTime;
            if (CurTimer > Timer)
            {
                CurTimer = 0;

                if (TheCodeText.GetComponent<Text>().text == NumberComboToUnlock)
                {
                    Debug.Log("Correcto");
                    BlockIncomingInput = true;
                    CorrectEffects();
                    StartCoroutine(IWin());
                }
                else
                {
                    Debug.Log("SKATE");
                }
                TheCodeText.GetComponent<Text>().text = "";
            }
        }

    }

    public IEnumerator IWin()
    {
        yield return new WaitForSeconds(2);
        if (TheSafeIcon.activeInHierarchy) TheSafeIcon.SetActive(false);
        physicWalk.CanMoveAtAll = true;
        ThisIsTheEnabledOne = false;
        IsUnlocked = true;
        GameObject.FindGameObjectWithTag("MainCamera").GetComponent<AdventurePointer>().ResetRayCasts();
        UnlockDoors();
        GetComponent<BoxCollider>().enabled = false;

        RedLightOff.SetActive(false);
        RedLightOn.SetActive(true);

        GreenLightOff.SetActive(true);
        GreenLightOn.SetActive(false);
    }

    internal void AttemptToOpen()
    {
        TheSafeIcon.SetActive(true);
        physicWalk.CanMoveAtAll = false;
        ThisIsTheEnabledOne = true;
        PosterScript.IsInspectingSafeOrPoster = true;
        SelectFirstButton();
        TheCodeText.GetComponent<Text>().text = "";
    }

    public void AddCharacter(string Character)
    {
        if (BlockIncomingInput) return;

        if (TheCodeText.GetComponent<Text>().text.Length == 4)
            return;


        TheCodeText.GetComponent<Text>().text += Character;

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

        if (ExplicitIDIfAny != -1)
            GameObject.FindGameObjectWithTag("Grandmaster").GetComponent<SubtitleManager>().ProgressThroughStoryExplicit(ExplicitIDIfAny);
    }

    public void SelectFirstButton()
    {
        FirstButton.Select();
    }

    public void CorrectEffects()
    {
        RedLightOff.SetActive(true);
        RedLightOn.SetActive(false);

        GreenLightOff.SetActive(false);
        GreenLightOn.SetActive(true);
    }
}
