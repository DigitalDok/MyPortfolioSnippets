using UnityEngine;
using System.Collections;

public class ResetLevelEnemy : MonoBehaviour {

    
    /// <summary>
    /// This script ensures that monsters are reset back to normal once the player dies and respawns, they also tint themselves when hit.
    /// Also contains custom hard-coded logic that changes depending on the monster, or boss holding this script.
    /// </summary>


    public int MaxHP;
    public Vector3 RestoreInitialPos;

    public float CurrentHP;
    public bool HasDied;
    public bool DamageTinter;
    private float lerpDuration = 0.05f;
    private Color minRange = Color.red;
    private Color maxRange = Color.white;
    private SpriteRenderer myRenderer;
    private float CurLerpTimer;
    private float LerpTimer = 0.5f;

    void Start()
    {
        if (gameObject.name.Contains("IgnoreThisForReset")) return;

        CurrentHP = MaxHP;
        RestoreInitialPos = transform.position;
        myRenderer = GetComponent<SpriteRenderer>();

        if (NavigationManager.Dok.IsBossLevel)
        {
            if(gameObject.GetComponent<TikiBossScript>())
            gameObject.GetComponent<TikiBossScript>().HP.value = CurrentHP;
            else if(gameObject.GetComponent<PinkyBossScript>())
                gameObject.GetComponent<PinkyBossScript>().HP.value = CurrentHP;
        }

    }

    void Update()
    {
        if (DamageTinter)
        {
            float lerp = Mathf.PingPong(Time.time, lerpDuration) / lerpDuration;
            myRenderer.color = Color.Lerp(minRange, maxRange, lerp);

            CurLerpTimer += Time.deltaTime;
            if (CurLerpTimer > LerpTimer)
            {
                myRenderer.color = maxRange;
                CurLerpTimer = 0;
                DamageTinter = false;
            }
            
        }
    }

    public void ResetThisEnemy()
    {
        if (gameObject.name.Contains("IgnoreThisForReset")) return;

        gameObject.SetActive(true);
        CurrentHP = MaxHP;
        transform.position = RestoreInitialPos;
        HasDied = false;
        DamageTinter = false;
        GetComponent<SpriteRenderer>().color = Color.white;

        //Forest
        if (GetComponent<CatAI>())
        {
            GetComponent<CatAI>().CatReset();
        }
        if (GetComponent<MonkeyAI>())
        {
            GetComponent<MonkeyAI>().MonkeyReset();
        }
        if (GetComponent<BeeAI>())
        {
            GetComponent<BeeAI>().BeeReset();
        }
        if (GetComponent<TikiWarriorAI>())
        {
            GetComponent<TikiWarriorAI>().TikiWarriorReset();
        }

        if (GetComponent<TikiBossScript>())
        {
            GetComponent<TikiBossScript>().ResetThisBoss();
            gameObject.GetComponent<TikiBossScript>().HP.value = CurrentHP;
        }

        //aquatic
        if (GetComponent<AnglerFish>())
        {
            GetComponent<AnglerFish>().ResetThisEnemy();
        }
        if (GetComponent<OctoAI>())
        {
            GetComponent<OctoAI>().ResetThisEnemy();
        }
        if (GetComponent<SharkAI>())
        {
            GetComponent<SharkAI>().ResetThisEnemy();
        }
        if (GetComponent<CrabAI>())
        {
            GetComponent<CrabAI>().ResetThisEnemy();
        }

        if (GetComponent<PinkyBossScript>())
        {
            GetComponent<PinkyBossScript>().InitializeBoss();
            gameObject.GetComponent<PinkyBossScript>().HP.value = CurrentHP;
        }


        if (GetComponent<PenguinAI>())
        {
            GetComponent<PenguinAI>().ResetThisEnemy();
        }
        if (GetComponent<EskimoAI>())
        {
            GetComponent<EskimoAI>().ResetThisEnemy();
        }
        if (GetComponent<ElfAI>())
        {
            GetComponent<ElfAI>().ResetThisEnemy();
        }
        if (GetComponent<WalrusAI>())
        {
            GetComponent<WalrusAI>().ResetThisEnemy();
        }

        if (GetComponent<SantaAI>())
        {
            GetComponent<SantaAI>().InitializeBoss();
            gameObject.GetComponent<SantaAI>().HP.value = CurrentHP;
        }


        if (GetComponent<SalamanderAI>())
        {
            GetComponent<SalamanderAI>().ResetThisOne();
        }
        if (GetComponent<TriceratopsAI>())
        {
            GetComponent<TriceratopsAI>().ResetThisEnemy();
        }

        if(transform.parent && gameObject.name.Contains("Imp"))
        {
            
            GetComponent<BeeAI>().BeeReset();
            transform.parent.gameObject.GetComponent<ImpRotatorScript>().ResetThisEnemy();
        }


        if (GetComponent<DragonAI_2>() || GetComponent<DragonAI_3>() || GetComponent<DragonAIPart4>())
        {
            transform.parent.GetComponent<DragonAI>().ResetThisHugeThing();
            
        }

        if (GetComponent<CrowAI>())
        {
            GetComponent<CrowAI>().ResetMe();
        }
        if (GetComponent<AeroplaneAI>())
        {
            GetComponent<AeroplaneAI>().RestartMe();
        }
        if(GetComponent<TornadoAI>())
        {
            GetComponent<TornadoAI>().Restart();
        }

        if (GetComponent<AzuriusBossScript>())
        {
            GetComponent<AzuriusBossScript>().ResetThisBoss();
        }


        if(GetComponent<ZombieAI>())
        {
            GetComponent<ZombieAI>().ResetThisEnemy();
        }

        if(GetComponent<FaceAI>())
        {
            GetComponent<FaceAI>().Reset();
        }


    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if (other.gameObject.CompareTag("Projectile"))
        {

            if (GetComponent<FaceAI>())
            {
                if (!GetComponent<FaceAI>().AbleToGetHit) return;
            }

            GameObject Poof = ProjPool.GlobalPool.GetFirstActiveObject(ProjPool.GlobalPool.MyProjPoofParticles);
            Poof.SetActive(true);

            
            Poof.transform.position = other.gameObject.transform.position;
            
            
            

            Poof.gameObject.GetComponent<ParticleShutter>().ParticlePlay();
            if (NavigationManager.Dok.Level>= 13 && NavigationManager.Dok.Level<=16)
            {
                Director.GlobalDirector.PlaySFX_Fire(Director.SoundEffectsFire.ExplosionPoof);
            }
            else
            {
                if(!Application.loadedLevelName.Contains("Tutorial"))
                Director.GlobalDirector.PlaySFX(Director.SoundEffects.ProjPoof);
            }
            
            other.gameObject.SetActive(false);
            CurrentHP--;
            if (!Application.loadedLevelName.Contains("Tutorial"))
            Director.GlobalDirector.AddScore(20);

            DamageTinter = true;

            if (GetComponent<TriceratopsAI>())
            {
                if (transform.GetChild(0))
                    transform.GetChild(0).gameObject.GetComponent<ResetLevelEnemy>().DamageTinter = true;
            }

            if (GetComponent<ZombieAI>())
            {
                CurrentHP++;
                Director.GlobalDirector.AddScore(-15);
                GetComponent<ZombieAI>().Damage();
                return;
            }

            if (NavigationManager.Dok.IsBossLevel)
            {
                if (GetComponent<TikiBossScript>())
                    gameObject.GetComponent<TikiBossScript>().HP.value = CurrentHP;
                if (GetComponent<PinkyBossScript>())
                {
                    gameObject.GetComponent<PinkyBossScript>().HP.value = CurrentHP;
                    gameObject.GetComponent<PinkyBossScript>().TeenFormGO.GetComponent<ResetLevelEnemy>().DamageTinter = true;
                    gameObject.GetComponent<PinkyBossScript>().TeenAttack.GetComponent<ResetLevelEnemy>().DamageTinter = true;
                    gameObject.GetComponent<PinkyBossScript>().AdultAttack.GetComponent<ResetLevelEnemy>().DamageTinter = true;
                    gameObject.GetComponent<PinkyBossScript>().AdultFormGO.GetComponent<ResetLevelEnemy>().DamageTinter = true;

                }
                if (GetComponent<SantaAI>())
                {
                    gameObject.GetComponent<SantaAI>().HP.value = CurrentHP;
                    gameObject.GetComponent<SantaAI>().Damage();
                }

                if(transform.parent)
                if (transform.parent.GetComponent<DragonAI>())
                {
                    transform.parent.gameObject.GetComponent<DragonAI>().DamageMe();
                }

                if (GetComponent<AzuriusBossScript>())
                {
                    GetComponent<AzuriusBossScript>().Damage();
                }

                if (GetComponent<FaceAI>())
                {
                    GetComponent<FaceAI>().Damage();
                }
            }

            if (CurrentHP <= 0)
            {
                if (GetComponent<TikiBossScript>())
                {
                    gameObject.GetComponent<TikiBossScript>().HP.value = 0;
                    gameObject.GetComponent<TikiBossScript>().MyState = TikiBossScript.TikiBossStates.Inactive;
                    StartCoroutine(BossVictory());

                }
                else if (GetComponent<PinkyBossScript>())
                {
                    gameObject.GetComponent<PinkyBossScript>().HP.value = 0;
                    gameObject.GetComponent<PinkyBossScript>().PinkyForm = PinkyBossScript.CurrentState.PinkyLost;
                    gameObject.transform.position = new Vector3(199999, 199999, 19999);
                    gameObject.GetComponent<PinkyBossScript>().enabled = false;
                    StartCoroutine(BossVictory());

                }
                else if (GetComponent<SantaAI>())
                {
                    gameObject.GetComponent<SantaAI>().HP.value = 0;
                    gameObject.transform.position = new Vector3(199999, 199999, 19999);
                    gameObject.GetComponent<SantaAI>().enabled = false;
                    StartCoroutine(BossVictory());
                }
                else if (GetComponent<DragonAI_3>())
                {
                    gameObject.transform.parent.gameObject.GetComponent<DragonAI>().BossSlider.value = 0;
                    gameObject.transform.parent.gameObject.transform.position = new Vector3(199999, 199999, 19999);
                    gameObject.transform.parent.gameObject.GetComponent<DragonAI>().enabled = false;
                    StartCoroutine(BossVictory());
                    
                }
                else if (GetComponent<AzuriusBossScript>())
                {
                    gameObject.GetComponent<AzuriusBossScript>().HP.value = 0;
                    gameObject.transform.position = new Vector3(199999, 199999, 19999);
                    gameObject.GetComponent<AzuriusBossScript>().enabled = false;
                    StartCoroutine(BossVictory());
                }
                else if (GetComponent<FaceAI>())
                {
                    gameObject.GetComponent<FaceAI>().HP.value = 0;
                    gameObject.transform.position = new Vector3(199999, 199999, 19999);
                    gameObject.GetComponent<FaceAI>().enabled = false;
                    StartCoroutine(BossVictory());
                }
                else
                {
                    GameObject Poof2 = ProjPool.GlobalPool.GetFirstActiveObject(ProjPool.GlobalPool.MyMonsterPoofParticles);
                    Poof2.SetActive(true);


                    
                    if (other.gameObject.name.Contains("Skeleton") || other.gameObject.name.Contains("Zombie") || other.gameObject.name.Contains("Necromancer"))
                    {
                        Poof2.transform.position = new Vector3(other.gameObject.transform.position.x, other.gameObject.transform.position.y, Director.Player.transform.position.z);
                    }
                    else
                    {
                        Poof2.transform.position = other.gameObject.transform.position;
                    }
                    if (!Application.loadedLevelName.Contains("Tutorial"))
                    {
                        Director.GlobalDirector.PlaySFX(Director.SoundEffects.ProjPoof);
                        Director.GlobalDirector.AddScore(100);
                    }
                    gameObject.SetActive(false);
                }
            }
        }
    }

    IEnumerator BossVictory()
    {
        Director.GlobalDirector.GameWon = true;


        yield return new WaitForSeconds(2);

        
        Director.PlayerCommands.ourRigid.velocity = Vector2.zero;
        Director.PlayerCommands.ourAnimator.SetInteger("Speed", 0);

        Director.GlobalDirector.StageClear.SetActive(true);
        Director.GameMusic.Stop();

        yield return new WaitForSeconds(2);
        Director.GlobalDirector.gameObject.GetComponent<AudioSource>().Play();
        Director.GlobalDirector.StartFading();
        yield return new WaitForSeconds(6.9f);
        Director.GlobalDirector.ShouldLoopMusicWin();
    }

}
