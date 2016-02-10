using UnityEngine;
using System.Collections;

public class OctoAI : MonoBehaviour {

    // This is the main logic of the Octopus AI Enemy, as seen in Water World.

    private Animator MyAnimator;

    public float OctoShooterTimer;
    private float CurOctoShooterTimer;

    public enum OctoStates
    {
        Idle,
        Slurp
    }
    public OctoStates MyState = OctoStates.Idle;
    public int ProcThreshold;
    public float SlurpVelocity;
    public GameObject SlurpiePos;

    private SpriteRenderer MyRenderer;

	void Start () 
    {
        MyAnimator = GetComponent<Animator>();
        MyRenderer = GetComponent<SpriteRenderer>();
	}


    public void ResetThisEnemy()
    {
        MyAnimator.SetBool("Slurpie", false);
        MyState = OctoStates.Idle;
        CurOctoShooterTimer = 0;
    }
	
	void Update () 
    {
        if (!MyRenderer.isVisible) return;
        switch (MyState)
        {
            case OctoStates.Idle:
                CurOctoShooterTimer += Time.deltaTime;
                if (CurOctoShooterTimer > OctoShooterTimer)
                {
                    MyAnimator.SetBool("Slurpie", true);
                    CurOctoShooterTimer = 0;
                    MyState = OctoStates.Slurp;
                }
                break;
            case OctoStates.Slurp:

                break;
            default:
                break;
        }
        
	}

    public void Shoot()
    {
        Director.GlobalDirector.PlaySFX_Aquatic(Director.SoundEffectsAquatic.OctoShoot);
        GameObject Slurpie = ProjPool.GlobalPool.GetFirstActiveArrow();
        Slurpie.gameObject.SetActive(true);
        Slurpie.gameObject.transform.position = SlurpiePos.transform.position;
        if (transform.localScale.x > 0)
            Slurpie.GetComponent<Rigidbody2D>().velocity = new Vector2(SlurpVelocity, 0);
        else
        {
            Vector3 Vec = Slurpie.transform.localScale;
            Vec.x *= -1;
            Slurpie.transform.localScale = Vec;
            Slurpie.GetComponent<Rigidbody2D>().velocity = new Vector2(-SlurpVelocity, 0);
        }


        int X = Random.Range(0, 100);

        if (X >= ProcThreshold)
        {
            MyAnimator.SetBool("Slurpie", false);
            MyState = OctoStates.Idle;
        }
    }
}
