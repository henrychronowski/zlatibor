using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Joint : MonoBehaviour
{
    public Vector3 axis;
    public Vector3 startOffset;
    public float minAngle = 0;
    public float maxAngle = 360;

    private void Awake()
    {
        startOffset = transform.localPosition;
    }
}
