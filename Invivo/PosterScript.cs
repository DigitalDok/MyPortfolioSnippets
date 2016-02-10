using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class PosterScript : MonoBehaviour
{
    public Sprite PosterSprite;
    private Image LePosterImage;
    private GameObject LePosterGO;
    internal bool ThisIsTheEnabledOne;
    public static bool IsInspectingSafeOrPoster;

    void Awake()
    {
        LePosterGO = GameObject.Find("PosterMaster");
        LePosterImage = GameObject.Find("Viewed Document Poster").GetComponent<Image>();
    }

    void Start()
    {
        LePosterGO.SetActive(false);

        Texture2D Y = (Texture2D)GetComponent<MeshRenderer>().material.mainTexture;
        PosterSprite = Sprite.Create(Y, new Rect(0, 0, Y.width, Y.height), new Vector2(0.5f, 0.5f));
    }


    void Update()
    {
        if (!ThisIsTheEnabledOne) return;
        if (Input.GetButtonDown("Cancel"))
        {
            if (LePosterGO.activeInHierarchy) LePosterGO.SetActive(false);
            physicWalk.CanMoveAtAll = true;
            ThisIsTheEnabledOne = false;
            IsInspectingSafeOrPoster = false;
        }
    }

    internal void LookAtPoster()
    {
        LePosterGO.SetActive(true);
        physicWalk.CanMoveAtAll = false;
        ThisIsTheEnabledOne = true;
        IsInspectingSafeOrPoster = true;
        LePosterImage.sprite = PosterSprite;
    }
}
