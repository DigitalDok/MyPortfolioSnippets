using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class ProjPool : MonoBehaviour {

    // This is an Object Pool System for C# in Unity. We determine a number of objects to spawn and then never cause any overhead in our memory or GC.

    public List<GameObject> MyGenericProjectile = new List<GameObject>();
    public GameObject OurGenericProjectile;
    public List<GameObject> MySFX_Players = new List<GameObject>();
    public GameObject SFX_Player;

    public List<GameObject> MyProjPoofParticles = new List<GameObject>();
    public GameObject OurPoofParticleFromProj;

    public List<GameObject> MyMonsterPoofParticles = new List<GameObject>();
    public GameObject OurPoofParticleFromMonster;

    public List<GameObject> MyCoinPoofParticles = new List<GameObject>();
    public GameObject OurCoinPoofParticle;


    public int GenericProjectileCount;
    public int SFX_PlayersCount;
    public int PoofParticlesCount;
    public int PoofParticlesMonsterCount;
    private int CoinPoofParticlesCount = 15;


    public List<List<GameObject>> AllSpawnables = new List<List<GameObject>>();

    public static ProjPool GlobalPool;
    

	// Initialization
	
	void Start () 
    {
        GlobalPool = this;
        
        for (int i = 0; i < GenericProjectileCount; i++)
        {
            var GO = (GameObject)Instantiate(OurGenericProjectile, Vector3.zero, Quaternion.identity);
            MyGenericProjectile.Add(GO);
            GO.transform.parent = gameObject.transform;
            GO.SetActive(false);
        }
        AllSpawnables.Add(MyGenericProjectile);

        for (int i = 0; i < SFX_PlayersCount; i++)
        {
            var GO2 = (GameObject)Instantiate(SFX_Player, Vector3.zero, Quaternion.identity);
            MySFX_Players.Add(GO2);
            GO2.transform.parent = gameObject.transform;
            GO2.SetActive(false);
        }
        AllSpawnables.Add(MySFX_Players);

        for (int i = 0; i < PoofParticlesCount; i++)
        {
            var GO2 = (GameObject)Instantiate(OurPoofParticleFromProj, Vector3.zero, Quaternion.identity);
            MyProjPoofParticles.Add(GO2);
            GO2.transform.parent = gameObject.transform;
            GO2.SetActive(false);
        }
        AllSpawnables.Add(MyProjPoofParticles);

        for (int i = 0; i < PoofParticlesMonsterCount; i++)
        {
            var GO2 = (GameObject)Instantiate(OurPoofParticleFromMonster, Vector3.zero, Quaternion.identity);
            MyMonsterPoofParticles.Add(GO2);
            GO2.transform.parent = gameObject.transform;
            GO2.SetActive(false);
        }
        AllSpawnables.Add(MyMonsterPoofParticles);


        for (int i = 0; i < CoinPoofParticlesCount; i++)
        {
            var GO2 = (GameObject)Instantiate(OurCoinPoofParticle, Vector3.zero, Quaternion.identity);
            MyCoinPoofParticles.Add(GO2);
            GO2.transform.parent = gameObject.transform;
            GO2.SetActive(false);
        }
        AllSpawnables.Add(MyMonsterPoofParticles);

	}

    
	// Object Pool Helpers
        foreach (var item in MyArrows)
        {
            if (!item.gameObject.activeInHierarchy)
                return item.gameObject;
        }
        return null;
    }

    internal GameObject GetFirstActiveObject(List<GameObject> TheList)
    {
        foreach (var item in TheList)
        {
            if (!item.gameObject.activeInHierarchy)
                return item.gameObject;
        }
        return null;
    }

    internal void DeactivateAllProjectiles()
    {
        foreach (var item in AllSpawnables)
        {
            foreach (var item2 in item)
            {
                item2.SetActive(false);
            }
        }
    }

	#region Sound Effect Playing
	
    internal void PlaySFXWithThisClip(AudioClip item)
    {
        GameObject ClipPlayer = GetFirstActiveObject(MySFX_Players);
        ClipPlayer.SetActive(true);
        ClipPlayer.GetComponent<ClipPlayer>().ShouldTurnOff = false;
        ClipPlayer.GetComponent<AudioSource>().clip = item;
        ClipPlayer.GetComponent<AudioSource>().Play();
        ClipPlayer.GetComponent<AudioSource>().spatialBlend = 0;
        ClipPlayer.GetComponent<ClipPlayer>().ShouldTurnOff = true;
    }

    internal void PlaySFXWithThisClip2(AudioClip item, float ChangeCD)
    {
        GameObject ClipPlayer = GetFirstActiveObject(MySFX_Players);
        ClipPlayer.SetActive(true);
        ClipPlayer.GetComponent<ClipPlayer>().ShouldTurnOff = false;
        ClipPlayer.GetComponent<AudioSource>().clip = item;
        ClipPlayer.GetComponent<AudioSource>().Play();
        ClipPlayer.GetComponent<ClipPlayer>().ShouldTurnOff = true;

        StartCoroutine(DisableInCDSec(ChangeCD,ClipPlayer));
        
    }

    IEnumerator DisableInCDSec(float CD,GameObject CP)
    {
        yield return new WaitForSeconds(CD);
        CP.GetComponent<AudioSource>().Stop();
        CP.SetActive(false);

    }

    internal void PlaySFXWithThisClip3D(AudioClip audioClip, Vector3 Pos)
    {
        GameObject ClipPlayer = GetFirstActiveObject(MySFX_Players);
        ClipPlayer.SetActive(true);
        ClipPlayer.GetComponent<ClipPlayer>().ShouldTurnOff = false;
        ClipPlayer.GetComponent<AudioSource>().clip = audioClip;
        ClipPlayer.GetComponent<AudioSource>().Play();
        ClipPlayer.GetComponent<AudioSource>().spatialBlend = 1;
        ClipPlayer.transform.position = Pos;
        ClipPlayer.GetComponent<AudioSource>().rolloffMode = AudioRolloffMode.Linear;
        ClipPlayer.GetComponent<AudioSource>().maxDistance = 5;
        ClipPlayer.GetComponent<ClipPlayer>().ShouldTurnOff = true;
    }

    internal void PlaySFXWithThisClip3D(AudioClip audioClip, Vector3 Pos, float p)
    {
        GameObject ClipPlayer = GetFirstActiveObject(MySFX_Players);
        ClipPlayer.SetActive(true);
        ClipPlayer.GetComponent<ClipPlayer>().ShouldTurnOff = false;
        ClipPlayer.GetComponent<AudioSource>().clip = audioClip;
        ClipPlayer.GetComponent<AudioSource>().Play();
        ClipPlayer.GetComponent<AudioSource>().spatialBlend = 1;
        ClipPlayer.transform.position = Pos;
        ClipPlayer.GetComponent<AudioSource>().rolloffMode = AudioRolloffMode.Linear;
        ClipPlayer.GetComponent<AudioSource>().maxDistance = p;
        ClipPlayer.GetComponent<ClipPlayer>().ShouldTurnOff = true;
    }
	
	#endregion
}
