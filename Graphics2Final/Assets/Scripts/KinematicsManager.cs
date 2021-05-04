using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class KinematicsManager : MonoBehaviour
{
    [Header("IK Parameters")]
    public float samplingDistance;
    public float learningRate;
    public float distanceThreshold;

    [Header("Object References")]
    public Transform IKTarget;
    public GameObject armature;

    [Header("Debug")]
    public List<Joint> modelJoints;
    [SerializeField]
    float[] localAngles;
    
    int boneCount;
    Joint[] joints;

    // Start is called before the first frame update
    void Start()
    {
        InitBones();

        boneCount = modelJoints.Count;
        joints = new Joint[boneCount];
        modelJoints.CopyTo(joints);

        localAngles = new float[boneCount];
        for (int i = 0; i < boneCount; i++)
            localAngles[i] = 0.0f;
    }

    // Update is called once per frame
    void Update()
    {
        // Calculate new rotations using IK
        CalculateInverseKinematics(IKTarget.position);

        // Apply new rotations
        for (int i = 0; i < boneCount; i++)
            joints[i].transform.localRotation = Quaternion.AngleAxis(localAngles[i], joints[i].axis);
    }

    void InitBones()
    {
        Vector3[] axis = new Vector3[3];
        axis[0] = new Vector3(1, 0, 0);
        axis[1] = new Vector3(0, 1, 0);
        axis[2] = new Vector3(0, 0, 1);

        Transform bone = armature.transform;
        Joint joint;
        int axisCounter = 0;

        // Add joint script to each bone on mesh
        while(bone.childCount > 0)
        {
            bone = bone.GetChild(0);
            
            // Set correct axis
            joint = bone.gameObject.AddComponent<Joint>();
            joint.axis = axis[axisCounter];

            // Set correct min/max angles
            joint.minAngle = -35.0f;
            joint.maxAngle = 35.0f;
            axisCounter = (axisCounter++) % 3;

            // Add joint to list of joints
            modelJoints.Add(joint);
        }
    }

    public void CalculateInverseKinematics(Vector3 target)
    {
        //break out of IK calculation if close enough to target
        if (GetDistanceFromTarget(target) < distanceThreshold)
            return;

        // Start looping from last joint in list
        for(int i = boneCount - 1; i >= 0; i--)
        {
            // Update local rotations based on learning rate, clamp between joint min/max angles
            float gradient = PartialGradient(target, i);
            localAngles[i] -= learningRate * gradient;
            localAngles[i] = Mathf.Clamp(localAngles[i], joints[i].minAngle, joints[i].maxAngle);

            //break out of IK calculation if close enough to target
            if (GetDistanceFromTarget(target) < distanceThreshold)
                return;
        }
    }

    public Vector3 CalculateForwardKinematics()
    {
        Vector3 prevJoint = joints[0].transform.position; // Highest joint in bone hierarchy
        Quaternion rotation = Quaternion.identity;

        for(int i = 1; i < boneCount; i++)
        {
            // Rotate around joint axis
            rotation *= Quaternion.AngleAxis(localAngles[i - 1], joints[i - 1].axis);
            Vector3 nextJoint = prevJoint + rotation * joints[i].startOffset;

            prevJoint = nextJoint;
        }

        // Return last joint in the bone hierarchy
        return prevJoint;
    }    

    
    public float PartialGradient(Vector3 target, int index)
    {
        float angle = localAngles[index];

        //Calculate Gradient - see https://www.alanzucconi.com/2017/04/10/gradient-descent/ 
        float distance = GetDistanceFromTarget(target);
        localAngles[index] += samplingDistance;
        float distPlusSample = GetDistanceFromTarget(target);

        // [F(x + h) - F(x)] / h (Simulate a limit)
        float gradient = (distPlusSample - distance) / samplingDistance;

        //Reset Local Angle
        localAngles[index] = angle;

        return gradient;
    }

    public float GetDistanceFromTarget(Vector3 target)
    {
        Vector3 start = CalculateForwardKinematics();
        float dist = Vector3.Distance(target, start);


        return dist;
    }
}
