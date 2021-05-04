# GPR 300 Intermediate Graphics & Animation Programming Final Project
In order to use this project Unity must be installed. The project is built in Unity 2019.3.14f1 and we recommend opening it in that version as upgrading projects to a newer version of Unity has the possibility of causing errors and unexpected behavior.

This project contains examples of a Blinn-Phong shader with normal mapping and emissives, a bloom and depth of feild post-processing effect, and progress towards working inverse kinematics. The IK is not currently fully functional but we wanted to include our progress on it.

Once this repository is cloned the project contained in the Graphics2Final/ directory can be opened with Unity. When the project starts the Demo scene should open by default. If it is not, navigate to the Scenes folder under Assets in the Project window and open the scene titled "Demo". 

In this scene the bloom and depth of feild effects are applied by scripts on the Camera, and the Blinn-Phong shader is applied to materials on objects in the scene. In order to experiment with the post-processing effects you must play the scene, click on the camera in the Hierarchy, and alter the values exposed by the scripts. To experiment with the WIP IK tentacle simply move the object titled "IKTarget".
