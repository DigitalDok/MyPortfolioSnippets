using UnityEngine;
using System.Collections;

public class AI_Senses : MonoBehaviour
{
    #region Fields
    public bool hasSight;       // Whether or not the monster can see the player.
    public float fieldOfViewAngle = 110f;       // Number of degrees, centred on forward, for the enemy see.
    public bool playerInSight;      // Whether or not the player is currently sighted.
    private float lightSensitivityRadius = 6.5f;        // The radius that the monster can see the lantern

    public bool hasHearing;     // Whether or not the monster can hear the player.
    public float personalHearingSensitivity;  // Monster based variable that determines the distance that player can hear you
    public float hearingDistanceMultiplier; // Monster Hearing distance multiplier (This modifies the hearing range, which is the sphere collider radius)

    public string currentRoom;

    public Vector3 lastSeenPosition;        // Last place this monster spotted the player.
    public Vector3 lastHeardPosition;       // Last place this monster heard the player.

    public string lastSeenPositionRoom;        // The Room the character was last seen.
    public string lastHeardPositionRoom;       // The Room the character was last heard.


    internal Vector3 resetPosition = new Vector3(10000, 10000, 10000);
    internal string resetRoomString = "None";

    private NavMeshAgent nav;       // Reference to the nav mesh agent.
    private SphereCollider col;     // Reference to the sphere collider trigger component.
    internal Player player;        // Reference to the player script.

    #endregion

    #region Initialization
    void Awake()
    {
        // Setting up the references.
        nav = GetComponent<NavMeshAgent>();
        col = GetComponent<SphereCollider>();
        player = GameObject.FindGameObjectWithTag("Player").GetComponent<Player>();

        lastSeenPosition = lastHeardPosition = resetPosition;
        lastSeenPositionRoom = lastHeardPositionRoom = resetRoomString;
    }
    #endregion

    #region Update    
    /*
    void Update()
    {
        if (hasSight && !player.isHidden)
        {
            
            if (IsPlayerInMyRoom() && IsPlayerUsingLantern())
            {
                if (CalculatePathLength(player.transform.position) <= lightSensitivityRadius)
                    lastSeenPosition = player.transform.position;
            }
            
        }

    }
    */
    #endregion

    #region Trigger Methods
    void OnTriggerStay(Collider other)
    {
        if (other.gameObject.CompareTag("Player"))
        {
            if (hasSight && !player.isHidden)
            {
                playerInSight = false;

                Vector3 direction = other.transform.position + other.transform.up - transform.position;
                float angle = Vector3.Angle(direction, transform.forward);

                if (angle < fieldOfViewAngle * 0.5f)
                {
                    RaycastHit hit;

                    Debug.DrawRay(transform.position, direction, Color.red);    // Debug Line

                    if (Physics.Raycast(transform.position, direction.normalized, out hit, col.radius * transform.localScale.x))
                    {
                        //Debug.Log(hit.collider.name);

                        if (hit.collider.gameObject.CompareTag("Player"))
                        {
                            // Player is SEEN.
                            playerInSight = true;
                            lastSeenPosition = player.transform.position;
                            lastSeenPositionRoom = player.currentRoom;
                        }
                    }
                }
            }
            if (hasHearing)
            {
                if (player.GetComponent<Player>().currentNoise > 0)
                {
                    if (CanIHearThePlayer())
                    {
                        if (CalculatePathLength(player.transform.position) <= col.radius * hearingDistanceMultiplier)
                            // ... set the last personal sighting of the player to the player's current position.
                            lastHeardPosition = player.transform.position;
                            lastHeardPositionRoom = player.currentRoom;
                    }
                }
            }
        }
    }
    

    private bool CanIHearThePlayer()
    {
        //Debug.Log(player.GetComponent<Player>().currentNoise / CalculatePathLength(player.transform.position));
        return (player.GetComponent<Player>().currentNoise / CalculatePathLength(player.transform.position) > personalHearingSensitivity);
    }

    void OnTriggerExit(Collider other)
    {
        if (hasSight)
        {
            if (other.gameObject.CompareTag("Player"))
            {
                playerInSight = false;
                lastSeenPosition = resetPosition;
                lastSeenPositionRoom = resetRoomString;
            }
        }
    }
    #endregion

    #region Helper Methods
    float CalculatePathLength(Vector3 targetPosition)
    {
        // Create a path and set it based on a target position.
        NavMeshPath path = new NavMeshPath();
        if (nav.enabled)
            nav.CalculatePath(targetPosition, path);

        // Create an array of points which is the length of the number of corners in the path + 2.
        Vector3[] allWayPoints = new Vector3[path.corners.Length + 2];

        // The first point is the enemy's position.
        allWayPoints[0] = transform.position;

        // The last point is the target position.
        allWayPoints[allWayPoints.Length - 1] = targetPosition;

        // The points inbetween are the corners of the path.
        for (int i = 0; i < path.corners.Length; i++)
        {
            allWayPoints[i + 1] = path.corners[i];
        }

        // Create a float to store the path length that is by default 0.
        float pathLength = 0;

        // Increment the path length by an amount equal to the distance between each waypoint and the next.
        for (int i = 0; i < allWayPoints.Length - 1; i++)
        {
            pathLength += Vector3.Distance(allWayPoints[i], allWayPoints[i + 1]);
        }

        return pathLength;
    }

    /// <summary>
    /// Checks if the player is using his lantern
    /// </summary>
    /// <returns>Returns true if he does</returns>
    private bool IsPlayerUsingLantern()
    {
        return player.AmIUsingLantern();
    }

    /// <summary>
    /// Checks if the player is in the same room, with the current monster
    /// </summary>
    /// <returns>Returns true if he does</returns>
    private bool IsPlayerInMyRoom()
    {
        return (currentRoom == player.currentRoom);
    }
    #endregion
}
