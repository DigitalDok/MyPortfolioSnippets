using UnityEngine;
using System.Collections;
using System;

public class CharacterControlScript : MonoBehaviour
{

    #region Global Character Variables

    public float MoveSpeed;
    public float JumpHeight;

    public bool isFacingRight = true;

    public LayerMask GroundLayers;
    public float GrounderRadius;
    public Transform Grounder;
    public bool isGrounded;

    internal Rigidbody2D ourRigid;
    internal Animator ourAnimator;
    private float Pressure;
    public bool CanAttack =true;
    public bool HasDied;

    public float VelSpeed;
    public GameObject Shooter;
    private bool Childed;
    public int Lives;
    public int Coins;

    private SpriteRenderer myrenderer;
    public GameObject Win;
    public bool IsOnSlidingLand;
    public float RaycastRange;


    #endregion

    #region World-Dependant Character Variables

    public bool IsFish;
    public bool IsBirdie;
    private short Pressure2;

    public bool IsSlippery;
    public LayerMask WallLayer;
    private float Speed;
    public float Rate;
    private bool startCounter;
    public float ourtimer;

    public Vector2 PyroForceX;
    public ForceMode2D PyroForceY;

    public float JetPackSpeed;
    public float BirdieAttackCD;
    private float CurBirdie;
    private float CurTimerInputLock;

    #endregion
    
    void Start () 
    {
        myrenderer = GetComponent<SpriteRenderer>();
        ourAnimator = GetComponent<Animator>();
        ourRigid = GetComponent<Rigidbody2D>();
	}
	
	void Update () 
    {
        // Enable Winning Animation if we have won.
        if(!Application.loadedLevelName.Contains("Tutorial"))
        if (Director.GlobalDirector.GameWon)
        {
            myrenderer.enabled = false;
            Win.SetActive(true);
        }

        // Enable input only when not paused.
        if (!PauseThing.isPaused && !PauseFunctionality.InputLocked)
        {
            UserInput();
        }
        else if (PauseFunctionality.InputLocked)
        {
            CurTimerInputLock += Time.deltaTime;
            if (CurTimerInputLock > 0.1f)
            {
                CurTimerInputLock = 0;
                PauseFunctionality.InputLocked = false;
            }
        }

        // Periodic Animation Control
        AnimationControl();
	}

    private void AnimationControl()
    {
        if (HasDied)
        {
            if(!Application.loadedLevelName.Contains("Tutorial"))
            if (Director.GlobalDirector.GameWon)
            {
                ourAnimator.SetInteger("Speed", 0);
                ourAnimator.SetBool("Moving", false);
                return;
            }
        }

        // If not in Water Land and not in Air Land (Must be in Nature / Fire / Ice / Dark) --> In these areas no special movement rules occur.
        if (!IsFish && !IsBirdie)
        {
            int Speed = (Mathf.Abs(Pressure) > 0 && !Input.GetButton("Fire1") && !ourAnimator.GetBool("HasAttacked") && !ourAnimator.GetBool("HasJumped") && (isGrounded || IsOnSlidingLand)) ? 1 : 0;
            ourAnimator.SetInteger("Speed", Speed);

            if(startCounter)
            {
                ourAnimator.SetBool("HasJumped", true);
            }
            else if(IsOnSlidingLand || isGrounded || Childed || ourAnimator.GetBool("Moving"))
            {
                ourAnimator.SetBool("HasJumped", false);
            }
            else if (ourRigid.velocity.y != 0 && !Physics2D.Raycast(transform.position, -Vector2.up, RaycastRange*1.5f, LayerMask.GetMask("SlidingLand")))
            {
                ourAnimator.SetBool("HasJumped", true);
            }
            else
            {
                ourAnimator.SetBool("HasJumped", false);
            }
        }
        else if(IsFish)
        {
            ourAnimator.SetBool("Moving", (Pressure!=0 || Pressure2!=0));
        }
        else
        {
            int Speed = (Mathf.Abs(Pressure) > 0 && !Input.GetButton("Fire1") && !ourAnimator.GetBool("HasAttacked") && !ourAnimator.GetBool("HasJumped") && (isGrounded || IsOnSlidingLand)) ? 1 : 0;
            ourAnimator.SetInteger("Speed", Speed);

            ourAnimator.SetBool("HasJumped", Input.GetButton("Jump") && !ourAnimator.GetBool("HasAttacked"));
        }
    }

    // Determine which input theme is going to be used.
    private void UserInput()
    {
        if (!Application.loadedLevelName.Contains("Tutorial"))
        if (HasDied || Director.GlobalDirector.GameWon) return;


        if (!IsFish && !IsBirdie)
        {
            NormalInput();
        }
        else if(IsBirdie)
        {
            BirdieInput();
        }
        else
        {
            FishInput();
        }
        
    }


    // World 5 - Specific Movement.
    // Our character glides in mid air, ascends and descends as we keep the jump button pressed.
    private void BirdieInput()
    {
        Pressure = Convert.ToInt16(Input.GetAxisRaw("Horizontal"));
        CurBirdie += Time.deltaTime;

        if (Input.GetButton("Fire1") && !ourAnimator.GetBool("HasAttacked")  && CurBirdie > BirdieAttackCD)
        {
            ourAnimator.SetBool("HasAttacked", true);
            ourRigid.velocity = new Vector2(MoveSpeed * Pressure, -0.5f);
        }


        if (Pressure != 0 && !ourAnimator.GetBool("HasAttacked") && !Input.GetButton("Fire1"))
        {
            ourRigid.velocity = new Vector2(MoveSpeed * Pressure, ourRigid.velocity.y);
            if (EligibleToFlip(Pressure))
            {
                Flip();
            }
        }
        else
        {

            ourRigid.velocity = new Vector2(MoveSpeed * Pressure, ourRigid.velocity.y);
        }

        if(Input.GetButton("Jump") && !ourAnimator.GetBool("HasAttacked"))
        {
            ourRigid.velocity = new Vector2(MoveSpeed * Pressure, JetPackSpeed);
        }
    }


    // Normal Movement - Worlds 1,3,4,6.
    // Our character normally walks, jumps and falls down.
    private void NormalInput()
    {
        Pressure = Convert.ToInt16(Input.GetAxisRaw("Horizontal"));
        if (Input.GetButton("Fire1") && !ourAnimator.GetCurrentAnimatorStateInfo(0).IsName("attack 2") && (isGrounded || (IsOnSlidingLand || Physics2D.Raycast(transform.position, -Vector2.up, RaycastRange * 1.5f, LayerMask.GetMask("SlidingLand"))) || Childed) && ourRigid.gravityScale == 0)
        {
            ourAnimator.SetBool("HasAttacked", true);
            if (!IsSlippery)
                ourRigid.velocity = new Vector2(0, ourRigid.velocity.y);
            else
                SlipCalculator();
        }
        else if (Pressure != 0 && !ourAnimator.GetBool("HasAttacked") && !Input.GetButton("Fire1"))
        {
            SlipperyHelper();

            ourRigid.velocity = new Vector2(MoveSpeed * Pressure, ourRigid.velocity.y);

            if (EligibleToFlip(Pressure))
            {
                Flip();
            }
        }
        else
        {
            if (!IsSlippery && !Childed)
            {
                
                ourRigid.velocity = new Vector2(0, ourRigid.velocity.y);
            }
            else
            {
                SlipCalculator();
            }
        }

        if (Input.GetButtonDown("Jump") && (isGrounded || IsOnSlidingLand || Childed) && !ourAnimator.GetBool("HasAttacked") && !ourAnimator.GetBool("HasJumped") && !startCounter)
        {
            if (Childed)
            {
                transform.parent = null;
                Childed = false;
            }

            startCounter = true;
            isGrounded = false;
            IsOnSlidingLand = false;
            ourRigid.gravityScale = 3;

            ourRigid.velocity = new Vector2(ourRigid.velocity.x, 0);
            ourRigid.AddForce(new Vector2(0, JumpHeight));
        }

        HelpingTimerForAnimation();
    }

    // World 2 - Specific Movement.
    // Our character is a fish, can swim in all directions and there is no gravity.
    private void FishInput()
    {
        Pressure = Convert.ToInt16(Input.GetAxisRaw("Horizontal"));
        Pressure2 = Convert.ToInt16(Input.GetAxisRaw("Vertical"));

        if (Input.GetButton("Fire1") && !ourAnimator.GetBool("HasAttacked"))
        {
            ourAnimator.SetBool("HasAttacked", true);
            ourRigid.velocity = new Vector2(0, 0);
        }
        else if ((Pressure != 0 || Pressure2 != 0) && !ourAnimator.GetBool("HasAttacked") && !Input.GetButton("Fire1"))
        {
            ourRigid.velocity = new Vector2(MoveSpeed * Pressure, MoveSpeed * Pressure2);
            if (EligibleToFlip(Pressure))
            {
                Flip();
            }
        }
        else
        {
            ourRigid.velocity = new Vector2(0, 0);
        }
    }


    private void HelpingTimerForAnimation()
    {
        if (startCounter)
        {
            ourtimer += Time.deltaTime;
            if (ourtimer > 0.360948f)
            {
                startCounter = false;
                ourtimer = 0;
            }
        }
    }

    
    // World 3 Specific Functions - The Ground is slippery.
    private void SlipCalculator()
    {
        if (ourRigid.velocity.x > 0)
        {
            Speed -= Rate;
            ourRigid.velocity = new Vector2(Speed, ourRigid.velocity.y);
            if (ourRigid.velocity.x <= 0)
            {
                ourRigid.velocity = new Vector2(0, ourRigid.velocity.y);
                Speed = 0;
            }
        }
        if (ourRigid.velocity.x < 0)
        {
            Speed += Rate;
            ourRigid.velocity = new Vector2(Speed, ourRigid.velocity.y);
            if (ourRigid.velocity.x >= 0)
            {
                ourRigid.velocity = new Vector2(0, ourRigid.velocity.y);
                Speed = 0;

            }
        }
    }

    private void SlipperyHelper()
    {
        if (IsSlippery)
        {
            if (Pressure > 0)
                Speed = MoveSpeed;
            else
                Speed = -MoveSpeed;
        }
    }

    void FixedUpdate()
    {
        if (!IsFish)
        {

            if (!startCounter && !HasDied)
            {
                isGrounded = Physics2D.OverlapCircle(Grounder.position, GrounderRadius, GroundLayers) || Physics2D.OverlapCircle(Grounder.position, GrounderRadius, WallLayer); ;
                IsOnSlidingLand = Physics2D.Raycast(transform.position, -Vector2.up, RaycastRange, LayerMask.GetMask("SlidingLand"));
                ourRigid.gravityScale = (IsOnSlidingLand || Childed || isGrounded) ? 0 : 3;

            }

            if (!HasDied)
            {
                if (IsBirdie)
                {
                    if (ourRigid.velocity.y < -JetPackSpeed) ourRigid.velocity = new Vector2(ourRigid.velocity.x, -JetPackSpeed);
                }
                else
                {
                    if (ourRigid.velocity.y < -8) ourRigid.velocity = new Vector2(ourRigid.velocity.x, -8);
                }

                if (!isGrounded && !IsOnSlidingLand && !Physics2D.Raycast(transform.position, -Vector2.up, RaycastRange * 1.5f, LayerMask.GetMask("SlidingLand"))) ResetFireAnim();
            }
        }
    }

    private void Flip()
    {
        Vector3 CurrentScale = transform.localScale;
        CurrentScale.x *= -1;

        transform.localScale = CurrentScale;
        isFacingRight = !isFacingRight;
        
    }

    private bool EligibleToFlip(float Pressure)
    {
        return (Pressure < 0 && isFacingRight) || (Pressure > 0 && !isFacingRight);
    }

    // Unique Firing Mechanics depending on the world.
    // World 1 - Horizontal Star Blast.
    // World 2 - Short living Bubble.
    // World 3 - Trajectory based Snowball (Going down).
    // World 4 - Trajectory based fireball (Arcs upwards).
    // World 5 - Horizontal Air Blast.
    // World 6 - Horizontal holy blast with light source.

    public void Fire()
    {
        if(IsBirdie)
        {
            if (!Application.loadedLevelName.Contains("Tutorial"))
            Director.GlobalDirector.PlaySFX(Director.SoundEffects.ProjShoot);
            GameObject theprojectile = ProjPool.GlobalPool.GetFirstActiveOne();
            theprojectile.SetActive(true);
            theprojectile.transform.position = Shooter.transform.position;
            var DirMultiplier = isFacingRight ? 1 : -1;
            theprojectile.GetComponent<Rigidbody2D>().velocity = new Vector2(VelSpeed * DirMultiplier, 0);
            CurBirdie = 0;
        }
        else
        {
            
            if(!Application.loadedLevelName.Contains("Tutorial"))
            Director.GlobalDirector.PlaySFX(Director.SoundEffects.ProjShoot);
            



            GameObject theprojectile = ProjPool.GlobalPool.GetFirstActiveOne();
            theprojectile.SetActive(true);

            
          
            theprojectile.transform.position = Shooter.transform.position;

            var DirMultiplier = isFacingRight ? 1 : -1;
            theprojectile.GetComponent<Rigidbody2D>().velocity = new Vector2(VelSpeed * DirMultiplier, 0);

            if (theprojectile.name.Contains("HolyProjectile"))
            {
               // theprojectile.GetComponent<Light>().enabled = true;
            }
            else
            {
                theprojectile.GetComponent<rotate>().RotSpeed *= (isFacingRight) ? -1 : 1;
            }

            
            
            

            if (IsSlippery)
                theprojectile.GetComponent<DecreaseHorizVelocity>().GetSpeed();
        }
    }

    public void FirePyro()
    {
        if (!Application.loadedLevelName.Contains("Tutorial"))
        Director.GlobalDirector.PlaySFX_Fire(Director.SoundEffectsFire.Fireball_Player);
        GameObject theprojectile = ProjPool.GlobalPool.GetFirstActiveOne();
        theprojectile.SetActive(true);
        theprojectile.transform.position = Shooter.transform.position;
        var DirMultiplier = isFacingRight ? 1 : -1;
        theprojectile.GetComponent<Rigidbody2D>().AddForce(new Vector2(PyroForceX.x * DirMultiplier,PyroForceX.y));
        
    }

    public void FireBubble()
    {
        if(!Application.loadedLevelName.Contains("Tutorial"))
        Director.GlobalDirector.PlaySFX(Director.SoundEffects.ProjShoot);
        
        GameObject theprojectile = ProjPool.GlobalPool.GetFirstActiveOne();
        theprojectile.SetActive(true);

        theprojectile.GetComponent<Projectile>().BubblePreparer();
        theprojectile.GetComponent<Projectile>().IsBubble = true;
        theprojectile.transform.localScale = Vector3.zero;
        theprojectile.transform.position = Shooter.transform.position;
        var DirMultiplier = isFacingRight ? 1 : -1;
        theprojectile.GetComponent<Rigidbody2D>().velocity = new Vector2(VelSpeed * DirMultiplier, 0);
        
    }

    // Used as a helper to reset animation system.
    public void ResetFireAnim()
    {
        ourAnimator.SetBool("HasAttacked", false);
        CanAttack = true;
    }



    // If we collide with an enemy, a hazard or a bullet or anything evil or cute-looking at all, we die. Provided we haven't won the game yet.
    void OnTriggerEnter2D(Collider2D col)
    {
        if (col.CompareTag("Hazard") || col.CompareTag("Monster") && !HasDied && !Director.GlobalDirector.GameWon && NavigationManager.Dok.CurrentState == NavigationManager.ManagerStates.Inactive)
        {
            if (HasDied || Director.GlobalDirector.GameWon) return;
            HasDied = true;
            ProcDeath();
        }
    }

    // Mid-Air Jumping, Rotation, Death Music and fall down away from screen. Make sure no collisions happen!
    public void ProcDeath()
    {
        if (Childed)
        {
            transform.parent = null;
            Childed = false;

            ourRigid.gravityScale = 3;
        }
        Director.GameMusic.volume = 0;
        Director.GlobalDirector.PlaySFX(Director.SoundEffects.Lose);
        transform.GetChild(0).GetComponent<CircleCollider2D>().enabled = false;
        
        ourRigid.constraints = RigidbodyConstraints2D.None;
        if (!IsFish)
        {
            ourRigid.AddTorque(500);
            ourRigid.AddForce(new Vector2(0, JumpHeight * 1.5f));
            ourRigid.velocity = new Vector2(-ourRigid.velocity.x / 3, ourRigid.velocity.y);
            GetComponent<BoxCollider2D>().enabled = false;
        }
        else
        {
            ourRigid.AddTorque(500);
            ourRigid.velocity = new Vector2(0, -5);
        }
            
        
        if(IsFish)
            GetComponent<CircleCollider2D>().enabled = false;
        Director.HasDeathOccured = true;
        ourAnimator.SetBool("HasDied", true);
        Director.GlobalDirector.StartFading();
    }

    IEnumerator DieIn2Sec()
    {
        yield return new WaitForSeconds(2);
        ourAnimator.SetBool("HasTouchedGround", true);
        ourAnimator.SetBool("HasDied", false);
    }

    // Helper methods to support moving platform integration.
    // The character must be childed to the platforms as long as he stays on them.
    // That way, he can move along with them.

    void OnCollisionEnter2D(Collision2D col)
    {
        if (col.gameObject.CompareTag("Platform") && ourRigid.velocity.y <= 0)
        {
                transform.parent = col.transform;
                Childed = true;
        }
    }

    void OnCollisionExit2D(Collision2D col)
    {
        if (col.gameObject.CompareTag("Platform"))
        {
            transform.parent = null;
            Childed = false;
        }
        
    }

    // Resets player after death
    internal void ResetPlayer()
    {
        HasDied = false;
        transform.localRotation = Quaternion.identity;
        ourRigid.constraints = RigidbodyConstraints2D.FreezeRotation;
        ourRigid.velocity = Vector2.zero;

        if (IsFish)
        {
            GetComponent<CircleCollider2D>().enabled = true;
        }
        else
        {
            GetComponent<BoxCollider2D>().enabled = true;
        }
        transform.GetChild(0).GetComponent<CircleCollider2D>().enabled = true;
        ourAnimator.SetBool("HasDied", false);
        ourAnimator.SetBool("HasAttacked", false);
        ourAnimator.SetBool("HasTouchedGround", false);
        CanAttack = true;
    }
}
