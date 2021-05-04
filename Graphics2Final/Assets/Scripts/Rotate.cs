using System.Collections;
using System.Collections.Generic;
using UnityEngine;

// This rotates the light at a steady speed.
public class Rotate : MonoBehaviour
{
    public bool rotate = false;
    [Range(0, 90)]
    public float speed = 45.0f;

    void Update()
    {
        if(rotate)
            transform.Rotate(Vector3.up * (speed * Time.deltaTime));
    }
}
