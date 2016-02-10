using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Patroller : MonoBehaviour
{
    #region Fields

    #region Initialization
    // Useful References
    private NavMeshAgent nav;
    private AI_Senses mySenses;
    private GameObject playerGameObject;

    // If The AI is sleeping, the navigation will not function
    public bool sleeping = true;

    // The Time Limit at which the AI will start functioning
    public float awakeTime = 1f;
    private float awakeTimer;

    // We hold the object's speed and acceleration, from the NavMesh Agent Component here.
    private float initialSpeed;
    private float initialAcceleration;
    private float initialAngularSpeed;

    #endregion

    #region Animation and Testing
    // A list with 2 animations, (0) Idle, (1) Moving.
    public AnimationClip[] myAnims;

    // We would like to set whether the AI is a model with animations or just a cube, for whiteboxing reasons.
    public bool isModel;

    public Animator animator;
    #endregion

    #region Waypoint Fields
    // A list containing all waypoints. Used for storing the actual waypoints.
    public List<GameObject> waypoints;
    // A list with all the available waypoints. We use this list for parsing.
    private List<GameObject> possibleWaypoints = new List<GameObject>();

    // Hold reference to the Current Waypoint for each of our AI bots.
    public GameObject currentWayPoint;

    #endregion

    #region Movement & Waiting
    public enum AIState
    {
        Sleeping,
        Patrolling,
        Idling,
        Investigating,
        Chasing,
        AlertedInvestigation,
        Surprise,
        Resting,
        DoorOpening,
        PermanentChase
    }
    public AIState aiState = AIState.Sleeping;

    // 2 Integers to be added to the inspector so we can set the 2 Limits of our randomized Idle Time.
    public float minRandIdleTime = 1;
    public float maxRandIdleTime = 2;

    // Floats determining for how long our character is waiting at Idle state before moving on.
    public float idleTime;
    public float idleTimer;

    public float investigateTime = 10f;
    public float investigateTimer;

    public float chaseSpeed = 4;

    public float alertedInvestigationTime = 5f;
    public float alertedInvestigationTimer;

    public float restTime = 5f;
    public float restTimer;

    #endregion

    #region Surprise Phase
    public enum SurprisePhase
    {
        None,
        MovingTowardsDoor,
        LookingAtPlayer,
        DeterminingNextMove,
        Randomizing,
        DeterminingFinalMove,
        SurprisePlayer
    }
    public SurprisePhase surprisePhase = SurprisePhase.None;

    // Store the monster AI state before entering Surprise Phase
    internal AIState aiStateBeforeSurprisePhase = AIState.Investigating;
    public Transform surpriseNode;
    public Door leftWardrobeDoorScript;
    public Door rightWardrobeDoorScript;
    // public bool isInsideMonsteroidTrigger = false; // Note: This is not actually used in this version

    public int percentageOfOpeningWardrobe = 50;
    private int rand = 0;
    private float timeSpentLooking = 0f;
    public float waitingTimeBeforeOpeningWardrobe = 3f;
    #endregion

    #region Door Opening Phase
    public enum DoorOpeningPhase
    {
        None,
        MovingTowardsDoorNode,
        OpenDoor,
        WaitingForDoorToOpen
    }
    public DoorOpeningPhase doorOpeningPhase = DoorOpeningPhase.None;

    // Store the monster AI state before entering Surprise Phase
    internal AIState aiStateBeforeDoorOpeningPhase = AIState.Patrolling;
    public Transform doorNode;
    private float openingDoorTime = 4f;  // This is a temp value. It's is actually get overridden by doorLimitTime value from the DoorMonsterScript  
    public float openingDoorTimer = 0f;
    // The door script, that the monster interacts 
    internal List<Door> doorScriptList;

    #endregion

    #endregion

    #region Properties
    public float OpeningDoorTime
    {
        set { openingDoorTime = value; }
    }

    public Vector3 LastSeenPosition
    {
        get { return mySenses.lastSeenPosition; }
    }

    public Vector3 LastHeardPosition
    {
        get { return mySenses.lastHeardPosition; }
    }
    #endregion

    #region Initialize Methods
    void Awake()
    {
        /* As a matter of special preference, any node that has the same name as the host gameobject along with the "_Node" suffix is added to our Waypoint List.
         * For example, if our name is "MON" and we have 40 Nodes in-game, 10 of which have the tag "MON_Node", these 10 nodes will be added to our Waypoint List.
         */
        waypoints = new List<GameObject>(GameObject.FindGameObjectsWithTag(transform.name + "_Node"));

        // Setting up all the references
        initialSpeed = GetComponent<NavMeshAgent>().speed;
        initialAcceleration = GetComponent<NavMeshAgent>().acceleration;
        initialAngularSpeed = GetComponent<NavMeshAgent>().angularSpeed;

        nav = GetComponent<NavMeshAgent>();
        mySenses = GetComponent<AI_Senses>();
        playerGameObject = GameObject.FindGameObjectWithTag("Player");
    }

    void Start()
    {
        // Determine monster initial AI State
        if (sleeping)
        {
            aiState = AIState.Sleeping;
            TransitionWalkToIdle();
        }
        else
        {
            aiState = AIState.Patrolling;
            TransitionIdleToWalk();
        }

        // Set the first monster destination
        SetDestination();

        // Initialize the idle time, in order to be used when the monster reaches its first waypoint destination
        idleTime = Random.Range(minRandIdleTime, maxRandIdleTime);
    }

    #endregion

    #region Update Methods
    void Update()
    {
        // Update monster AI
        UpdateAIState();
    }

    void UpdateAIState()
    {
        switch (aiState)
        {
            case AIState.Sleeping:
                // If AI is not enabled yet, progress the timer until it awakes
                AwakenMonsterAfterAwakeTime();
                break;

            case AIState.Patrolling:
                Patrol();

                // Check for player presence
                CheckMonsterSenses();
                break;

            case AIState.Idling:
                // While the AI is idle define a new destination
                DefineNewDestinationWhileIdling();

                // Check for player presence
                CheckMonsterSenses();
                break;

            case AIState.Investigating:
                Investigate();

                // Check for player presence
                CheckMonsterSenses();
                break;

            case AIState.Chasing:
                Chase();

                // Check for player presence
                //CheckMonsterSenses();
                CheckMonsterSight();
                break;

            case AIState.PermanentChase:
                PermanentChase();
                break;

            case AIState.AlertedInvestigation:
                AlertedInvestigation();

                // Check for player presence
                CheckMonsterSenses();
                break;

            case AIState.Resting:
                Rest();

                // Check for player presence
                CheckMonsterSenses();
                break;

            case AIState.Surprise:
                UpdateSurprisePhase();
                break;

            case AIState.DoorOpening:
                UpdateDoorOpeningPhase();
                break;

            default:
                break;
        }
    }
    #endregion

    #region Core Functionality

    #region Sleeping State
    /// <summary>
    /// Wakes up monster after a certain amount of time
    /// </summary>
    private void AwakenMonsterAfterAwakeTime()
    {
        awakeTimer += Time.deltaTime;

        if (awakeTimer > awakeTime)
        {
            sleeping = false;

            // Move to Patrolling state
            aiState = AIState.Patrolling;

            TransitionIdleToWalk();
        }
    }
    #endregion

    #region Patrolling State
    private void Patrol()
    {
        // Always set monster destination to the current waypoint
        nav.destination = currentWayPoint.transform.position;
    }

    /// <summary>
    /// Checks if the monster is standing on the current waypoint. 
    /// This method is called, when the monster enters a node
    /// </summary>
    /// <param name="reachedWaypoint">This is the reached waypoint gameobject</param>
    public void CheckIfWeFoundTheDestinationWaypoint(GameObject reachedWaypoint)
    {
        if (aiState == AIState.Patrolling && reachedWaypoint == currentWayPoint)
        {
            if (isModel)
            {
                GetComponent<Animation>().clip = myAnims[0];
                GetComponent<Animation>().Play();
            }

            // Move to idle state
            aiState = AIState.Idling;

            TransitionWalkToIdle();

            FreezeNavMeshAgent();
        }
    }
    #endregion

    #region Idling State
    private void DefineNewDestinationWhileIdling()
    {
        idleTimer += Time.deltaTime;

        if (idleTimer > idleTime)
        {
            // Reset the countdown
            idleTimer = 0f;

            // Randomize the idle time (Time to wait after destination is reached in order to determine next destination and start moving)
            idleTime = Random.Range(minRandIdleTime, maxRandIdleTime);

            // Determine a new destination
            DefineNewWaypoint();

            // If our AI bot is a 3D Model, play the animations that the bot should play.
            AnimationsForModel();

            // Grant the AI the same speed and acceleration it had, once it starts moving again 
            SetNormalSpeedInNavMeshAgent();

            // Move to Patrolling state
            aiState = AIState.Patrolling;

            TransitionIdleToWalk();

            // Occupy The Target Node, so other AI bots do NOT go for it.
            currentWayPoint.GetComponent<node>().isOccupied = true;

            if (!currentWayPoint) Debug.Log("Target is null!");
        }
    }
    #endregion

    #region Investigating State
    /// <summary>
    /// Puts the monster in Investigation State
    /// </summary>
    private void Investigate()
    {
        // Reset other timers
        idleTimer = 0f;
        alertedInvestigationTimer = 0f;
        restTimer = 0f;

        // Head to the last heard position
        nav.destination = mySenses.lastHeardPosition;

        SetNormalSpeedInNavMeshAgent();

        // If near the last heared position...
        if (nav.remainingDistance < nav.stoppingDistance)
        {
            // ... increment the timer.
            investigateTimer += Time.deltaTime;

            // If the timer exceeds the investigate time...
            if (investigateTimer > investigateTime)
            {
                // ... reset last global sighting, the last personal sighting and the timer.
                //lastPlayerSighting.position = lastPlayerSighting.resetPosition;     // TODO: I don't know if we need this line
                mySenses.lastHeardPosition = mySenses.resetPosition;
                mySenses.lastHeardPositionRoom = mySenses.resetRoomString;

                investigateTimer = 0f;

                SetNormalSpeedInNavMeshAgent();

                // Return to Patrolling state
                aiState = AIState.Patrolling;
            }
        }
    }
    #endregion

    #region Chasing State
    void Chase()
    {
        // Reset other timers
        idleTimer = 0f;
        investigateTimer = 0f;
        alertedInvestigationTimer = 0f;
        restTimer = 0f;

        transform.LookAt(new Vector3(nav.destination.x, transform.position.y, nav.destination.z));

        // Create a vector from the enemy to the last sighting of the player.
        Vector3 sightingDeltaPos = mySenses.lastSeenPosition - transform.position;

        // If the the last personal sighting of the player is not close...
        if (sightingDeltaPos.sqrMagnitude > 4f)
        {
            // ... set the destination for the NavMeshAgent to the last personal sighting of the player.
            nav.destination = mySenses.lastSeenPosition;
        }

        // Set the appropriate speed for the NavMeshAgent.
        SetChaseSpeedInNavMeshAgent();

        // If near the last personal sighting...
        if (nav.remainingDistance < nav.stoppingDistance)
        {
            // ... reset last global sighting, the last personal sighting and the timer.
            //lastPlayerSighting.position = lastPlayerSighting.resetPosition;
            mySenses.lastSeenPosition = mySenses.resetPosition;
            mySenses.lastSeenPositionRoom = mySenses.resetRoomString;

            // Move to Alerted Investigation State
            aiState = AIState.AlertedInvestigation;

            TransitionChaseToWalk();

            // Reset last hearing position (because we proceed to Alerted Investigation State)
            mySenses.lastHeardPosition = mySenses.resetPosition;
            mySenses.lastHeardPositionRoom = mySenses.resetRoomString;

            // TODO: I don't know if we need this block of code
            //if (mySenses.lastHeardPosition != mySenses.resetPosition)
            //{
            //    return;
            //}
            //else
            //{
            //    GetComponent<NavMeshAgent>().speed = initialSpeed;
            //    GetComponent<NavMeshAgent>().acceleration = initialAcceleration;
            //    // isIdle = false;
            //}  
        }
    }
    #endregion

    #region Permanent Chase State
    void PermanentChase()
    {
        // Reset other timers
        idleTimer = 0f;
        investigateTimer = 0f;
        alertedInvestigationTimer = 0f;
        restTimer = 0f;

        // Set the destination for the NavMeshAgent to the player position.
        nav.destination = playerGameObject.transform.position;

        // Look at the player
        transform.LookAt(new Vector3(nav.destination.x, transform.position.y, nav.destination.z));

        // Set the appropriate speed for the NavMeshAgent.
        SetChaseSpeedInNavMeshAgent();
    }

    /// <summary>
    /// Moves the monster to permanent chase mode
    /// </summary>
    public void EnablePermanentChase()
    {
        // Move to permanent chase
        aiState = AIState.PermanentChase;

        TransitionWalkToChase(); // Note: (Dimitris) The animation works correctly when the monster is 
        // walking (Patrolling or Investigating). There is not a transition from IndleToChase
    }
    #endregion

    #region Alerted Investigation State
    private void AlertedInvestigation()
    {
        // Reset other timers
        idleTimer = 0f;
        investigateTimer = 0f;
        restTimer = 0f;

        SetNormalSpeedInNavMeshAgent();

        // Head to the player position
        nav.destination = mySenses.player.transform.position;

        // ... increment the timer.
        alertedInvestigationTimer += Time.deltaTime;

        // If the timer exceeds the alerted investigation time...
        if (alertedInvestigationTimer > alertedInvestigationTime)
        {
            // Set monster destination to the current position
            nav.destination = transform.position;

            // Move to Resting State
            aiState = AIState.Resting;

            TransitionWalkToIdle();
        }
    }
    #endregion

    #region Resting State
    /// <summary>
    /// Makes the monster rest for some time (after an Alerted Investigation)    
    /// </summary>
    private void Rest()
    {
        // Reset other timers
        idleTimer = 0f;
        investigateTimer = 0f;
        alertedInvestigationTimer = 0f;

        if (isModel)
        {
            GetComponent<Animation>().clip = myAnims[0];
            GetComponent<Animation>().Play();
        }

        FreezeNavMeshAgent();

        // ... increment the timer.
        restTimer += Time.deltaTime;

        // If the timer exceeds the resting time...
        if (restTimer > restTime)
        {
            SetDestination();

            // Return to Patrolling state
            aiState = AIState.Patrolling;

            TransitionIdleToWalk();
        }
    }
    #endregion

    #region Surprise Phase
    private void UpdateSurprisePhase()
    {
        // Manage Surprise Phase States
        switch (surprisePhase)
        {
            case SurprisePhase.MovingTowardsDoor:
                nav.destination = surpriseNode.position;

                if (nav.remainingDistance < nav.stoppingDistance)
                {
                    surprisePhase = SurprisePhase.LookingAtPlayer;
                }
                break;

            case SurprisePhase.LookingAtPlayer:
                timeSpentLooking += Time.deltaTime;
                if (timeSpentLooking > waitingTimeBeforeOpeningWardrobe)
                {
                    timeSpentLooking = 0;
                    // Look at player
                    transform.LookAt(mySenses.player.transform.position); // Note: We need to smoothly look at player
                    surprisePhase = SurprisePhase.DeterminingNextMove;
                }
                break;

            case SurprisePhase.DeterminingNextMove:
                // Check if monster was chasing the player
                if (aiStateBeforeSurprisePhase == AIState.Chasing)
                {
                    // Open Doors
                    OpenWardrobeDoors(true);

                    //Game over
                    Debug.Log("GAME OVER FROM CHASE");


                    surprisePhase = SurprisePhase.None;
                }
                // Check if monster was Investigating or was coming from Alerted Investigation
                else if (aiStateBeforeSurprisePhase == AIState.Investigating ||
                         aiStateBeforeSurprisePhase == AIState.AlertedInvestigation)
                {
                    surprisePhase = SurprisePhase.Randomizing;
                }
                break;

            case SurprisePhase.Randomizing:
                // Randomize
                rand = Random.Range(1, 101);

                // Proceed to final move 
                surprisePhase = SurprisePhase.DeterminingFinalMove;
                break;

            case SurprisePhase.DeterminingFinalMove:

                if (rand <= percentageOfOpeningWardrobe)
                {
                    // Open Doors
                    OpenWardrobeDoors(false);

                    // GameOver
                    Debug.Log("GAME OVER FROM RANDOM");

                    surprisePhase = SurprisePhase.None;
                }
                else
                {
                    surprisePhase = SurprisePhase.None;

                    mySenses.lastHeardPosition = mySenses.resetPosition;
                    mySenses.lastSeenPosition = mySenses.resetPosition;

                    mySenses.lastSeenPositionRoom = mySenses.resetRoomString;
                    mySenses.lastHeardPositionRoom = mySenses.resetRoomString;

                    idleTimer = 0f;
                    investigateTimer = 0f;
                    alertedInvestigationTimer = 0f;
                    restTimer = 0f;

                    SetNormalSpeedInNavMeshAgent();

                    // Return to patrolling state
                    aiState = AIState.Patrolling;
                }
                break;

            case SurprisePhase.SurprisePlayer:
                nav.destination = surpriseNode.position;

                if (nav.remainingDistance < nav.stoppingDistance)
                {
                    Debug.Log("Opened the Doors and Surprise the player");
                    OpenWardrobeDoors(false);

                    //Game over
                    Debug.Log("GAME OVER BECAUSE MONSTER SAW YOU IN THE CLOSET");

                    surprisePhase = SurprisePhase.None;
                }
                break;

            default:
                break;
        }

    }

    public void OpenWardrobeDoors(bool monsterIsChasingThePlayer)
    {
        // Monster opens both wardrobe doors 
        leftWardrobeDoorScript.OpenWardrobeDoor();
        rightWardrobeDoorScript.OpenWardrobeDoor();
    }
    #endregion

    #region AI Senses Methods
    /// <summary>
    /// Check for player presence using the AI Senses
    /// </summary>
    private bool CheckMonsterSenses()
    {
        // Check if monster has seen the player
        if (mySenses.lastSeenPosition != mySenses.resetPosition)
        {
            if (aiState == AIState.Idling)
            {
                TransitionIdleToChase();
            }
            else
            {
                TransitionWalkToChase();
            }

            aiState = AIState.Chasing;

            return true;
        }
        // Check if monster has heard the player
        else if (mySenses.lastHeardPosition != mySenses.resetPosition)
        {
            TransitionIdleToWalk();
            aiState = AIState.Investigating;

            return true;
        }
        else
        {
            return false;
        }
    }

    /// <summary>
    /// Check for player presence using only the Monster Sight
    /// </summary>
    private bool CheckMonsterSight()
    {
        // Check if monster has seen the player
        if (mySenses.lastSeenPosition != mySenses.resetPosition)
        {
            if (aiState == AIState.Idling)
            {
                TransitionIdleToChase();
            }
            else
            {
                TransitionWalkToChase();
            }

            aiState = AIState.Chasing;

            return true;
        }
        else
        {
            return false;
        }
    }
    #endregion

    #region Door Opening (or Breaking) Phase
    private void UpdateDoorOpeningPhase()
    {
        switch (doorOpeningPhase)
        {
            case DoorOpeningPhase.None:
                break;

            case DoorOpeningPhase.MovingTowardsDoorNode:
                nav.destination = doorNode.position;

                if (nav.remainingDistance < nav.stoppingDistance)
                {
                    doorOpeningPhase = DoorOpeningPhase.OpenDoor;

                    FreezeNavMeshAgent();

                    TransitionWalkToIdle();
                }
                break;

            case DoorOpeningPhase.OpenDoor:
                if (aiStateBeforeDoorOpeningPhase == AIState.Chasing ||
                    aiStateBeforeDoorOpeningPhase == AIState.PermanentChase)
                {
                    for (int i = 0; i < doorScriptList.Count; i++)
                    {
                        doorScriptList[i].BreakTheDoor(transform.forward);
                    }

                    // Reset door opening chase (because we are breaking the door(s) we don't 
                    // need to proceed to WaitingForDoorToOpen)
                    doorOpeningPhase = DoorOpeningPhase.None;

                    // Reset timer 
                    openingDoorTimer = 0f;

                    // Return to monsters' previous state
                    aiState = aiStateBeforeDoorOpeningPhase;

                    SetChaseSpeedInNavMeshAgent();

                    TransitionIdleToChase();
                }
                else
                {
                    // Open the door(s)
                    for (int i = 0; i < doorScriptList.Count; i++)
                    {
                        doorScriptList[i].Open();
                    }

                    // Move to the next phase
                    doorOpeningPhase = DoorOpeningPhase.WaitingForDoorToOpen;
                }
                break;

            case DoorOpeningPhase.WaitingForDoorToOpen:
                openingDoorTimer += Time.deltaTime;

                if (CheckMonsterSenses())
                {
                    for (int i = 0; i < doorScriptList.Count; i++)
                    {
                        doorScriptList[i].isUsedByMonster = false;
                    }

                    // Reset timer 
                    openingDoorTimer = 0f;

                    doorOpeningPhase = DoorOpeningPhase.None;
                }

                if (openingDoorTimer > openingDoorTime || doorScriptList[0].CurrentDoorState == Door.DoorState.FullyOpened) // Note: We just need to check the one door
                {
                    doorOpeningPhase = DoorOpeningPhase.None;

                    // The door is not used by the monster anymore
                    for (int i = 0; i < doorScriptList.Count; i++)
                    {
                        doorScriptList[i].isUsedByMonster = false;
                    }

                    // Reset timer 
                    openingDoorTimer = 0f;

                    // Return to monsters' previous state
                    aiState = aiStateBeforeDoorOpeningPhase;

                    SetNormalSpeedInNavMeshAgent();

                    TransitionIdleToWalk();
                }
                break;

            default:
                break;
        }
    }

    #endregion

    #endregion

    #region Waypoint Management Methods
    /// <summary>
    /// Defines a monster destination
    /// </summary>
    public void SetDestination()
    {
        // Determine a new destination
        DefineNewWaypoint();

        // If our AI bot is a 3D Model, play the animations that the bot should play.
        AnimationsForModel();

        // Grant the AI the same speed and acceleration it had, once it starts moving again 
        SetNormalSpeedInNavMeshAgent();

        // Occupy The Target Node, so other AI bots do NOT go for it.
        currentWayPoint.GetComponent<node>().isOccupied = true;

        if (!currentWayPoint) Debug.Log("Target is null!");
    }

    /// <summary>
    /// Choose a new waypoint
    /// </summary>
    private void DefineNewWaypoint()
    {
        possibleWaypoints.Clear();

        foreach (var item in waypoints)
        {
            // Check if the node is not occupied 
            if (!item.GetComponent<node>().isOccupied)
            {
                possibleWaypoints.Add(item);
            }
        }

        // Release the current waypoint
        if (currentWayPoint)
        {
            currentWayPoint.GetComponent<node>().isOccupied = false;
        }

        // Pick at random new destination
        currentWayPoint = possibleWaypoints[Random.Range(0, possibleWaypoints.Count)];
    }

    public void AddNewWaypointForTheMonster(GameObject newWaypoint)
    {
        string monsterName = transform.name;
        string monsterNameWithSuffix = monsterName + "_Node";

        // Check if we it's the appropriate waypoint for the current monster
        if (newWaypoint.CompareTag(monsterNameWithSuffix))
        {
            waypoints.Add(newWaypoint);
        }
    }
    #endregion

    #region Animation Methods
    private void AnimationsForModel()
    {
        if (isModel)
        {
            GetComponent<Animation>().clip = myAnims[1];
            GetComponent<Animation>().Play();
        }
    }

    private void TransitionIdleToChase()
    {
        animator.SetBool("IsChasing", true);
    }

    private void TransitionIdleToWalk()
    {
        animator.SetBool("IsWalking", true);
    }

    private void TransitionWalkToIdle()
    {
        animator.SetBool("IsWalking", false);
    }

    private void TransitionWalkToChase()
    {
        animator.SetBool("IsChasing", true);
    }

    private void TransitionChaseToWalk()
    {
        animator.SetBool("IsChasing", false);
    }
    #endregion

    #region Nav Mesh Agent Methods
    private void FreezeNavMeshAgent()
    {
        nav.speed = 0f;
        nav.acceleration = 0f;
        nav.angularSpeed = 0f;
    }

    private void SetNormalSpeedInNavMeshAgent()
    {
        nav.speed = initialSpeed;
        nav.acceleration = initialAcceleration;
        nav.angularSpeed = initialAngularSpeed;
    }

    private void SetChaseSpeedInNavMeshAgent()
    {
        nav.speed = chaseSpeed;
        nav.acceleration = initialAcceleration;
        nav.angularSpeed = initialAngularSpeed;
    }
    #endregion
}
